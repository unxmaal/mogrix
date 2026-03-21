"""Spec file writer for mogrix."""

import logging
import re
from dataclasses import dataclass, field
from fnmatch import fnmatch, translate as fnmatch_translate
from pathlib import Path

from mogrix.parser.sections import split_spec_sections, reassemble_spec, find_sections
from mogrix.rules.engine import TransformResult

logger = logging.getLogger(__name__)


@dataclass
class ReplacementMatch:
    """Tracking info for a single spec_replacement application."""

    pattern: str  # first 80 chars of the pattern
    matched: bool
    optional: bool = False
    is_regex: bool = False


@dataclass
class WriteResult:
    """Result of writing a spec file, with match tracking."""

    content: str
    unmatched: list[ReplacementMatch] = field(default_factory=list)
    all_matches: list[ReplacementMatch] = field(default_factory=list)

    @property
    def unmatched_required(self) -> list[ReplacementMatch]:
        """Non-optional replacements that didn't match."""
        return [m for m in self.unmatched if not m.optional]

class SpecReplacementError(Exception):
    """Raised in strict mode when required spec_replacements don't match."""


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
        extra_cflags: list[str] | None = None,
        skip_find_lang: bool = False,
        skip_check: bool = False,
        install_cleanup: list[str] | None = None,
        spec_replacements: list[dict[str, str]] | None = None,
        strict: bool = False,
    ) -> WriteResult:
        """Generate modified spec content from transform result.

        Returns a WriteResult containing the content and match tracking info.
        Callers that only need the string can use write_result.content.
        """
        content = result.spec.raw_content
        drops = drops or []
        adds = adds or []
        write_result = WriteResult(content="")

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

        # Apply spec file text replacements (Phase 1: match confirmation, Phase 4: regex)
        if spec_replacements:
            for replacement in spec_replacements:
                pattern = replacement.get("pattern", "")
                pattern_regex = replacement.get("pattern_regex", "")
                repl = replacement.get("replacement", "")
                optional = replacement.get("optional", False)
                section = replacement.get("section", "")
                is_regex = bool(pattern_regex)
                search_key = pattern_regex or pattern

                if not search_key:
                    continue

                display = search_key[:80]
                old_content = content

                if is_regex:
                    if section:
                        content = self._apply_regex_replacement_in_section(
                            content, pattern_regex, repl, section
                        )
                    else:
                        content = re.sub(pattern_regex, repl, content, flags=re.MULTILINE)
                else:
                    if section:
                        content = self._apply_literal_replacement_in_section(
                            content, pattern, repl, section
                        )
                    else:
                        content = content.replace(pattern, repl)

                matched = content != old_content
                match_info = ReplacementMatch(
                    pattern=display, matched=matched, optional=optional, is_regex=is_regex
                )
                write_result.all_matches.append(match_info)
                if not matched:
                    write_result.unmatched.append(match_info)
                    if optional:
                        logger.debug(
                            "spec_replacement%s (optional) did not match: %s",
                            " regex" if is_regex else "",
                            display,
                        )
                    else:
                        logger.warning(
                            "spec_replacement%s did not match: %s",
                            " regex" if is_regex else "",
                            display,
                        )

        # Apply comment_matching (Phase 2a)
        if result.comment_matching:
            content = self._apply_comment_matching(content, result.comment_matching)

        # Apply remove_matching (Phase 2a)
        if result.remove_matching:
            content = self._apply_remove_matching(content, result.remove_matching)

        # Apply flip_globals (Phase 2c)
        if result.flip_globals:
            content = self._apply_flip_globals(content, result.flip_globals)

        # Apply drop_patches (Phase 2b)
        if result.drop_patches:
            content = self._apply_drop_patches(content, result.drop_patches)

        # Handle conditionals early — MUST run before compat/extra/patch Source
        # injection. remove_conditionals deletes %if...%endif blocks that may
        # contain Source lines; if compat Source entries are injected first they
        # land inside the conditional and get deleted along with it.
        content = self._handle_conditionals(content, result)

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
                # Match dep at start of string OR after whitespace
                pkg_pattern = rf"(?:(?<=\s)|^){escaped}(?![a-zA-Z0-9_-])(%\{{[^}}]+\}})?(\s*[<>=]+\s*[\d.]+)?"
                new_packages = re.sub(pkg_pattern, "", packages).strip()
                new_packages = re.sub(r"\s+", " ", new_packages)
                if not new_packages:
                    return ""
                return f"{prefix} {new_packages}"

            # Match any BuildRequires line (callback checks for the specific dep)
            pattern_multi = rf"^(BuildRequires:)\s+(.+)$"
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
                # Match dep at start of string OR after whitespace
                pkg_pattern = rf"(?:(?<=\s)|^)({dep_regex})(?![a-zA-Z0-9_-])(%\{{[^}}]+\}})?(\s*[<>=]+\s*[\d.]+)?"
                new_packages = re.sub(pkg_pattern, "", packages).strip()
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

        # Remove dropped Requires (includes PreReq: handling)
        if drop_requires:
            for dep in drop_requires:
                escaped_dep = re.escape(dep)
                # Match all Requires variants: Requires:, Requires(pre):, Requires(post):, PreReq:
                # Also handle %{_isa} suffix and version constraints
                # Pattern 1: Single-package Requires/PreReq line (with optional scriptlet qualifier)
                pattern = rf"^(?:Requires(\([^)]+\))?|PreReq):\s*{escaped_dep}(\([^)]*\))?(%\{{[^}}]+\}})?(\s*[<>=].*)?$"
                content = re.sub(pattern, "", content, flags=re.MULTILINE)

                # Pattern 2: Package in a multi-package Requires/PreReq line
                def remove_from_multi_requires(match, escaped=escaped_dep):
                    prefix = match.group(1)  # "Requires:", "Requires(pre):", "PreReq:" etc
                    packages = match.group(2)
                    # Match dep at start of string OR after whitespace
                    pkg_pattern = rf"(?:(?<=\s)|^){escaped}(?![a-zA-Z0-9_-])(%\{{[^}}]+\}})?(\s*[<>=]+\s*[\d.]+)?"
                    new_packages = re.sub(pkg_pattern, "", packages).strip()
                    new_packages = re.sub(r"\s+", " ", new_packages)
                    if not new_packages:
                        return ""
                    return f"{prefix} {new_packages}"

                # Match any Requires/PreReq line (callback checks for the specific dep)
                pattern_multi = rf"^((?:Requires(?:\([^)]+\))?|PreReq):)\s+(.+)$"
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

        # Remove configure flags (conditional-aware)
        if result.configure_flags_remove:
            for flag in result.configure_flags_remove:
                escaped = re.escape(flag)
                flag_re = re.compile(rf"\s*{escaped}(?=[=\s\\]|$)(=[^\s]+)?")
                # Check if the flag appears inside a %if/%else/%endif block.
                # If so, skip removal and warn — the text engine can't safely
                # restructure conditional blocks. Use bcond flipping instead.
                lines = content.splitlines()
                cond_depth = 0
                in_conditional = {}  # line_idx -> bool
                for idx, line in enumerate(lines):
                    s = line.strip()
                    if re.match(r"^%if\b", s):
                        cond_depth += 1
                    in_conditional[idx] = cond_depth > 0
                    if re.match(r"^%endif\b", s):
                        cond_depth -= 1

                flag_in_cond = False
                for idx, line in enumerate(lines):
                    if flag_re.search(line) and in_conditional.get(idx, False):
                        flag_in_cond = True
                        break

                if flag_in_cond:
                    logger.warning(
                        "configure_flags remove: '%s' is inside %%if/%%endif block, "
                        "skipping (use bcond flipping or spec_replacements instead)",
                        flag,
                    )
                else:
                    content = flag_re.sub("", content)

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
        # Build all %prep injections into a single block, then inject once.
        # Each injection previously did its own regex substitution on %setup/%autosetup,
        # but each substitution consumed the match (count=1), making subsequent ones fail.
        # Now we build the full block and inject it in one operation.
        prep_injection_parts = []

        # 1. origfedora copy (always)
        prep_injection_parts.append(origfedora_cmd)

        # 2. MOGRIX_ROOT export (always — needed by safepatch in prep_commands)
        mogrix_root_export = f'export MOGRIX_ROOT="{MOGRIX_ROOT}"'
        prep_injection_parts.append(mogrix_root_export)

        # 3. Compat prep commands (compat function sources)
        if compat_prep:
            prep_injection_parts.append(compat_prep)

        # 4. Patch application commands (only for %setup, not %autosetup)
        if patch_prep:
            uses_autosetup = bool(re.search(r"^%autosetup\b", content, re.MULTILINE))
            if not uses_autosetup:
                patch_comment = "# Apply mogrix patches"
                prep_injection_parts.append(f"{patch_comment}\n{patch_prep}")

        # 5. Custom prep commands (safepatch, sed, etc.)
        if result.prep_commands:
            prep_cmds = "\n".join(result.prep_commands)
            prep_comment = "# Cross-compilation prep fixes (injected by mogrix)"
            prep_injection_parts.append(f"{prep_comment}\n{prep_cmds}")

        # Single injection after %setup or %autosetup
        prep_block = "\n\n".join(prep_injection_parts)
        # Escape backslashes in prep_block so re.sub doesn't interpret them
        # as regex group references (e.g. \w in perl commands)
        prep_block_escaped = prep_block.replace("\\", "\\\\")
        content = re.sub(
            r"^(%(?:auto)?setup(?:[ \t]+.*)?)$",
            f"\\1\n{prep_block_escaped}",
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

        # Inject extra CFLAGS
        if extra_cflags:
            cflags_str = " ".join(extra_cflags)
            cflags_line = f'export CFLAGS="{cflags_str} $CFLAGS"'
            if "%build" in content:
                content = re.sub(
                    r"^(%build)(\s*\n)",
                    f"\\1\\2{cflags_line}\n",
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
                    r"(^%install\s*\n.*?)(\n^%(?:files|pre|post|preun|postun|changelog|package|check|trigger|transfiletrigger)\b)",
                    lambda m: f"{m.group(1)}\n\n{cleanup_comment}\n{cleanup_cmds}{m.group(2)}",
                    content,
                    count=1,
                    flags=re.MULTILINE | re.DOTALL,
                )

        # Apply section_replace (Phase 3a)
        if result.section_replace:
            content = self._apply_section_replace(content, result.section_replace)

        # Clean up empty lines from removals
        content = re.sub(r"\n{3,}", "\n\n", content)

        write_result.content = content

        # Phase 5a: strict mode — unmatched required replacements are errors
        if strict and write_result.unmatched_required:
            patterns = [m.pattern for m in write_result.unmatched_required]
            raise SpecReplacementError(
                f"{len(patterns)} spec_replacement(s) did not match: {patterns}"
            )

        return write_result

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
        """Comment out a conditional block, handling nested %if/%endif."""
        escaped = re.escape(cond_name)
        lines = content.splitlines()
        result = list(lines)
        i = 0
        while i < len(lines):
            stripped = lines[i].strip()
            if re.match(rf"^%if.*{escaped}", stripped):
                # Track nesting to find matching %endif
                depth = 1
                j = i + 1
                while j < len(lines) and depth > 0:
                    s = lines[j].strip()
                    if re.match(r"^%if\b", s):
                        depth += 1
                    elif s == "%endif" or re.match(r"^%endif\s", s):
                        depth -= 1
                    j += 1
                # Comment lines i through j-1
                for k in range(i, j):
                    if lines[k].strip():
                        result[k] = "#" + lines[k]
                i = j
                continue
            i += 1
        return "\n".join(result)

    def _remove_conditional(self, content: str, cond_name: str) -> str:
        """Remove a conditional block entirely, handling nested %if/%endif."""
        escaped = re.escape(cond_name)
        lines = content.splitlines(keepends=True)
        result_lines = []
        i = 0
        while i < len(lines):
            stripped = lines[i].strip()
            # Find %if line containing the condition name
            if re.match(rf"^%if.*{escaped}", stripped):
                # Track nesting to find matching %endif
                depth = 1
                j = i + 1
                while j < len(lines) and depth > 0:
                    s = lines[j].strip()
                    if re.match(r"^%if\b", s):
                        depth += 1
                    elif s == "%endif" or re.match(r"^%endif\s", s):
                        depth -= 1
                    j += 1
                # Skip lines i..j-1 (the entire %if...%endif block)
                i = j
                continue
            result_lines.append(lines[i])
            i += 1
        return "".join(result_lines)

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

    # Section markers that delimit subpackage-specific sections
    _SUBPKG_SECTION_RE = re.compile(
        r"^%(package|description|files|pre|post|preun|postun|pretrans|posttrans)\s+"
    )
    _ALL_SECTION_MARKERS = re.compile(
        r"^%(files|package|description|prep|build|install|"
        r"check|pre|post|preun|postun|pretrans|posttrans|"
        r"changelog|clean|verifyscript|trigger|transfiletrigger)\b"
    )

    def _comment_subpackage(self, content: str, subpkg_pattern: str) -> str:
        """Comment out a subpackage and ALL its related sections.

        Phase 2d enhancement: handles %package, %description, %files,
        %pre, %post, %preun, %postun, %pretrans, %posttrans for the
        dropped subpackage.
        """
        import fnmatch as fnmatch_mod

        lines = content.splitlines()
        result_lines = []

        i = 0
        while i < len(lines):
            line = lines[i]
            stripped = line.strip()

            # Check for any subpackage-targeted section directive
            # Matches: %package foo, %description -n foo, %files foo,
            #          %pre foo, %post -n foo, %preun foo, etc.
            section_match = self._SUBPKG_SECTION_RE.match(stripped)
            if section_match:
                # Extract subpackage name (handle -n prefix)
                rest = stripped[section_match.end():].strip()
                if rest.startswith("-n ") or rest.startswith("-n\t"):
                    subpkg_name = rest[3:].strip().split()[0] if len(rest) > 3 else ""
                else:
                    subpkg_name = rest.split()[0] if rest else ""

                if subpkg_name and fnmatch_mod.fnmatch(subpkg_name, subpkg_pattern):
                    # Comment out this section header and all content until next section
                    result_lines.append("#" + line)
                    i += 1
                    while i < len(lines):
                        next_line = lines[i]
                        next_stripped = next_line.strip()
                        if self._ALL_SECTION_MARKERS.match(next_stripped):
                            break
                        result_lines.append(
                            "#" + next_line if next_line.strip() else next_line
                        )
                        i += 1
                    continue

            result_lines.append(line)
            i += 1

        return "\n".join(result_lines)

    # --- Phase 2a: comment_matching / remove_matching ---

    def _apply_comment_matching(
        self, content: str, entries: list[dict[str, str]]
    ) -> str:
        """Comment out lines matching regex patterns, optionally scoped to a section."""
        for entry in entries:
            regex = entry.get("regex", "")
            section = entry.get("section", "")
            comment = entry.get("comment", "")
            if not regex:
                continue

            compiled = re.compile(regex)

            if section:
                sections = split_spec_sections(content)
                for sec in sections:
                    if sec.name == section or sec.marker_line.startswith(section):
                        new_lines = []
                        for line in sec.content.split("\n"):
                            if compiled.search(line):
                                prefix = f"# {comment} " if comment else "# "
                                new_lines.append(f"{prefix}{line}")
                            else:
                                new_lines.append(line)
                        sec.content = "\n".join(new_lines)
                content = reassemble_spec(sections)
            else:
                lines = content.split("\n")
                new_lines = []
                for line in lines:
                    if compiled.search(line):
                        prefix = f"# {comment} " if comment else "# "
                        new_lines.append(f"{prefix}{line}")
                    else:
                        new_lines.append(line)
                content = "\n".join(new_lines)

        return content

    def _apply_remove_matching(
        self, content: str, entries: list[dict[str, str]]
    ) -> str:
        """Remove lines matching regex patterns, optionally scoped to a section."""
        for entry in entries:
            regex = entry.get("regex", "")
            section = entry.get("section", "")
            if not regex:
                continue

            compiled = re.compile(regex)

            if section:
                sections = split_spec_sections(content)
                for sec in sections:
                    if sec.name == section or sec.marker_line.startswith(section):
                        new_lines = [
                            line
                            for line in sec.content.split("\n")
                            if not compiled.search(line)
                        ]
                        sec.content = "\n".join(new_lines)
                content = reassemble_spec(sections)
            else:
                lines = content.split("\n")
                content = "\n".join(
                    line for line in lines if not compiled.search(line)
                )

        return content

    # --- Phase 2b: drop_patches ---

    def _apply_drop_patches(self, content: str, config: dict | str) -> str:
        """Drop patches based on configuration.

        Config can be:
          - "all": drop all patches
          - {"except": [100, 200]}: drop all except listed numbers
          - {"only": [400, 502]}: drop only listed numbers
        """
        if config == "all":
            mode = "all"
            nums: set[int] = set()
        elif isinstance(config, dict):
            if "except" in config:
                mode = "except"
                nums = set(config["except"])
            elif "only" in config:
                mode = "only"
                nums = set(config["only"])
            else:
                return content
        else:
            return content

        def should_drop(patch_num: int) -> bool:
            if mode == "all":
                return True
            elif mode == "except":
                return patch_num not in nums
            elif mode == "only":
                return patch_num in nums
            return False

        lines = content.split("\n")
        result_lines = []
        for line in lines:
            stripped = line.strip()
            # Match Patch<N>: header lines
            patch_header = re.match(r"^Patch(\d+)\s*:", stripped)
            if patch_header and should_drop(int(patch_header.group(1))):
                result_lines.append(f"# {line}")
                continue

            # Match %patch -P <N> application lines
            patch_apply = re.match(r"^%patch\s+-P\s*(\d+)", stripped)
            if patch_apply and should_drop(int(patch_apply.group(1))):
                result_lines.append(f"# {line}")
                continue

            # Match legacy %patch<N> application lines
            patch_legacy = re.match(r"^%patch(\d+)\b", stripped)
            if patch_legacy and should_drop(int(patch_legacy.group(1))):
                result_lines.append(f"# {line}")
                continue

            result_lines.append(line)

        return "\n".join(result_lines)

    # --- Phase 2c: flip_globals ---

    def _apply_flip_globals(self, content: str, globals_list: list[str]) -> str:
        """Flip %global values between 0 and 1."""
        for name in globals_list:
            escaped = re.escape(name)
            # Match %global <name> 0 or 1
            pattern = rf"^(%global\s+{escaped}\s+)([01])(\s*)$"
            match = re.search(pattern, content, re.MULTILINE)
            if match:
                old_val = match.group(2)
                new_val = "0" if old_val == "1" else "1"
                content = re.sub(
                    pattern,
                    rf"\g<1>{new_val}\3",
                    content,
                    flags=re.MULTILINE,
                )
            else:
                logger.warning("flip_globals: %%global %s not found", name)
        return content

    # --- Phase 3a: section_replace ---

    def _apply_section_replace(
        self, content: str, replacements: list[dict[str, str]]
    ) -> str:
        """Replace entire section contents."""
        sections = split_spec_sections(content)
        for repl in replacements:
            target_section = repl.get("section", "")
            new_content = repl.get("content", "")
            if not target_section:
                continue

            matched = False
            for sec in sections:
                if sec.name == target_section and not sec.subpackage:
                    sec.content = new_content
                    matched = True
                    break
            if not matched:
                logger.warning("section_replace: section %s not found", target_section)

        return reassemble_spec(sections)

    # --- Phase 4: section-scoped replacements ---

    def _apply_literal_replacement_in_section(
        self, content: str, pattern: str, replacement: str, section: str
    ) -> str:
        """Apply a literal string replacement only within a specific section."""
        sections = split_spec_sections(content)
        for sec in sections:
            if sec.name == section or sec.marker_line.startswith(section):
                sec.content = sec.content.replace(pattern, replacement)
        return reassemble_spec(sections)

    def _apply_regex_replacement_in_section(
        self, content: str, pattern_regex: str, replacement: str, section: str
    ) -> str:
        """Apply a regex replacement only within a specific section."""
        sections = split_spec_sections(content)
        for sec in sections:
            if sec.name == section or sec.marker_line.startswith(section):
                sec.content = re.sub(
                    pattern_regex, replacement, sec.content, flags=re.MULTILINE
                )
        return reassemble_spec(sections)

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
                # Both commented and uncommented %if/%endif are tracked
                # symmetrically to handle nesting correctly.
                depth = 1
                j = i + 1
                all_commented = True
                endif_idx = None
                while j < len(lines) and depth > 0:
                    s = lines[j].strip()
                    bare = s.lstrip("#").strip()
                    if re.match(r"^%if\b", bare):
                        # Count both commented and uncommented %if for nesting
                        depth += 1
                    elif bare == "%endif" or bare == "%endif #":
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
