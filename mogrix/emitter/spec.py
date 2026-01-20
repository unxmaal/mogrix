"""Spec file writer for mogrix."""

import re
from mogrix.rules.engine import TransformResult


class SpecWriter:
    """Writes modified spec file content."""

    def write(
        self,
        result: TransformResult,
        drops: list[str] | None = None,
        adds: list[str] | None = None,
    ) -> str:
        """Generate modified spec content from transform result."""
        content = result.spec.raw_content
        drops = drops or []
        adds = adds or []

        # Remove dropped BuildRequires
        for dep in drops:
            # Match BuildRequires line containing the dep
            pattern = rf"^BuildRequires:\s*{re.escape(dep)}\s*$"
            content = re.sub(pattern, "", content, flags=re.MULTILINE)

        # Add new BuildRequires (after last existing one)
        if adds:
            # Find last BuildRequires line
            lines = content.splitlines()
            last_br_idx = -1
            for i, line in enumerate(lines):
                if line.strip().startswith("BuildRequires:"):
                    last_br_idx = i

            if last_br_idx >= 0:
                # Insert after last BuildRequires
                for dep in adds:
                    lines.insert(last_br_idx + 1, f"BuildRequires: {dep}")
                    last_br_idx += 1
                content = "\n".join(lines)

        # Inject configure --disable flags
        if result.configure_disable:
            flags = " ".join(
                f"--disable-{f}" for f in result.configure_disable
            )
            # Replace %configure with %configure + flags
            content = re.sub(
                r"^(%configure)(\s*)$",
                rf"\1 {flags}\2",
                content,
                flags=re.MULTILINE,
            )
            # Also handle %configure with existing flags
            content = re.sub(
                r"^(%configure\s+)(.+)$",
                rf"\1{flags} \2",
                content,
                flags=re.MULTILINE,
            )

        # Rewrite paths
        for old_path, new_path in result.path_rewrites.items():
            content = content.replace(old_path, new_path)

        # Clean up empty lines from removals
        content = re.sub(r"\n{3,}", "\n\n", content)

        return content
