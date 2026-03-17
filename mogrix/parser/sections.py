"""Spec file section splitting and reassembly.

Shared utility for section-aware spec transformations. Used by
comment_matching, remove_matching, drop_subpackages, section_replace,
and feature flag expansion.
"""

import re
from dataclasses import dataclass

# Section markers recognized by RPM. Order matters for display but not parsing.
# Each marker may have a subpackage suffix: %files server, %post -n libfoo, etc.
_SECTION_RE = re.compile(
    r"^(%(?:description|package|prep|build|install|check|clean"
    r"|files|pre|post|preun|postun|pretrans|posttrans"
    r"|triggerin|triggerun|triggerpostun|verifyscript"
    r"|patchlist|sourcelist|changelog))\b(.*)?$",
    re.MULTILINE,
)


@dataclass
class SpecSection:
    """A single section of an RPM spec file."""

    name: str  # "%prep", "%build", "%files", etc.
    start_line: int  # line number (0-based) of the section marker
    end_line: int  # line number of next section marker (exclusive)
    content: str  # the section's text EXCLUDING the marker line
    marker_line: str  # the full marker line (e.g. "%files server")
    subpackage: str  # "" for main package, "server" for %files server, etc.


def _parse_subpackage(suffix: str) -> str:
    """Extract subpackage name from section marker suffix.

    "%files server" -> "server"
    "%post -n libfoo" -> "libfoo"
    "%description" -> ""
    """
    suffix = suffix.strip()
    if not suffix:
        return ""
    # Handle -n prefix
    if suffix.startswith("-n ") or suffix.startswith("-n\t"):
        return suffix[3:].strip().split()[0] if len(suffix) > 3 else ""
    # First token is the subpackage name
    return suffix.split()[0]


def split_spec_sections(content: str) -> list[SpecSection]:
    """Split spec content into sections.

    The preamble (everything before the first section marker) is returned
    as a section with name="preamble".

    Round-trip guarantee: reassemble_spec(split_spec_sections(s)) == s
    """
    lines = content.split("\n")
    sections: list[SpecSection] = []

    # Find all section marker positions
    markers: list[tuple[int, str, str, str]] = []  # (line_idx, name, suffix, full_line)
    for i, line in enumerate(lines):
        m = _SECTION_RE.match(line)
        if m:
            markers.append((i, m.group(1), (m.group(2) or "").strip(), line))

    if not markers:
        # No section markers — entire content is preamble
        return [
            SpecSection(
                name="preamble",
                start_line=0,
                end_line=len(lines),
                content=content,
                marker_line="",
                subpackage="",
            )
        ]

    # Preamble: everything before first section marker
    first_marker_line = markers[0][0]
    if first_marker_line > 0:
        preamble_lines = lines[:first_marker_line]
        sections.append(
            SpecSection(
                name="preamble",
                start_line=0,
                end_line=first_marker_line,
                content="\n".join(preamble_lines),
                marker_line="",
                subpackage="",
            )
        )

    # Each section runs from its marker to the next marker
    for idx, (line_num, name, suffix, full_line) in enumerate(markers):
        if idx + 1 < len(markers):
            end = markers[idx + 1][0]
        else:
            end = len(lines)

        # Content is everything after the marker line up to (exclusive) end
        section_content_lines = lines[line_num + 1 : end]
        sections.append(
            SpecSection(
                name=name,
                start_line=line_num,
                end_line=end,
                content="\n".join(section_content_lines),
                marker_line=full_line,
                subpackage=_parse_subpackage(suffix),
            )
        )

    return sections


def reassemble_spec(sections: list[SpecSection]) -> str:
    """Reassemble sections back into spec content.

    Preserves exact whitespace and line endings from the original split.
    """
    parts: list[str] = []
    for section in sections:
        if section.name == "preamble":
            parts.append(section.content)
        else:
            parts.append(section.marker_line)
            if section.content:
                parts.append(section.content)
            else:
                # Empty section — just the marker
                pass
    return "\n".join(parts)


def find_sections(
    sections: list[SpecSection],
    name: str | None = None,
    subpackage: str | None = None,
) -> list[SpecSection]:
    """Find sections matching criteria.

    Args:
        sections: List from split_spec_sections()
        name: Section name to match (e.g. "%prep", "%files"). None matches all.
        subpackage: Subpackage to match. None matches all, "" matches main package only.
    """
    result = []
    for s in sections:
        if name is not None and s.name != name:
            continue
        if subpackage is not None and s.subpackage != subpackage:
            continue
        result.append(s)
    return result
