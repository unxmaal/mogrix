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
        cppflags: str | None = None,
        compat_sources: str | None = None,
        compat_prep: str | None = None,
        compat_build: str | None = None,
        ac_cv_overrides: dict[str, str] | None = None,
        drop_requires: list[str] | None = None,
        remove_lines: list[str] | None = None,
    ) -> str:
        """Generate modified spec content from transform result."""
        content = result.spec.raw_content
        drops = drops or []
        adds = adds or []

        # Remove dropped BuildRequires
        for dep in drops:
            # Match BuildRequires line containing the dep (with optional version)
            # Handles: BuildRequires: pkg, BuildRequires: pkg >= 1.0, etc.
            pattern = rf"^BuildRequires:\s*{re.escape(dep)}(\s*[<>=].*)?$"
            content = re.sub(pattern, "", content, flags=re.MULTILINE)

        # Remove dropped Requires
        if drop_requires:
            for dep in drop_requires:
                # Match Requires line (but not BuildRequires)
                pattern = rf"^Requires:\s*{re.escape(dep)}(\s*[<>=].*)?$"
                content = re.sub(pattern, "", content, flags=re.MULTILINE)

        # Remove specific lines
        if remove_lines:
            for line_pattern in remove_lines:
                # Escape and match the line
                escaped = re.escape(line_pattern)
                content = re.sub(
                    rf"^{escaped}\s*$",
                    "",
                    content,
                    flags=re.MULTILINE
                )

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

        # Remove configure flags
        if result.configure_flags_remove:
            for flag in result.configure_flags_remove:
                # Remove the flag and any argument (e.g., --with-foo=bar)
                escaped = re.escape(flag)
                # Handle flag with = argument
                content = re.sub(
                    rf"\s*{escaped}(=[^\s]+)?",
                    "",
                    content,
                )

        # Add configure flags
        if result.configure_flags_add:
            add_flags = " ".join(result.configure_flags_add)
            # Add to %configure with no arguments
            content = re.sub(
                r"^(%configure)(\s*)$",
                rf"\1 {add_flags}\2",
                content,
                flags=re.MULTILINE,
            )
            # Add to %configure with existing arguments
            content = re.sub(
                r"^(%configure\s+)(.+)$",
                rf"\1{add_flags} \2",
                content,
                flags=re.MULTILINE,
            )

        # Rewrite paths
        for old_path, new_path in result.path_rewrites.items():
            content = content.replace(old_path, new_path)

        # Inject compat Source entries (after last Source/Patch line)
        if compat_sources:
            lines = content.splitlines()
            last_source_idx = -1
            for i, line in enumerate(lines):
                stripped = line.strip()
                if (stripped.startswith("Source") or
                    stripped.startswith("Patch")) and ":" in stripped:
                    last_source_idx = i

            if last_source_idx >= 0:
                # Insert after last Source/Patch line
                for src_line in compat_sources.splitlines():
                    last_source_idx += 1
                    lines.insert(last_source_idx, src_line)
                content = "\n".join(lines)

        # Inject compat prep commands (after %setup)
        if compat_prep:
            content = re.sub(
                r"^(%setup\s+.*)$",
                f"\\1\n\n{compat_prep}",
                content,
                count=1,
                flags=re.MULTILINE,
            )

        # Inject CPPFLAGS for header overlays
        if cppflags:
            # Insert CPPFLAGS export right after %build line
            cppflags_line = f'export CPPFLAGS="{cppflags} $CPPFLAGS"'
            if "%build" in content:
                content = re.sub(
                    r"^(%build)(\s*\n)",
                    f"\\1\\2{cppflags_line}\n",
                    content,
                    count=1,
                    flags=re.MULTILINE,
                )

        # Inject ac_cv overrides (autoconf cache variables)
        if ac_cv_overrides:
            ac_cv_lines = "\n".join(
                f"export {var}={val}" for var, val in ac_cv_overrides.items()
            )
            if "%build" in content:
                content = re.sub(
                    r"^(%build)(\s*\n)",
                    f"\\1\\2# Autoconf cache overrides for cross-compilation\n{ac_cv_lines}\n",
                    content,
                    count=1,
                    flags=re.MULTILINE,
                )

        # Inject compat build commands (after %build and CPPFLAGS)
        if compat_build:
            if "%build" in content:
                # Find %build section and insert after initial setup
                content = re.sub(
                    r"^(%build\s*\n(?:export [^\n]+\n)*)",
                    f"\\1{compat_build}\n",
                    content,
                    count=1,
                    flags=re.MULTILINE,
                )

        # Clean up empty lines from removals
        content = re.sub(r"\n{3,}", "\n\n", content)

        return content
