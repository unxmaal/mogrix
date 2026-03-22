# pyright: basic
"""Line-oriented parser for RPM spec files → AST.

Parses a spec file into an AST of typed nodes. Shell content inside sections
is opaque (ShellLine). The parser handles:
- Preamble directives (Name:, BuildRequires:, Source0:, etc.)
- %global/%define macro definitions
- %if/%else/%endif conditionals (nested)
- Sections (%prep, %build, %install, %files, %changelog, etc.)
- %patch application lines
- Comments and blank lines
- Backslash continuation lines

Key invariant: serialize(parse(text)) == text for unmodified nodes.
"""

from __future__ import annotations

import re
from dataclasses import field

from mogrix.parser.ast import (
    ASTNode,
    Blank,
    Comment,
    Conditional,
    ContinuationLine,
    Directive,
    DirectiveKind,
    MacroDef,
    PatchApp,
    Section,
    SectionKind,
    ShellLine,
    SourceSpan,
    SpecFile,
)

# --- Directive tag mapping ---

_DIRECTIVE_MAP: dict[str, DirectiveKind] = {
    "name": DirectiveKind.NAME,
    "version": DirectiveKind.VERSION,
    "release": DirectiveKind.RELEASE,
    "summary": DirectiveKind.SUMMARY,
    "license": DirectiveKind.LICENSE,
    "url": DirectiveKind.URL,
    "buildrequires": DirectiveKind.BUILD_REQUIRES,
    "requires": DirectiveKind.REQUIRES,
    "prereq": DirectiveKind.PREREQ,
    "provides": DirectiveKind.PROVIDES,
    "conflicts": DirectiveKind.CONFLICTS,
    "obsoletes": DirectiveKind.OBSOLETES,
    "group": DirectiveKind.GROUP,
    "epoch": DirectiveKind.EPOCH,
    "buildarch": DirectiveKind.BUILDARCH,
    "excludearch": DirectiveKind.EXCLUDEARCH,
    "exclusivearch": DirectiveKind.EXCLUSIVEARCH,
    "autoreq": DirectiveKind.AUTOREQ,
    "autoprov": DirectiveKind.AUTOPROV,
}

# --- Section keyword mapping ---

_SECTION_MAP: dict[str, SectionKind] = {
    "description": SectionKind.DESCRIPTION,
    "prep": SectionKind.PREP,
    "build": SectionKind.BUILD,
    "install": SectionKind.INSTALL,
    "check": SectionKind.CHECK,
    "files": SectionKind.FILES,
    "changelog": SectionKind.CHANGELOG,
    "clean": SectionKind.CLEAN,
    "pre": SectionKind.PRE,
    "post": SectionKind.POST,
    "preun": SectionKind.PREUN,
    "postun": SectionKind.POSTUN,
    "pretrans": SectionKind.PRETRANS,
    "posttrans": SectionKind.POSTTRANS,
    "verifyscript": SectionKind.VERIFYSCRIPT,
    "triggerin": SectionKind.TRIGGERIN,
    "triggerun": SectionKind.TRIGGERUN,
    "triggerpostun": SectionKind.TRIGGERPOSTUN,
    "transfiletriggerin": SectionKind.TRANSFILETRIGGERIN,
    "transfiletriggerpostun": SectionKind.TRANSFILETRIGGERPOSTUN,
    "package": SectionKind.PACKAGE,
    "patchlist": SectionKind.PATCHLIST,
}

# Regex patterns
_RE_MACRO_DEF = re.compile(r"^%(global|define)\s+(\S+)\s+(.*?)\s*$")
_RE_DIRECTIVE = re.compile(r"^([A-Za-z]\w*(?:\([^)]*\))?)\s*:\s*(.*?)\s*$")
_RE_SOURCE_TAG = re.compile(r"^(Source|Patch)(\d*)\s*:", re.IGNORECASE)
_RE_SECTION = re.compile(r"^%(\w+)\b(.*)$")
_RE_IF = re.compile(r"^%if\b\s*(.*?)\s*$")
_RE_ELSE = re.compile(r"^%else\s*$")
_RE_ENDIF = re.compile(r"^%endif\s*$")
_RE_PATCH_APP = re.compile(r"^%patch\s+-P\s*(\d+)(.*)")
_RE_PATCH_LEGACY = re.compile(r"^%patch(\d+)\b(.*)")
_RE_AUTOSETUP = re.compile(r"^%autosetup\b(.*)")
_RE_SETUP = re.compile(r"^%setup\b(.*)")


def _parse_directive_values(raw: str) -> list[str]:
    """Parse a directive value string into individual values.

    Handles simple space-separated values and version constraints like >= 1.0.
    Does NOT expand macros.
    """
    parts: list[str] = []
    tokens = raw.split()
    i = 0
    while i < len(tokens):
        tok = tokens[i]
        # Check if next tokens are a version constraint (>=, <=, =, >, <)
        if i + 2 < len(tokens) and re.match(r"^[<>=]+$", tokens[i + 1]):
            parts.append(f"{tok} {tokens[i + 1]} {tokens[i + 2]}")
            i += 3
        else:
            parts.append(tok)
            i += 1
    return parts


def _classify_directive(tag: str) -> tuple[DirectiveKind, int | None]:
    """Classify a directive tag and extract number if applicable."""
    # Handle Source/Patch numbered tags
    m = _RE_SOURCE_TAG.match(tag + ":")
    if m:
        prefix = m.group(1).lower()
        num_str = m.group(2)
        num = int(num_str) if num_str else 0
        kind = DirectiveKind.SOURCE if prefix == "source" else DirectiveKind.PATCH
        return kind, num

    # Handle Requires with scriptlet qualifier: Requires(pre), Requires(post), etc.
    base = tag.split("(")[0].lower()
    kind = _DIRECTIVE_MAP.get(base, DirectiveKind.OTHER)
    return kind, None


def _is_section_start(line: str) -> tuple[SectionKind, str, str | None] | None:
    """Check if a line starts a new section.

    Returns (kind, full_header, subpackage_name) or None.
    """
    stripped = line.strip()
    m = _RE_SECTION.match(stripped)
    if not m:
        return None

    keyword = m.group(1).lower()
    rest = m.group(2).strip()

    if keyword not in _SECTION_MAP:
        return None

    kind = _SECTION_MAP[keyword]

    # Extract subpackage name for applicable sections
    subpkg: str | None = None
    if kind in (
        SectionKind.PACKAGE, SectionKind.DESCRIPTION, SectionKind.FILES,
        SectionKind.PRE, SectionKind.POST, SectionKind.PREUN, SectionKind.POSTUN,
        SectionKind.PRETRANS, SectionKind.POSTTRANS,
    ):
        if rest:
            if rest.startswith("-n ") or rest.startswith("-n\t"):
                parts = rest[3:].strip().split()
                subpkg = parts[0] if parts else None
            else:
                parts = rest.split()
                # Skip flags like -f, -p
                for p in parts:
                    if not p.startswith("-"):
                        subpkg = p
                        break

    return kind, line, subpkg


class SpecASTParser:
    """Parse a spec file into an AST."""

    def parse(self, text: str) -> SpecFile:
        """Parse spec file text into a SpecFile AST."""
        self._lines = text.splitlines(keepends=True)
        self._pos = 0
        self._text = text

        nodes = self._parse_nodes(top_level=True)
        return SpecFile(nodes=nodes, source_text=text)

    def _parse_nodes(
        self,
        top_level: bool = False,
        stop_at_else: bool = False,
        stop_at_endif: bool = False,
    ) -> list[ASTNode]:
        """Parse a sequence of nodes.

        Args:
            top_level: True if parsing the top-level spec (parse until EOF)
            stop_at_else: Stop when %else is encountered (for %if true branch)
            stop_at_endif: Stop when %endif is encountered (for %else false branch)
        """
        nodes: list[ASTNode] = []

        while self._pos < len(self._lines):
            line = self._lines[self._pos]
            stripped = line.strip()

            # Check stop conditions for conditional parsing
            if stop_at_else and (_RE_ELSE.match(stripped) or _RE_ENDIF.match(stripped)):
                break
            if stop_at_endif and _RE_ENDIF.match(stripped):
                break

            node = self._parse_one_node(line, stripped)
            if node is not None:
                nodes.append(node)

        return nodes

    def _parse_one_node(self, line: str, stripped: str) -> ASTNode | None:
        """Parse one node starting at current position. Advances _pos."""
        lineno = self._pos + 1  # 1-based

        # Blank line
        if not stripped:
            self._pos += 1
            return Blank(span=SourceSpan(lineno, lineno, line))

        # Comment
        if stripped.startswith("#"):
            self._pos += 1
            return Comment(
                span=SourceSpan(lineno, lineno, line),
                text=stripped[1:].lstrip(),
            )

        # %if conditional
        m = _RE_IF.match(stripped)
        if m:
            return self._parse_conditional(lineno, line, m.group(1))

        # %global / %define
        m = _RE_MACRO_DEF.match(stripped)
        if m:
            self._pos += 1
            return MacroDef(
                span=SourceSpan(lineno, lineno, line),
                keyword=m.group(1),
                name=m.group(2),
                value=m.group(3),
            )

        # Section start
        sec = _is_section_start(line)
        if sec:
            return self._parse_section(lineno, line, sec[0], sec[2])

        # %patch application
        m = _RE_PATCH_APP.match(stripped)
        if m:
            self._pos += 1
            return PatchApp(
                span=SourceSpan(lineno, lineno, line),
                patch_number=int(m.group(1)),
                flags=m.group(2).strip(),
            )

        m = _RE_PATCH_LEGACY.match(stripped)
        if m:
            self._pos += 1
            return PatchApp(
                span=SourceSpan(lineno, lineno, line),
                patch_number=int(m.group(1)),
                flags=m.group(2).strip(),
            )

        # %autosetup / %setup
        m = _RE_AUTOSETUP.match(stripped)
        if m:
            self._pos += 1
            return PatchApp(
                span=SourceSpan(lineno, lineno, line),
                patch_number=None,
                flags=m.group(1).strip(),
                is_autosetup=True,
            )

        m = _RE_SETUP.match(stripped)
        if m:
            self._pos += 1
            return ShellLine(span=SourceSpan(lineno, lineno, line), text=stripped)

        # Directive (Tag: value)
        m = _RE_DIRECTIVE.match(stripped)
        if m:
            tag = m.group(1)
            raw_value = m.group(2)
            kind, number = _classify_directive(tag)
            values = _parse_directive_values(raw_value)
            self._pos += 1
            return Directive(
                span=SourceSpan(lineno, lineno, line),
                kind=kind,
                tag=tag,
                values=values,
                raw_value=raw_value,
                number=number,
            )

        # Backslash continuation
        if stripped.endswith("\\"):
            return self._parse_continuation(lineno)

        # Default: opaque shell line
        self._pos += 1
        return ShellLine(span=SourceSpan(lineno, lineno, line), text=stripped)

    def _parse_conditional(
        self, start_line: int, if_line_text: str, condition: str
    ) -> Conditional:
        """Parse a %if/%else/%endif block with nested support."""
        self._pos += 1  # skip %if line

        # Parse true branch (stops at %else or %endif)
        true_branch = self._parse_nodes(stop_at_else=True)

        # Check what stopped us
        else_line: str | None = None
        false_branch: list[ASTNode] = []

        if self._pos < len(self._lines):
            stopped = self._lines[self._pos].strip()
            if _RE_ELSE.match(stopped):
                else_line = self._lines[self._pos]
                self._pos += 1  # skip %else
                false_branch = self._parse_nodes(stop_at_endif=True)

        # Consume %endif
        endif_line = ""
        if self._pos < len(self._lines):
            stopped = self._lines[self._pos].strip()
            if _RE_ENDIF.match(stopped):
                endif_line = self._lines[self._pos]
                self._pos += 1

        end_line = self._pos  # 1-based end is current pos

        # Build the full original text
        original_lines = self._lines[start_line - 1 : self._pos]
        original_text = "".join(original_lines)

        return Conditional(
            span=SourceSpan(start_line, end_line, original_text),
            condition=condition,
            if_line=if_line_text,
            true_branch=true_branch,
            false_branch=false_branch,
            else_line=else_line,
            endif_line=endif_line,
        )

    def _parse_section(
        self, start_line: int, header_line: str, kind: SectionKind,
        subpackage: str | None,
    ) -> Section:
        """Parse a section and its body until the next section or EOF."""
        self._pos += 1  # skip section header

        body: list[ASTNode] = []

        # %changelog is special — treat everything as opaque text
        if kind == SectionKind.CHANGELOG:
            changelog_lines: list[str] = []
            cl_start = self._pos
            while self._pos < len(self._lines):
                line = self._lines[self._pos]
                stripped = line.strip()
                # Stop at next section
                if _is_section_start(line) is not None:
                    break
                changelog_lines.append(line)
                self._pos += 1

            if changelog_lines:
                text = "".join(changelog_lines)
                body.append(ShellLine(
                    span=SourceSpan(cl_start + 1, self._pos, text),
                    text=text.rstrip("\n"),
                ))
        else:
            # Parse body nodes until next section start
            while self._pos < len(self._lines):
                line = self._lines[self._pos]
                stripped = line.strip()

                # Stop at section boundaries
                if _is_section_start(line) is not None:
                    break

                node = self._parse_one_node(line, stripped)
                if node is not None:
                    body.append(node)

        end_line = self._pos
        original_lines = self._lines[start_line - 1 : self._pos]
        original_text = "".join(original_lines)

        return Section(
            span=SourceSpan(start_line, end_line, original_text),
            kind=kind,
            header=header_line,
            subpackage=subpackage,
            body=body,
        )

    def _parse_continuation(self, start_line: int) -> ContinuationLine:
        """Parse backslash-continued lines."""
        lines: list[str] = []
        while self._pos < len(self._lines):
            line = self._lines[self._pos]
            lines.append(line)
            self._pos += 1
            if not line.rstrip("\n").endswith("\\"):
                break

        original_text = "".join(lines)
        # Build logical line by removing trailing backslash-newline
        logical = "".join(
            l.rstrip("\n").rstrip("\\") for l in lines
        ).strip()

        return ContinuationLine(
            span=SourceSpan(start_line, start_line + len(lines) - 1, original_text),
            lines=[l.rstrip("\n") for l in lines],
            logical_text=logical,
        )
