# pyright: basic
"""AST node types for RPM spec files.

Design principles:
1. Every node has a SourceSpan with original_text preserving exact whitespace.
2. Round-trip fidelity: serialize(parse(text)) == text for unmodified nodes.
3. Nodes are dataclasses — cheap to construct and inspect.
4. Shell content inside sections is opaque (ShellLine). We don't parse shell.
"""

from __future__ import annotations

from dataclasses import dataclass, field
from enum import Enum, auto
from typing import Union


@dataclass
class SourceSpan:
    """Location and original text of a parsed element.

    original_text is the exact bytes from the input, including whitespace,
    comments, and line endings. For round-trip fidelity, unmodified nodes
    emit their original_text verbatim.
    """

    start_line: int  # 1-based line number
    end_line: int  # inclusive
    original_text: str  # exact text including newlines


class DirectiveKind(Enum):
    """Types of RPM spec directives (tag: value lines)."""

    NAME = auto()
    VERSION = auto()
    RELEASE = auto()
    SUMMARY = auto()
    LICENSE = auto()
    URL = auto()
    SOURCE = auto()  # Source0, Source1, etc.
    PATCH = auto()  # Patch0, Patch1, etc.
    BUILD_REQUIRES = auto()
    REQUIRES = auto()
    PREREQ = auto()
    PROVIDES = auto()
    CONFLICTS = auto()
    OBSOLETES = auto()
    GROUP = auto()
    EPOCH = auto()
    BUILDARCH = auto()
    EXCLUDEARCH = auto()
    EXCLUSIVEARCH = auto()
    AUTOREQ = auto()
    AUTOPROV = auto()
    OTHER = auto()  # Any unrecognized Tag: value


class SectionKind(Enum):
    """Types of RPM spec sections."""

    DESCRIPTION = auto()
    PREP = auto()
    BUILD = auto()
    INSTALL = auto()
    CHECK = auto()
    FILES = auto()
    CHANGELOG = auto()
    CLEAN = auto()
    PRE = auto()
    POST = auto()
    PREUN = auto()
    POSTUN = auto()
    PRETRANS = auto()
    POSTTRANS = auto()
    VERIFYSCRIPT = auto()
    TRIGGERIN = auto()
    TRIGGERUN = auto()
    TRIGGERPOSTUN = auto()
    TRANSFILETRIGGERIN = auto()
    TRANSFILETRIGGERPOSTUN = auto()
    PACKAGE = auto()  # %package subpackage definition
    PATCHLIST = auto()  # %patchlist


# --- AST Nodes ---


@dataclass
class Comment:
    """A comment line (# ...)."""

    span: SourceSpan
    text: str  # comment text (without leading #)


@dataclass
class Blank:
    """An empty or whitespace-only line."""

    span: SourceSpan


@dataclass
class MacroDef:
    """%global or %define macro definition.

    Example: %global _hardened_build 1
    """

    span: SourceSpan
    keyword: str  # "global" or "define"
    name: str  # macro name
    value: str  # macro value (may include macro expansions)


@dataclass
class Directive:
    """A tag: value line in the preamble or package section.

    Example: BuildRequires: foo bar >= 1.0
    """

    span: SourceSpan
    kind: DirectiveKind
    tag: str  # original tag text (e.g., "BuildRequires", "Source0")
    values: list[str]  # parsed individual values
    raw_value: str  # unparsed value string (after the colon)
    number: int | None = None  # for Source/Patch numbered tags


@dataclass
class Conditional:
    """%if/%else/%endif block, possibly nested.

    The body is split into true_branch and false_branch (optional).
    Each branch is a list of nodes.
    """

    span: SourceSpan
    condition: str  # the expression after %if (e.g., '0%{?with_selinux}')
    if_line: str  # full original %if line text
    true_branch: list[ASTNode] = field(default_factory=list)
    false_branch: list[ASTNode] = field(default_factory=list)
    else_line: str | None = None  # full original %else line, if present
    endif_line: str = ""  # full original %endif line


@dataclass
class ShellLine:
    """An opaque line of shell script or unrecognized content.

    Everything inside sections that isn't a recognized directive,
    macro definition, or conditional is treated as a ShellLine.
    """

    span: SourceSpan
    text: str  # the line content


@dataclass
class ContinuationLine:
    """A group of lines joined by \\ continuation.

    Preserves the original multi-line format while allowing
    the full logical line to be inspected.
    """

    span: SourceSpan
    lines: list[str]  # individual physical lines (including \\)
    logical_text: str  # joined text (backslash-newline removed)


@dataclass
class PatchApp:
    """A %patch application line.

    Example: %patch -P 0 -p1
    Example: %patch0 -p1
    Example: %autosetup
    """

    span: SourceSpan
    patch_number: int | None  # None for %autosetup
    flags: str  # everything after the patch ref (e.g., "-p1")
    is_autosetup: bool = False


@dataclass
class Section:
    """A spec section (%prep, %build, %install, %files, etc.).

    The section header line and its body are stored separately.
    Subpackage association is tracked for sections like %files foo.
    """

    span: SourceSpan
    kind: SectionKind
    header: str  # full original section header line
    subpackage: str | None = None  # for %package/-n name
    body: list[ASTNode] = field(default_factory=list)


@dataclass
class SpecFile:
    """Root AST node representing an entire spec file.

    Children are a flat list of top-level nodes: preamble directives,
    macro definitions, conditionals, sections, comments, and blanks.
    """

    nodes: list[ASTNode] = field(default_factory=list)
    source_text: str = ""  # original full text for round-trip verification


# Union type for all AST nodes
ASTNode = Union[
    Comment, Blank, MacroDef, Directive, Conditional,
    ShellLine, ContinuationLine, PatchApp, Section, SpecFile,
]
