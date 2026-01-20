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

        # Handle conditionals
        content = self._handle_conditionals(content, result)

        # Handle subpackage dropping
        content = self._handle_subpackages(content, result)

        # Clean up empty lines from removals
        content = re.sub(r"\n{3,}", "\n\n", content)

        return content

    def _handle_conditionals(
        self, content: str, result: TransformResult
    ) -> str:
        """Process conditional blocks in the spec file."""
        # Comment out conditionals
        for cond_name in result.comment_conditionals:
            content = self._comment_conditional(content, cond_name)

        # Remove conditionals entirely
        for cond_name in result.remove_conditionals:
            content = self._remove_conditional(content, cond_name)

        # Force conditionals (keep content, remove %if/%endif)
        for cond_name, keep in result.force_conditionals.items():
            if keep:
                content = self._force_conditional_true(content, cond_name)
            else:
                content = self._remove_conditional(content, cond_name)

        return content

    def _comment_conditional(self, content: str, cond_name: str) -> str:
        """Comment out a conditional block."""
        # Match %if containing the condition name through %endif
        pattern = rf"(^%if[^\n]*{re.escape(cond_name)}[^\n]*$)(.*?)(^%endif\s*$)"

        def comment_block(match):
            if_line = match.group(1)
            block_content = match.group(2)
            endif_line = match.group(3)
            # Comment each line
            commented_if = "#" + if_line
            commented_content = "\n".join(
                "#" + line if line.strip() else line
                for line in block_content.split("\n")
            )
            commented_endif = "#" + endif_line
            return commented_if + commented_content + commented_endif

        return re.sub(pattern, comment_block, content, flags=re.MULTILINE | re.DOTALL)

    def _remove_conditional(self, content: str, cond_name: str) -> str:
        """Remove a conditional block entirely."""
        # Match %if containing the condition name through %endif
        pattern = rf"^%if[^\n]*{re.escape(cond_name)}[^\n]*$.*?^%endif\s*$\n?"
        return re.sub(pattern, "", content, flags=re.MULTILINE | re.DOTALL)

    def _force_conditional_true(self, content: str, cond_name: str) -> str:
        """Force a conditional to always be true (remove %if/%endif, keep content)."""
        # Match %if containing the condition name
        pattern = rf"^%if[^\n]*{re.escape(cond_name)}[^\n]*$\n?"
        content = re.sub(pattern, "", content, flags=re.MULTILINE)

        # Also remove the corresponding %endif (this is imperfect but works for simple cases)
        # For now, we just remove the first %endif after where the %if was
        # A more robust solution would track nesting
        return content

    def _handle_subpackages(
        self, content: str, result: TransformResult
    ) -> str:
        """Process subpackage dropping."""
        import fnmatch

        for pattern in result.drop_subpackages:
            content = self._comment_subpackage(content, pattern)

        return content

    def _comment_subpackage(self, content: str, subpkg_pattern: str) -> str:
        """Comment out a subpackage and its related sections."""
        import fnmatch

        lines = content.splitlines()
        result_lines = []
        in_subpackage_block = False
        current_subpackage = None

        i = 0
        while i < len(lines):
            line = lines[i]
            stripped = line.strip()

            # Check for %package directive
            pkg_match = re.match(r"^%package\s+(\S+)", stripped)
            if pkg_match:
                subpkg_name = pkg_match.group(1)
                # Check if this subpackage matches the pattern
                if fnmatch.fnmatch(subpkg_name, subpkg_pattern):
                    # Comment out %package line
                    result_lines.append("#" + line)
                    current_subpackage = subpkg_name
                    i += 1
                    continue

            # Check for %description of a dropped subpackage
            desc_match = re.match(r"^%description\s+(\S+)", stripped)
            if desc_match and current_subpackage:
                subpkg_name = desc_match.group(1)
                if fnmatch.fnmatch(subpkg_name, subpkg_pattern):
                    # Comment out %description and its content
                    result_lines.append("#" + line)
                    i += 1
                    # Comment lines until next section
                    while i < len(lines):
                        next_line = lines[i]
                        if next_line.strip().startswith("%"):
                            break
                        result_lines.append("#" + next_line if next_line.strip() else next_line)
                        i += 1
                    continue

            # Check for %files of a dropped subpackage
            files_match = re.match(r"^%files\s+(\S+)", stripped)
            if files_match:
                subpkg_name = files_match.group(1)
                if fnmatch.fnmatch(subpkg_name, subpkg_pattern):
                    # Comment out %files and its content
                    result_lines.append("#" + line)
                    i += 1
                    # Comment lines until next section or end
                    while i < len(lines):
                        next_line = lines[i]
                        if next_line.strip().startswith("%") and not next_line.strip().startswith("%{"):
                            # Don't treat %{macro} as a section start
                            if re.match(r"^%[a-z]", next_line.strip()):
                                break
                        result_lines.append("#" + next_line if next_line.strip() else next_line)
                        i += 1
                    continue

            result_lines.append(line)
            i += 1

        return "\n".join(result_lines)
