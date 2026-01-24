"""Rule application engine for mogrix."""

from dataclasses import dataclass, field

from mogrix.parser.spec import SpecFile
from mogrix.rules.loader import RuleLoader


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

        # Load and apply package-specific rules
        pkg_rules = self.loader.load_package(spec.name)
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
                br for br in result.spec.buildrequires if br not in drops
            ]
            for br in original:
                if br in drops:
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

    def _apply_package_rules(self, result: TransformResult, pkg_rules: dict) -> None:
        """Apply package-specific rules to the result."""
        rules = pkg_rules.get("rules", pkg_rules)

        # Handle add_patch at top level (outside rules section)
        if "add_patch" in pkg_rules:
            result.add_patches.extend(pkg_rules["add_patch"])
            result.applied_rules.append(
                f"add_patch: {len(pkg_rules['add_patch'])} patches"
            )

        # Inject compat functions
        if "inject_compat_functions" in rules:
            funcs = rules["inject_compat_functions"]
            result.compat_functions.extend(funcs)
            result.applied_rules.append(f"inject_compat_functions: {funcs}")

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
                br for br in result.spec.buildrequires if br not in drops
            ]
            for br in original:
                if br in drops:
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
        if "add_patch" in rules:
            result.add_patches.extend(rules["add_patch"])
            result.applied_rules.append(
                f"add_patch: {len(rules['add_patch'])} patches"
            )
