"""Spec file writer for mogrix."""

import re
from fnmatch import fnmatch, translate as fnmatch_translate
from pathlib import Path

from mogrix.rules.engine import TransformResult

# Calculate MOGRIX_ROOT from this file's location
# This allows $MOGRIX_ROOT/tools/... to work in specs
MOGRIX_ROOT = str(Path(__file__).parent.parent.parent.resolve())


class SpecWriter:
    """Writes modified spec file content."""

    def write(
        self,
        result: TransformResult,
        drops: list[str] | None = None,
        adds: list[str] | None = None,
        add_requires: list[str] | None = None,
        cppflags: str | None = None,
        compat_sources: str | None = None,
        compat_prep: str | None = None,
        compat_build: str | None = None,
        patch_sources: str | None = None,
        patch_prep: str | None = None,
        extra_sources: str | None = None,
        ac_cv_overrides: dict[str, str] | None = None,
        drop_requires: list[str] | None = None,
        remove_lines: list[str] | None = None,
        rpm_macros: dict[str, str] | None = None,
        export_vars: dict[str, str] | None = None,
        skip_find_lang: bool = False,
        skip_check: bool = False,
        install_cleanup: list[str] | None = None,
        spec_replacements: list[dict[str, str]] | None = None,
    ) -> str:
        """Generate modified spec content from transform result."""
        content = result.spec.raw_content
        drops = drops or []
        adds = adds or []

        # Inject RPM macros at the top (replaces sgug-rpm-config)
        if rpm_macros:
            macro_lines = ["# IRIX/SGUG path macros (injected by mogrix)"]
            for name, value in rpm_macros.items():
                macro_lines.append(f"%define {name} {value}")
            # Disable automatic dependency detection for cross-compiled packages
            # IRIX system libraries aren't in the RPM database
            macro_lines.append("")
            macro_lines.append("# Disable auto-deps for cross-compilation")
            macro_lines.append("AutoReq: no")
            macro_lines.append("AutoProv: no")
            macro_lines.append("")
            content = "\n".join(macro_lines) + content

        # Apply spec file text replacements (for fixing macros, etc.)
        if spec_replacements:
            for replacement in spec_replacements:
                pattern = replacement.get("pattern", "")
                repl = replacement.get("replacement", "")
                if pattern:
                    content = content.replace(pattern, repl)

        # Remove dropped BuildRequires
        # Separate exact names from glob patterns
        exact_drops = [d for d in drops if "*" not in d and "?" not in d]
        glob_drops = [d for d in drops if "*" in d or "?" in d]

        # Handle exact drops with targeted regex
        for dep in exact_drops:
            escaped_dep = re.escape(dep)
            # Pattern 1: Single-package BuildRequires line
            # Matches: BuildRequires: pkg, BuildRequires: pkg >= 1.0, BuildRequires: pkg%{_isa}
            pattern = rf"^BuildRequires:\s*{escaped_dep}(%\{{[^}}]+\}})?(\s*[<>=].*)?$"
            content = re.sub(pattern, "", content, flags=re.MULTILINE)

            # Pattern 2: Package in a multi-package BuildRequires line
            def remove_from_multi_buildrequires(match, escaped=escaped_dep):
                prefix = match.group(1)  # "BuildRequires:"
                packages = match.group(2)
                pkg_pattern = rf"(?:^|\s){escaped}(?![a-zA-Z0-9_-])(%\{{[^}}]+\}})?(\s*[<>=]+\s*[\d.]+)?"
                new_packages = re.sub(pkg_pattern, " ", packages).strip()
                new_packages = re.sub(r"\s+", " ", new_packages)
                if not new_packages:
                    return ""
                return f"{prefix} {new_packages}"

            pattern_multi = rf"^(BuildRequires:)\s+(.+(?:^|\s){escaped_dep}(?![a-zA-Z0-9_-]).*)$"
            content = re.sub(pattern_multi, remove_from_multi_buildrequires, content, flags=re.MULTILINE)

        # Handle glob drops — match BuildRequires lines where the package name matches the glob
        for glob_pat in glob_drops:
            # Convert glob to regex (e.g., "rust-*" -> "rust\\-.*")
            glob_re = fnmatch_translate(glob_pat).rstrip("\\Z$").rstrip(")")
            # fnmatch_translate returns "(?s:rust\\-.*)\\Z" — extract the inner pattern
            inner = re.match(r"\(\?s:(.*)\)", fnmatch_translate(glob_pat))
            if inner:
                dep_re = inner.group(1).rstrip("\\Z")
            else:
                dep_re = re.escape(glob_pat).replace(r"\*", ".*").replace(r"\?", ".")
            # Single-package line
            pattern = rf"^BuildRequires:\s*{dep_re}(%\{{[^}}]+\}})?(\s*[<>=].*)?$"
            content = re.sub(pattern, "", content, flags=re.MULTILINE)

            # Multi-package line: remove matching package from the line
            def remove_glob_from_multi(match, dep_regex=dep_re):
                prefix = match.group(1)
                packages = match.group(2)
                pkg_pattern = rf"(?:^|\s)({dep_regex})(?![a-zA-Z0-9_-])(%\{{[^}}]+\}})?(\s*[<>=]+\s*[\d.]+)?"
                new_packages = re.sub(pkg_pattern, " ", packages).strip()
                new_packages = re.sub(r"\s+", " ", new_packages)
                if not new_packages:
                    return ""
                return f"{prefix} {new_packages}"

            # Match any BuildRequires line containing text that could match our glob
            content = re.sub(
                r"^(BuildRequires:)\s+(.+)$",
                lambda m: remove_glob_from_multi(m) if fnmatch(m.group(2).split()[0].split("%")[0], glob_pat) else m.group(0),
                content,
                flags=re.MULTILINE,
            )

        # Remove dropped Requires
        if drop_requires:
            for dep in drop_requires:
                escaped_dep = re.escape(dep)
                # Match all Requires variants: Requires:, Requires(pre):, Requires(post):, etc.
                # Also handle %{_isa} suffix and version constraints
                # Pattern 1: Single-package Requires line (with optional scriptlet qualifier)
                # Matches: Requires: pkg, Requires(pre): pkg, Requires: pkg%{_isa} >= 1.0
                # Handle: pkg, pkg%{_isa}, pkg(%{version}), pkg(%{version}) >= 1.0
                pattern = rf"^Requires(\([^)]+\))?:\s*{escaped_dep}(\([^)]*\))?(%\{{[^}}]+\}})?(\s*[<>=].*)?$"
                content = re.sub(pattern, "", content, flags=re.MULTILINE)

                # Pattern 2: Package in a multi-package Requires line
                # Remove just that package from the line, preserving others
                # Matches: "Requires: foo bar pkg baz" -> "Requires: foo bar baz"
                def remove_from_multi_requires(match):
                    full_line = match.group(0)
                    prefix = match.group(1)  # "Requires:" or "Requires(pre):" etc
                    packages = match.group(2)
                    # Remove the specific package (with optional version constraint)
                    # Handle: pkg, pkg >= 1.0, pkg%{_isa}, pkg%{_isa} >= 1.0
                    # Use negative lookahead (?![a-zA-Z0-9_-]) to avoid matching pkg in pkg-devel
                    pkg_pattern = rf"(?:^|\s){escaped_dep}(?![a-zA-Z0-9_-])(%\{{[^}}]+\}})?(\s*[<>=]+\s*[\d.]+)?"
                    new_packages = re.sub(pkg_pattern, " ", packages).strip()
                    # Clean up multiple spaces
                    new_packages = re.sub(r"\s+", " ", new_packages)
                    if not new_packages:
                        return ""  # Remove entire line if no packages left
                    return f"{prefix} {new_packages}"

                # Match Requires lines that contain multiple packages
                # Use word boundary to avoid matching pkg in pkg-devel
                pattern_multi = rf"^(Requires(?:\([^)]+\))?:)\s+(.+(?:^|\s){escaped_dep}(?![a-zA-Z0-9_-]).*)$"
                content = re.sub(pattern_multi, remove_from_multi_requires, content, flags=re.MULTILINE)

        # Remove specific lines (substring match - removes lines containing pattern)
        if remove_lines:
            for line_pattern in remove_lines:
                # Escape and match lines containing the pattern
                escaped = re.escape(line_pattern)
                content = re.sub(rf"^.*{escaped}.*$\n?", "", content, flags=re.MULTILINE)

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

        # Add new Requires (after Name: line)
        # Cross-compiled packages have AutoReq: no, so dependencies must be explicit
        if add_requires:
            lines = content.splitlines()
            for i, line in enumerate(lines):
                if line.strip().startswith("Name:"):
                    # Insert after Name line (in reverse order so first dep ends up first)
                    for dep in reversed(add_requires):
                        lines.insert(i + 1, f"Requires: {dep}")
                    break
            content = "\n".join(lines)

        # Inject configure --disable flags
        if result.configure_disable:
            flags = " ".join(f"--disable-{f}" for f in result.configure_disable)
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
                # Use lookahead to ensure we match the WHOLE flag, not a prefix
                # e.g., --enable-jit must not match inside --enable-jit-sealloc
                content = re.sub(
                    rf"\s*{escaped}(?=[=\s\\]|$)(=[^\s]+)?",
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

        # Escape commented-out %configure lines to prevent RPM macro expansion.
        # RPM expands macros BEFORE shell processing, so #%configure still
        # expands the multi-line %configure macro. The \ continuations mean
        # only the first line is commented — the rest executes, causing the
        # "commented" configure to actually run (and fail/conflict).
        # %%configure is a literal % in RPM specs, preventing expansion.
        content = re.sub(
            r"^#(%configure\b)",
            r"#%\1",
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
                if (
                    stripped.startswith("Source") or stripped.startswith("Patch")
                ) and ":" in stripped:
                    last_source_idx = i

            if last_source_idx >= 0:
                # Insert after last Source/Patch line
                for src_line in compat_sources.splitlines():
                    last_source_idx += 1
                    lines.insert(last_source_idx, src_line)
                content = "\n".join(lines)

        # Inject extra Source entries (after last Source/Patch line)
        if extra_sources:
            lines = content.splitlines()
            last_source_idx = -1
            for i, line in enumerate(lines):
                stripped = line.strip()
                if (
                    stripped.startswith("Source") or stripped.startswith("Patch")
                ) and ":" in stripped:
                    last_source_idx = i

            if last_source_idx >= 0:
                lines.insert(last_source_idx + 1, "# Mogrix extra sources")
                last_source_idx += 1
                for src_line in extra_sources.splitlines():
                    last_source_idx += 1
                    lines.insert(last_source_idx, src_line)
                content = "\n".join(lines)

        # Inject Patch entries
        # If %patchlist exists, add patch filenames to end of patchlist
        # (so they're applied AFTER other patchlist patches)
        # Otherwise, add as PatchN: tags after last Source/Patch line
        if patch_sources:
            lines = content.splitlines()
            has_patchlist = any("%patchlist" in line for line in lines)

            if has_patchlist:
                # Find %patchlist and the next section after it
                patchlist_idx = -1
                patchlist_end_idx = -1
                for i, line in enumerate(lines):
                    if "%patchlist" in line:
                        patchlist_idx = i
                    elif patchlist_idx >= 0 and line.strip().startswith("%"):
                        # Found next section (e.g., %prep, %description)
                        patchlist_end_idx = i
                        break

                if patchlist_idx >= 0:
                    # Find last non-empty line before next section
                    if patchlist_end_idx < 0:
                        patchlist_end_idx = len(lines)
                    insert_idx = patchlist_end_idx
                    # Work backwards to find last non-blank line in patchlist
                    for i in range(patchlist_end_idx - 1, patchlist_idx, -1):
                        if lines[i].strip() and not lines[i].strip().startswith("#"):
                            insert_idx = i + 1
                            break

                    # Extract just patch filenames from PatchN: lines
                    patch_filenames = []
                    for patch_line in patch_sources.splitlines():
                        if patch_line.strip():
                            # Extract filename from "Patch200: filename.patch"
                            if ":" in patch_line:
                                patch_filenames.append(patch_line.split(":", 1)[1].strip())
                            else:
                                patch_filenames.append(patch_line.strip())

                    # Insert patch filenames into patchlist
                    lines.insert(insert_idx, "# Mogrix IRIX compatibility patches")
                    for pf in patch_filenames:
                        insert_idx += 1
                        lines.insert(insert_idx, pf)
                    content = "\n".join(lines)
            else:
                # No patchlist - use Patch tags
                last_source_idx = -1
                for i, line in enumerate(lines):
                    stripped = line.strip()
                    if (
                        stripped.startswith("Source") or stripped.startswith("Patch")
                    ) and ":" in stripped:
                        last_source_idx = i

                if last_source_idx >= 0:
                    # Insert after last Source/Patch line with a comment
                    lines.insert(last_source_idx + 1, "# Mogrix patches (IRIX compatibility)")
                    last_source_idx += 1
                    for patch_line in patch_sources.splitlines():
                        last_source_idx += 1
                        lines.insert(last_source_idx, patch_line)
                    content = "\n".join(lines)

        # Create .origfedora copy for patch development
        # This goes FIRST, before any patches or modifications are applied
        origfedora_cmd = """
# Create .origfedora copy for patch development (mkpatch workflow)
# Run 'rpmbuild --short-circuit -bp' then diff against .origfedora to create patches
# Use subshell to avoid directory change on failure; exclude .git (mode 0444 objects)
_mogrix_origdir=$(pwd)
(cd .. && cp -a "$(basename "$_mogrix_origdir")" "$(basename "$_mogrix_origdir").origfedora" 2>/dev/null || true)
"""
        content = re.sub(
            r"^(%(?:auto)?setup(?:[ \t]+.*)?)$",
            f"\\1\n{origfedora_cmd}",
            content,
            count=1,
            flags=re.MULTILINE,
        )

        # Inject compat prep commands (after %setup or %autosetup)
        if compat_prep:
            content = re.sub(
                r"^(%(?:auto)?setup(?:[ \t]+.*)?)$",
                f"\\1\n\n{compat_prep}",
                content,
                count=1,
                flags=re.MULTILINE,
            )

        # Inject patch application commands (after %setup)
        # NOTE: Skip if spec uses %autosetup - it applies all patches automatically
        if patch_prep:
            uses_autosetup = bool(re.search(r"^%autosetup\b", content, re.MULTILINE))
            if not uses_autosetup:
                patch_comment = "# Apply mogrix patches"
                content = re.sub(
                    r"^(%setup(?:[ \t]+.*)?)$",
                    lambda m: f"{m.group(0)}\n\n{patch_comment}\n{patch_prep}",
                    content,
                    count=1,
                    flags=re.MULTILINE,
                )

        # Inject custom prep commands (e.g., sed for Makefile fixes)
        if result.prep_commands:
            prep_cmds = "\n".join(result.prep_commands)
            prep_comment = "# Cross-compilation prep fixes (injected by mogrix)"
            content = re.sub(
                r"^(%(?:auto)?setup(?:[ \t]+.*)?)$",
                lambda m: f"{m.group(0)}\n\n{prep_comment}\n{prep_cmds}",
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
                f'export {var}="{val}"' for var, val in ac_cv_overrides.items()
            )
            if "%build" in content:
                content = re.sub(
                    r"^(%build)(\s*\n)",
                    f"\\1\\2# Autoconf cache overrides for cross-compilation\n{ac_cv_lines}\n",
                    content,
                    count=1,
                    flags=re.MULTILINE,
                )

        # Inject export_vars (e.g., LD for libtool)
        # Always include MOGRIX_ROOT so helper scripts can be found
        all_export_vars = {"MOGRIX_ROOT": MOGRIX_ROOT}
        if export_vars:
            all_export_vars.update(export_vars)
        if all_export_vars:
            export_lines = "\n".join(
                f'export {var}="{val}"' for var, val in all_export_vars.items()
            )
            if "%build" in content:
                content = re.sub(
                    r"^(%build)(\s*\n)",
                    f"\\1\\2# Use our IRIX linker wrapper for libtool\n{export_lines}\n",
                    content,
                    count=1,
                    flags=re.MULTILINE,
                )

        # Inject compat build commands (after %build and all exports/comments)
        if compat_build:
            if "%build" in content:
                # Find %build section and insert after initial setup
                # Match export lines AND comment lines (injected by CPPFLAGS/ac_cv/export_vars)
                content = re.sub(
                    r"^(%build\s*\n(?:(?:export |# )[^\n]*\n)*)",
                    f"\\1{compat_build}\n",
                    content,
                    count=1,
                    flags=re.MULTILINE,
                )

        # Handle conditionals
        content = self._handle_conditionals(content, result)

        # Handle subpackage dropping
        content = self._handle_subpackages(content, result)

        # Skip find_lang (for packages with NLS disabled)
        if skip_find_lang:
            # Comment out %find_lang lines
            content = re.sub(
                r"^(%find_lang\s+.*)$",
                r"# Skip find_lang for cross-compilation (NLS disabled)\n#\1",
                content,
                flags=re.MULTILINE,
            )
            # Remove -f lang references from %files
            content = re.sub(
                r"^(%files.*)\s+-f\s+\S+\.lang(.*)$",
                r"\1\2",
                content,
                flags=re.MULTILINE,
            )

        # Skip check section (for cross-compilation where tests can't run)
        if skip_check:
            # Comment out the contents of %check section
            # Match from %check to the next section (%install, %files, %post, %pre, %changelog, etc.)
            def comment_check_section(match):
                check_line = match.group(1)
                section_content = match.group(2)
                next_section = match.group(3)
                # Comment out the content, but keep the %check marker
                # Preserve %if* and %endif* lines (RPM conditionals must remain)
                commented_lines = []
                for line in section_content.split("\n"):
                    stripped = line.strip()
                    if stripped.startswith("%if") or stripped.startswith("%endif"):
                        # Keep RPM conditionals uncommented
                        commented_lines.append(line)
                    elif stripped:
                        commented_lines.append("# " + line)
                    else:
                        commented_lines.append(line)
                commented_content = "\n".join(commented_lines)
                return f"{check_line}\n# Tests skipped for cross-compilation (binaries can't run on host)\n{commented_content}{next_section}"

            content = re.sub(
                r"^(%check)\s*\n(.*?)(^%(?:install|files|pre|post|preun|postun|changelog|package|description)\b)",
                comment_check_section,
                content,
                flags=re.MULTILINE | re.DOTALL,
            )

        # Install cleanup commands (e.g., remove .la files)
        if install_cleanup:
            cleanup_cmds = "\n".join(install_cleanup)
            cleanup_comment = "# Install cleanup (injected by mogrix)"
            # Try patterns in order of preference
            patterns = [
                r"^(%make_install)(\s*)$",  # Standard autotools
                r"^(%cmake_install)(\s*)$",  # cmake macro
                r"^(make install DESTDIR=%\{buildroot\})(\s*)$",  # Direct make install
            ]
            inserted = False
            for pattern in patterns:
                new_content = re.sub(
                    pattern,
                    lambda m: f"{m.group(1)}{m.group(2)}\n\n{cleanup_comment}\n{cleanup_cmds}",
                    content,
                    count=1,
                    flags=re.MULTILINE,
                )
                if new_content != content:
                    content = new_content
                    inserted = True
                    break
            # Fallback: insert at end of %install section (before next section)
            if not inserted:
                content = re.sub(
                    r"(^%install\s*\n.*?)(\n^%(?:files|pre|post|preun|postun|changelog|package|check)\b)",
                    lambda m: f"{m.group(1)}\n\n{cleanup_comment}\n{cleanup_cmds}{m.group(2)}",
                    content,
                    count=1,
                    flags=re.MULTILINE | re.DOTALL,
                )

        # Clean up empty lines from removals
        content = re.sub(r"\n{3,}", "\n\n", content)

        return content

    def _handle_conditionals(self, content: str, result: TransformResult) -> str:
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

    def _handle_subpackages(self, content: str, result: TransformResult) -> str:
        """Process subpackage dropping."""

        for pattern in result.drop_subpackages:
            content = self._comment_subpackage(content, pattern)
        content = self._comment_orphaned_conditionals(content)

        return content

    def _comment_subpackage(self, content: str, subpkg_pattern: str) -> str:
        """Comment out a subpackage and its related sections."""
        import fnmatch

        lines = content.splitlines()
        result_lines = []
        current_subpackage = None

        i = 0
        while i < len(lines):
            line = lines[i]
            stripped = line.strip()

            # Check for %package directive
            # Handle both "%package foo" (suffix) and "%package -n foo" (full name)
            pkg_match = re.match(r"^%package\s+(?:-n\s+)?(\S+)", stripped)
            if pkg_match:
                subpkg_name = pkg_match.group(1)
                # Check if this subpackage matches the pattern
                if fnmatch.fnmatch(subpkg_name, subpkg_pattern):
                    # Comment out %package line and its metadata
                    # (Summary, Requires, Provides, etc. until next section)
                    result_lines.append("#" + line)
                    current_subpackage = subpkg_name
                    i += 1
                    _pkg_section_markers = re.compile(
                        r"^%(package|description|files|prep|build|install|"
                        r"check|pre|post|preun|postun|pretrans|posttrans|"
                        r"changelog|clean|verifyscript)\b"
                    )
                    while i < len(lines):
                        next_line = lines[i]
                        next_stripped = next_line.strip()
                        if _pkg_section_markers.match(next_stripped):
                            break
                        result_lines.append(
                            "#" + next_line if next_line.strip() else next_line
                        )
                        i += 1
                    continue

            # Check for %description of a dropped subpackage
            desc_match = re.match(r"^%description\s+(?:-n\s+)?(\S+)", stripped)
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
                        result_lines.append(
                            "#" + next_line if next_line.strip() else next_line
                        )
                        i += 1
                    continue

            # Check for %files of a dropped subpackage
            files_match = re.match(r"^%files\s+(?:-n\s+)?(\S+)", stripped)
            if files_match:
                subpkg_name = files_match.group(1)
                if fnmatch.fnmatch(subpkg_name, subpkg_pattern):
                    # Comment out %files and its content
                    result_lines.append("#" + line)
                    i += 1
                    # Comment lines until next RPM section or end.
                    # Only actual RPM section markers should stop commenting.
                    # %files directives (%dir, %doc, %license, etc.) and
                    # conditionals (%if, %endif, etc.) are part of %files
                    # content and should be commented too.
                    _section_markers = re.compile(
                        r"^%(files|package|description|prep|build|install|"
                        r"check|pre|post|preun|postun|pretrans|posttrans|"
                        r"changelog|clean|verifyscript)\b"
                    )
                    while i < len(lines):
                        next_line = lines[i]
                        next_stripped = next_line.strip()
                        if _section_markers.match(next_stripped):
                            break
                        result_lines.append(
                            "#" + next_line if next_line.strip() else next_line
                        )
                        i += 1
                    continue

            result_lines.append(line)
            i += 1

        return "\n".join(result_lines)

    def _comment_orphaned_conditionals(self, content: str) -> str:
        """Comment out %if/%endif blocks whose content is all commented or empty.

        When subpackage sections are wrapped in %if 1 / %endif, commenting
        the inner sections can leave orphaned %if (the %endif may already be
        commented by the %files handler, or left uncommented by the %description
        handler). This post-pass finds %if blocks where all non-empty content
        lines are already commented, and comments both %if and %endif.
        """
        lines = content.splitlines()
        result = list(lines)

        i = 0
        while i < len(lines):
            stripped = lines[i].strip()
            if re.match(r"^%if\b", stripped):
                # Find matching %endif, tracking nesting.
                # %endif may already be commented (#%endif) by inner handlers.
                depth = 1
                j = i + 1
                all_commented = True
                endif_idx = None
                while j < len(lines) and depth > 0:
                    s = lines[j].strip()
                    # Check for %if/%endif (both commented and uncommented)
                    bare = s.lstrip("#").strip()
                    if re.match(r"^%if\b", bare) and not s.startswith("#"):
                        # Only count uncommented %if as nesting
                        depth += 1
                    elif bare == "%endif":
                        depth -= 1
                        if depth == 0:
                            endif_idx = j
                            break
                    elif s and not s.startswith("#"):
                        all_commented = False
                    j += 1

                if endif_idx is not None and all_commented:
                    result[i] = "#" + lines[i]
                    # %endif may already be commented
                    if not lines[endif_idx].strip().startswith("#"):
                        result[endif_idx] = "#" + lines[endif_idx]
            i += 1

        return "\n".join(result)
