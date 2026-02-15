"""Rule application engine for mogrix."""

from dataclasses import dataclass, field
from fnmatch import fnmatch

from mogrix.parser.spec import SpecFile
from mogrix.rules.loader import RuleLoader


def _matches_any_drop(name: str, drops: set[str]) -> bool:
    """Check if name matches any drop entry, supporting glob patterns.

    Entries without wildcards are checked with exact set membership (O(1)).
    Entries containing ``*`` or ``?`` are checked with fnmatch.
    """
    if name in drops:
        return True
    for pattern in drops:
        if ("*" in pattern or "?" in pattern) and fnmatch(name, pattern):
            return True
    return False


@dataclass
class TransformResult:
    """Result of applying rules to a spec file."""

    spec: SpecFile
    applied_rules: list[str] = field(default_factory=list)
    configure_disable: list[str] = field(default_factory=list)
    configure_flags_add: list[str] = field(default_factory=list)
    configure_flags_remove: list[str] = field(default_factory=list)
    header_overlays: list[str] = field(default_factory=list)
    path_rewrites: dict[str, str] = field(default_factory=dict)
    compat_functions: list[str] = field(default_factory=list)
    ac_cv_overrides: dict[str, str] = field(default_factory=dict)
    drop_requires: list[str] = field(default_factory=list)
    remove_lines: list[str] = field(default_factory=list)
    comment_conditionals: list[str] = field(default_factory=list)
    remove_conditionals: list[str] = field(default_factory=list)
    force_conditionals: dict[str, bool] = field(default_factory=dict)
    drop_subpackages: list[str] = field(default_factory=list)
    rpm_macros: dict[str, str] = field(default_factory=dict)
    prep_commands: list[str] = field(default_factory=list)
    export_vars: dict[str, str] = field(default_factory=dict)
    skip_find_lang: bool = False
    skip_check: bool = False
    install_cleanup: list[str] = field(default_factory=list)
    spec_replacements: list[dict[str, str]] = field(default_factory=list)
    warnings: list[str] = field(default_factory=list)
    add_patches: list[str] = field(default_factory=list)  # Patch filenames to add
    add_sources: list[str] = field(default_factory=list)  # Extra source files to add
    add_requires: list[str] = field(default_factory=list)  # Runtime deps to add (cross-compiled pkgs)


class RuleEngine:
    """Applies transformation rules to spec files."""

    def __init__(self, loader: RuleLoader):
        """Initialize with a rule loader."""
        self.loader = loader

    def apply(self, spec: SpecFile) -> TransformResult:
        """Apply all applicable rules to a spec file."""
        # Create a copy of the spec to modify
        result = TransformResult(
            spec=SpecFile(
                name=spec.name,
                version=spec.version,
                release=spec.release,
                summary=spec.summary,
                license=spec.license,
                url=spec.url,
                buildrequires=list(spec.buildrequires),
                requires=list(spec.requires),
                sources=dict(spec.sources),
                patches=dict(spec.patches),
                raw_content=spec.raw_content,
            )
        )

        # Load and apply generic rules
        generic = self.loader.load_generic()
        if generic and "generic" in generic:
            self._apply_generic_rules(result, generic["generic"])

        # Load package rules to check for class membership
        pkg_rules = self.loader.load_package(spec.name)

        # Load and apply class rules (between generic and package)
        if pkg_rules:
            for class_name in pkg_rules.get("classes", []):
                class_rules = self.loader.load_class(class_name)
                if class_rules and "rules" in class_rules:
                    self._apply_class_rules(result, class_rules["rules"], class_name)

        # Apply package-specific rules
        if pkg_rules:
            self._apply_package_rules(result, pkg_rules)

        return result

    def _apply_generic_rules(self, result: TransformResult, rules: dict) -> None:
        """Apply generic rules to the result."""
        # Drop BuildRequires
        if "drop_buildrequires" in rules:
            drops = set(rules["drop_buildrequires"])
            original = result.spec.buildrequires[:]
            result.spec.buildrequires = [
                br for br in result.spec.buildrequires if not _matches_any_drop(br, drops)
            ]
            for br in original:
                if _matches_any_drop(br, drops):
                    result.applied_rules.append(f"drop_buildrequires: removed {br}")

        # Add BuildRequires
        if "add_buildrequires" in rules:
            for br in rules["add_buildrequires"]:
                if br not in result.spec.buildrequires:
                    result.spec.buildrequires.append(br)
                    result.applied_rules.append(f"add_buildrequires: added {br}")

        # Collect configure --disable flags
        if "configure_disable" in rules:
            result.configure_disable.extend(rules["configure_disable"])
            result.applied_rules.append(
                f"configure_disable: {rules['configure_disable']}"
            )

        # Collect path rewrites
        if "rewrite_paths" in rules:
            result.path_rewrites.update(rules["rewrite_paths"])
            result.applied_rules.append("rewrite_paths: collected")

        # Collect header overlays
        if "header_overlays" in rules:
            result.header_overlays.extend(rules["header_overlays"])
            result.applied_rules.append(f"header_overlays: {rules['header_overlays']}")

        # Configure flags add/remove
        if "configure_flags" in rules:
            cfg = rules["configure_flags"]
            if "add" in cfg:
                result.configure_flags_add.extend(cfg["add"])
                result.applied_rules.append(f"configure_flags_add: {cfg['add']}")
            if "remove" in cfg:
                result.configure_flags_remove.extend(cfg["remove"])
                result.applied_rules.append(f"configure_flags_remove: {cfg['remove']}")

        # Conditional handling
        if "comment_conditionals" in rules:
            result.comment_conditionals.extend(rules["comment_conditionals"])
            result.applied_rules.append(
                f"comment_conditionals: {rules['comment_conditionals']}"
            )

        if "remove_conditionals" in rules:
            result.remove_conditionals.extend(rules["remove_conditionals"])
            result.applied_rules.append(
                f"remove_conditionals: {rules['remove_conditionals']}"
            )

        if "force_conditionals" in rules:
            result.force_conditionals.update(rules["force_conditionals"])
            result.applied_rules.append(
                f"force_conditionals: {list(rules['force_conditionals'].keys())}"
            )

        # Drop subpackages
        if "drop_subpackages" in rules:
            result.drop_subpackages.extend(rules["drop_subpackages"])
            result.applied_rules.append(
                f"drop_subpackages: {rules['drop_subpackages']}"
            )

        # RPM macros (replaces sgug-rpm-config)
        if "rpm_macros" in rules:
            result.rpm_macros.update(rules["rpm_macros"])
            result.applied_rules.append(
                f"rpm_macros: {list(rules['rpm_macros'].keys())}"
            )

        # Lines to remove globally (e.g., gpgverify)
        if "remove_lines" in rules:
            result.remove_lines.extend(rules["remove_lines"])
            result.applied_rules.append(
                f"remove_lines: {len(rules['remove_lines'])} patterns"
            )

        # Skip check section (for cross-compilation where tests can't run)
        if rules.get("skip_check"):
            result.skip_check = True
            result.applied_rules.append("skip_check: true")

        # Skip find_lang (locale files removed by install_cleanup)
        if rules.get("skip_find_lang"):
            result.skip_find_lang = True
            result.applied_rules.append("skip_find_lang: true")

        # AC_CV overrides for autoconf (generic)
        if "ac_cv_overrides" in rules:
            result.ac_cv_overrides.update(rules["ac_cv_overrides"])
            result.applied_rules.append(
                f"ac_cv_overrides: {list(rules['ac_cv_overrides'].keys())}"
            )

        # Install cleanup commands (generic)
        if "install_cleanup" in rules:
            result.install_cleanup.extend(rules["install_cleanup"])
            result.applied_rules.append(
                f"install_cleanup: {len(rules['install_cleanup'])} commands"
            )

        # Inject compat functions (generic)
        if "inject_compat_functions" in rules:
            funcs = rules["inject_compat_functions"]
            result.compat_functions.extend(funcs)
            result.applied_rules.append(f"inject_compat_functions: {funcs}")

    def _apply_class_rules(
        self, result: TransformResult, rules: dict, class_name: str
    ) -> None:
        """Apply class rules to the result.

        Class rules use the same keys as package rules. Wraps the rules
        dict and delegates to _apply_package_rules for consistent behavior.
        """
        result.applied_rules.append(f"class: {class_name}")
        # Wrap in the structure _apply_package_rules expects
        self._apply_package_rules(result, {"rules": rules})

    def _apply_package_rules(self, result: TransformResult, pkg_rules: dict) -> None:
        """Apply package-specific rules to the result."""
        rules = pkg_rules.get("rules", pkg_rules)

        # Handle add_patch at top level (outside rules section)
        if "add_patch" in pkg_rules:
            result.add_patches.extend(pkg_rules["add_patch"])
            result.applied_rules.append(
                f"add_patch: {len(pkg_rules['add_patch'])} patches"
            )

        # Handle add_source at top level (outside rules section)
        if "add_source" in pkg_rules:
            result.add_sources.extend(pkg_rules["add_source"])
            result.applied_rules.append(
                f"add_source: {len(pkg_rules['add_source'])} sources"
            )

        # Inject compat functions
        if "inject_compat_functions" in rules:
            funcs = rules["inject_compat_functions"]
            result.compat_functions.extend(funcs)
            result.applied_rules.append(f"inject_compat_functions: {funcs}")

        # dlmalloc is NO LONGER auto-injected into compat archives.
        # It's linked by irix-ld for executables only (dlmalloc.o in staging).
        # Shared libraries leave malloc/free undefined so rld resolves them
        # to the executable's single dlmalloc, preventing cross-heap corruption.
        # See rules/INDEX.md "dlmalloc shared library crash".
        if "dlmalloc" in result.compat_functions:
            result.compat_functions.remove("dlmalloc")
            result.applied_rules.append(
                "removed dlmalloc from compat (linked by irix-ld for exe only)"
            )

        # Add additional BuildRequires
        if "add_buildrequires" in rules:
            for br in rules["add_buildrequires"]:
                if br not in result.spec.buildrequires:
                    result.spec.buildrequires.append(br)
                    result.applied_rules.append(
                        f"package add_buildrequires: added {br}"
                    )

        # Drop BuildRequires
        if "drop_buildrequires" in rules:
            drops = set(rules["drop_buildrequires"])
            original = result.spec.buildrequires[:]
            result.spec.buildrequires = [
                br for br in result.spec.buildrequires if not _matches_any_drop(br, drops)
            ]
            for br in original:
                if _matches_any_drop(br, drops):
                    result.applied_rules.append(
                        f"package drop_buildrequires: removed {br}"
                    )

        # Configure disable flags
        if "configure_disable" in rules:
            result.configure_disable.extend(rules["configure_disable"])
            result.applied_rules.append(
                f"package configure_disable: {rules['configure_disable']}"
            )

        # AC_CV overrides for autoconf
        if "ac_cv_overrides" in rules:
            result.ac_cv_overrides.update(rules["ac_cv_overrides"])
            result.applied_rules.append(
                f"ac_cv_overrides: {list(rules['ac_cv_overrides'].keys())}"
            )

        # Additional header overlays
        if "header_overlays" in rules:
            result.header_overlays.extend(rules["header_overlays"])
            result.applied_rules.append(
                f"package header_overlays: {rules['header_overlays']}"
            )

        # Drop runtime Requires
        if "drop_requires" in rules:
            result.drop_requires.extend(rules["drop_requires"])
            result.applied_rules.append(f"drop_requires: {rules['drop_requires']}")

        # Add runtime Requires (for cross-compiled packages with AutoReq disabled)
        if "add_requires" in rules:
            result.add_requires.extend(rules["add_requires"])
            result.applied_rules.append(f"add_requires: {rules['add_requires']}")

        # Lines to remove
        if "remove_lines" in rules:
            result.remove_lines.extend(rules["remove_lines"])
            result.applied_rules.append(
                f"remove_lines: {len(rules['remove_lines'])} patterns"
            )

        # Package-specific configure flags
        if "configure_flags" in rules:
            cfg = rules["configure_flags"]
            if "add" in cfg:
                result.configure_flags_add.extend(cfg["add"])
                result.applied_rules.append(
                    f"package configure_flags_add: {cfg['add']}"
                )
            if "remove" in cfg:
                result.configure_flags_remove.extend(cfg["remove"])
                result.applied_rules.append(
                    f"package configure_flags_remove: {cfg['remove']}"
                )

        # Package-specific conditional handling
        if "comment_conditionals" in rules:
            result.comment_conditionals.extend(rules["comment_conditionals"])
            result.applied_rules.append(
                f"package comment_conditionals: {rules['comment_conditionals']}"
            )

        if "remove_conditionals" in rules:
            result.remove_conditionals.extend(rules["remove_conditionals"])
            result.applied_rules.append(
                f"package remove_conditionals: {rules['remove_conditionals']}"
            )

        if "force_conditionals" in rules:
            result.force_conditionals.update(rules["force_conditionals"])
            result.applied_rules.append(
                f"package force_conditionals: {list(rules['force_conditionals'].keys())}"
            )

        # Package-specific drop subpackages
        if "drop_subpackages" in rules:
            result.drop_subpackages.extend(rules["drop_subpackages"])
            result.applied_rules.append(
                f"package drop_subpackages: {rules['drop_subpackages']}"
            )

        # Commands to inject into %prep section
        if "prep_commands" in rules:
            result.prep_commands.extend(rules["prep_commands"])
            result.applied_rules.append(
                f"prep_commands: {len(rules['prep_commands'])} commands"
            )

        # Environment variables to export in %build
        if "export_vars" in rules:
            result.export_vars.update(rules["export_vars"])
            result.applied_rules.append(
                f"export_vars: {list(rules['export_vars'].keys())}"
            )

        # Path rewrites (override generic rewrite_paths for hand-written specs)
        if "rewrite_paths" in rules:
            result.path_rewrites.update(rules["rewrite_paths"])
            result.applied_rules.append("package rewrite_paths: override")

        # Skip find_lang (for packages with NLS disabled)
        if rules.get("skip_find_lang"):
            result.skip_find_lang = True
            result.applied_rules.append("skip_find_lang: true")

        # Skip check section (for cross-compilation where tests can't run)
        if rules.get("skip_check"):
            result.skip_check = True
            result.applied_rules.append("skip_check: true")

        # Install cleanup commands (run at end of %install)
        if "install_cleanup" in rules:
            result.install_cleanup.extend(rules["install_cleanup"])
            result.applied_rules.append(
                f"install_cleanup: {len(rules['install_cleanup'])} commands"
            )

        # Spec file text replacements (for fixing macros, etc.)
        if "spec_replacements" in rules:
            result.spec_replacements.extend(rules["spec_replacements"])
            result.applied_rules.append(
                f"spec_replacements: {len(rules['spec_replacements'])} patterns"
            )

        # Patches to add from mogrix patches directory
        # Only process here if rules is a separate subsection (not the same as pkg_rules,
        # which is already handled at the top level in _apply_package_rules)
        if "add_patch" in rules and rules is not pkg_rules:
            result.add_patches.extend(rules["add_patch"])
            result.applied_rules.append(
                f"add_patch: {len(rules['add_patch'])} patches"
            )

        # Extra source files to add
        if "add_source" in rules and rules is not pkg_rules:
            result.add_sources.extend(rules["add_source"])
            result.applied_rules.append(
                f"add_source: {len(rules['add_source'])} sources"
            )
