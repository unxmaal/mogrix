"""RPM spec file parser."""

import re
from dataclasses import dataclass, field
from pathlib import Path


@dataclass
class SpecFile:
    """Parsed RPM spec file."""

    name: str = ""
    version: str = ""
    release: str = ""
    summary: str = ""
    license: str = ""
    url: str = ""

    buildrequires: list[str] = field(default_factory=list)
    requires: list[str] = field(default_factory=list)

    sources: dict[str, str] = field(default_factory=dict)
    patches: dict[str, str] = field(default_factory=dict)

    raw_content: str = ""


class SpecParser:
    """Parser for RPM spec files."""

    # Preamble field patterns
    PREAMBLE_FIELDS = {
        "Name": "name",
        "Version": "version",
        "Release": "release",
        "Summary": "summary",
        "License": "license",
        "URL": "url",
    }

    def parse(self, path: Path) -> SpecFile:
        """Parse a spec file and return structured data."""
        content = Path(path).read_text()
        spec = SpecFile(raw_content=content)

        in_preamble = True

        for line in content.splitlines():
            line = line.strip()

            # Skip comments and empty lines
            if not line or line.startswith("#"):
                continue

            # End preamble at first section marker
            if line.startswith("%description") or line.startswith("%package"):
                in_preamble = False

            # Parse preamble fields (only in main preamble)
            if in_preamble:
                for tag, attr in self.PREAMBLE_FIELDS.items():
                    if line.startswith(f"{tag}:"):
                        value = line[len(tag) + 1:].strip()
                        setattr(spec, attr, value)
                        break

            # Parse BuildRequires
            if line.startswith("BuildRequires:"):
                deps = line[14:].strip()
                # Split on comma or whitespace, filter empty
                for dep in re.split(r"[,\s]+", deps):
                    dep = dep.strip()
                    if dep and not dep.startswith("%"):
                        # Remove version constraints
                        dep = re.split(r"\s*[<>=]", dep)[0]
                        if dep:
                            spec.buildrequires.append(dep)

            # Parse Requires
            if line.startswith("Requires:") and not line.startswith("BuildRequires:"):
                deps = line[9:].strip()
                for dep in re.split(r"[,\s]+", deps):
                    dep = dep.strip()
                    if dep and not dep.startswith("%"):
                        dep = re.split(r"\s*[<>=]", dep)[0]
                        if dep:
                            spec.requires.append(dep)

            # Parse Source entries
            match = re.match(r"^(Source\d*):(.+)$", line)
            if match:
                key = match.group(1)
                value = match.group(2).strip()
                spec.sources[key] = value

            # Parse Patch entries
            match = re.match(r"^(Patch\d+):(.+)$", line)
            if match:
                key = match.group(1)
                value = match.group(2).strip()
                spec.patches[key] = value

        return spec
