"""Rule validation for mogrix."""

import re
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any

import yaml


@dataclass
class ValidationIssue:
    """A validation issue found in a rule file."""

    file: str
    severity: str  # "error", "warning", "info"
    message: str
    line: int | None = None


@dataclass
class ValidationResult:
    """Result of validating rules."""

    issues: list[ValidationIssue] = field(default_factory=list)
    files_checked: int = 0
    packages_checked: int = 0

    @property
    def errors(self) -> list[ValidationIssue]:
        return [i for i in self.issues if i.severity == "error"]

    @property
    def warnings(self) -> list[ValidationIssue]:
        return [i for i in self.issues if i.severity == "warning"]

    @property
    def is_valid(self) -> bool:
        return len(self.errors) == 0


# Valid rule keys for package rules
VALID_PACKAGE_RULE_KEYS = {
    "inject_compat_functions",
    "drop_buildrequires",
    "add_buildrequires",
    "drop_requires",
    "add_requires",
    "configure_disable",
    "configure_enable",
    "configure_flags",
    "configure_opts",
    "ac_cv_overrides",
    "header_overlays",
    "remove_lines",
    "comment_conditionals",
    "remove_conditionals",
    "force_conditionals",
    "drop_subpackages",
    "prep_commands",
    "export_vars",
    "skip_find_lang",
    "skip_check",
    "skip_manpages",
    "install_cleanup",
    "spec_replacements",
    "add_patch",
    "add_source",
    "extra_cflags",
    "make_env",
    "files_no_lang",
    "patches",
    "notes",
}

# Valid top-level keys for package rule files
VALID_PACKAGE_TOP_KEYS = {
    "package",
    "rules",
    "version",
    "notes",
    "add_patch",
    "add_source",
    "aliases",
    "ac_cv_overrides",
    "drop_buildrequires",
    "classes",
    "smoke_test",
    "upstream",
    "bundle_trampoline_exclude",
}

# Valid keys inside the upstream: block
VALID_UPSTREAM_KEYS = {
    "url",
    "version",
    "ref",
    "type",
    "build_system",
    "license",
    "summary",
    "source_dir",
}

REQUIRED_UPSTREAM_KEYS = {"url", "version", "build_system"}

VALID_BUILD_SYSTEMS = {"autoconf", "cmake", "meson", "makefile"}

VALID_UPSTREAM_TYPES = {"git", "tarball"}


class RuleValidator:
    """Validates mogrix rule files."""

    def __init__(self, rules_dir: Path, compat_dir: Path | None = None):
        """Initialize validator.

        Args:
            rules_dir: Path to rules directory
            compat_dir: Path to compat directory (for function validation)
        """
        self.rules_dir = Path(rules_dir)
        self.compat_dir = Path(compat_dir) if compat_dir else None
        self.valid_compat_functions: set[str] = set()
        self._generic_rules: dict = {}
        self._load_compat_catalog()
        self._load_generic_rules()

    def _load_compat_catalog(self) -> None:
        """Load the compat catalog to get valid function names."""
        if self.compat_dir is None:
            # Try default location
            self.compat_dir = self.rules_dir.parent / "compat"

        catalog_path = self.compat_dir / "catalog.yaml"
        if catalog_path.exists():
            with open(catalog_path) as f:
                catalog = yaml.safe_load(f)
                if catalog and "functions" in catalog:
                    self.valid_compat_functions = set(catalog["functions"].keys())

    def _load_generic_rules(self) -> None:
        """Load generic.yaml to detect package rules that duplicate it."""
        generic_path = self.rules_dir / "generic.yaml"
        if generic_path.exists():
            with open(generic_path) as f:
                data = yaml.safe_load(f)
            if data and "generic" in data:
                self._generic_rules = data["generic"]

    def validate_all(self) -> ValidationResult:
        """Validate all rule files.

        Returns:
            ValidationResult with any issues found
        """
        result = ValidationResult()

        # Validate generic.yaml
        generic_path = self.rules_dir / "generic.yaml"
        if generic_path.exists():
            result.files_checked += 1
            self._validate_generic(generic_path, result)
        else:
            result.issues.append(
                ValidationIssue(
                    file="generic.yaml",
                    severity="warning",
                    message="generic.yaml not found",
                )
            )

        # Validate package rules
        packages_dir = self.rules_dir / "packages"
        if packages_dir.exists():
            for rule_file in sorted(packages_dir.glob("*.yaml")):
                result.files_checked += 1
                result.packages_checked += 1
                self._validate_package_rule(rule_file, result)

        # Validate class rules
        classes_dir = self.rules_dir / "classes"
        if classes_dir.exists():
            for rule_file in sorted(classes_dir.glob("*.yaml")):
                result.files_checked += 1
                self._validate_class_rule(rule_file, result)

        # Cross-validation checks
        self._validate_cross_references(result)

        return result

    def _validate_generic(self, path: Path, result: ValidationResult) -> None:
        """Validate generic.yaml structure."""
        try:
            with open(path) as f:
                data = yaml.safe_load(f)

            if not isinstance(data, dict):
                result.issues.append(
                    ValidationIssue(
                        file=str(path.name),
                        severity="error",
                        message="generic.yaml must be a dictionary",
                    )
                )
                return

            if "generic" not in data:
                result.issues.append(
                    ValidationIssue(
                        file=str(path.name),
                        severity="error",
                        message="generic.yaml must have a 'generic' key",
                    )
                )

        except yaml.YAMLError as e:
            result.issues.append(
                ValidationIssue(
                    file=str(path.name),
                    severity="error",
                    message=f"YAML parse error: {e}",
                )
            )

    def _validate_package_rule(self, path: Path, result: ValidationResult) -> None:
        """Validate a package rule file."""
        try:
            with open(path) as f:
                data = yaml.safe_load(f)

            if not isinstance(data, dict):
                result.issues.append(
                    ValidationIssue(
                        file=str(path.name),
                        severity="error",
                        message="Package rule must be a dictionary",
                    )
                )
                return

            # Check required 'package' key
            if "package" not in data:
                result.issues.append(
                    ValidationIssue(
                        file=str(path.name),
                        severity="error",
                        message="Missing required 'package' key",
                    )
                )
            else:
                # Check package name matches filename
                expected_name = path.stem
                if data["package"] != expected_name:
                    result.issues.append(
                        ValidationIssue(
                            file=str(path.name),
                            severity="warning",
                            message=f"Package name '{data['package']}' doesn't match filename '{expected_name}'",
                        )
                    )

            # Check for unknown top-level keys
            for key in data.keys():
                if key not in VALID_PACKAGE_TOP_KEYS:
                    result.issues.append(
                        ValidationIssue(
                            file=str(path.name),
                            severity="warning",
                            message=f"Unknown top-level key: '{key}'",
                        )
                    )

            # Validate classes references
            if "classes" in data:
                classes = data["classes"]
                if not isinstance(classes, list):
                    result.issues.append(
                        ValidationIssue(
                            file=str(path.name),
                            severity="error",
                            message="'classes' must be a list",
                        )
                    )
                else:
                    classes_dir = self.rules_dir / "classes"
                    for cls in classes:
                        cls_path = classes_dir / f"{cls}.yaml"
                        if not cls_path.exists():
                            result.issues.append(
                                ValidationIssue(
                                    file=str(path.name),
                                    severity="error",
                                    message=f"Referenced class not found: '{cls}' (expected {cls_path})",
                                )
                            )

            # Validate upstream block
            if "upstream" in data:
                self._validate_upstream(path, data["upstream"], result)

            # Validate rules section
            if "rules" in data:
                self._validate_rules_section(path, data["rules"], result)

        except yaml.YAMLError as e:
            result.issues.append(
                ValidationIssue(
                    file=str(path.name),
                    severity="error",
                    message=f"YAML parse error: {e}",
                )
            )

    def _validate_upstream(
        self, path: Path, upstream: Any, result: ValidationResult
    ) -> None:
        """Validate the upstream: block of a package rule."""
        if not isinstance(upstream, dict):
            result.issues.append(
                ValidationIssue(
                    file=str(path.name),
                    severity="error",
                    message="'upstream' must be a dictionary",
                )
            )
            return

        # Check required keys
        for key in REQUIRED_UPSTREAM_KEYS:
            if key not in upstream:
                result.issues.append(
                    ValidationIssue(
                        file=str(path.name),
                        severity="error",
                        message=f"upstream: missing required key '{key}'",
                    )
                )

        # Check for unknown keys
        for key in upstream:
            if key not in VALID_UPSTREAM_KEYS:
                result.issues.append(
                    ValidationIssue(
                        file=str(path.name),
                        severity="warning",
                        message=f"upstream: unknown key '{key}'",
                    )
                )

        # Validate build_system value
        bs = upstream.get("build_system")
        if bs and bs not in VALID_BUILD_SYSTEMS:
            result.issues.append(
                ValidationIssue(
                    file=str(path.name),
                    severity="error",
                    message=f"upstream: build_system '{bs}' not in {VALID_BUILD_SYSTEMS}",
                )
            )

        # Validate type value
        utype = upstream.get("type")
        if utype and utype not in VALID_UPSTREAM_TYPES:
            result.issues.append(
                ValidationIssue(
                    file=str(path.name),
                    severity="error",
                    message=f"upstream: type '{utype}' not in {VALID_UPSTREAM_TYPES}",
                )
            )

    def _validate_rules_section(
        self, path: Path, rules: Any, result: ValidationResult
    ) -> None:
        """Validate the rules section of a package rule."""
        if not isinstance(rules, dict):
            result.issues.append(
                ValidationIssue(
                    file=str(path.name),
                    severity="error",
                    message="'rules' must be a dictionary",
                )
            )
            return

        # Check for unknown rule keys
        for key in rules.keys():
            if key not in VALID_PACKAGE_RULE_KEYS:
                result.issues.append(
                    ValidationIssue(
                        file=str(path.name),
                        severity="warning",
                        message=f"Unknown rule key: '{key}'",
                    )
                )

        # Validate inject_compat_functions
        if "inject_compat_functions" in rules:
            funcs = rules["inject_compat_functions"]
            if not isinstance(funcs, list):
                result.issues.append(
                    ValidationIssue(
                        file=str(path.name),
                        severity="error",
                        message="inject_compat_functions must be a list",
                    )
                )
            elif self.valid_compat_functions:
                for func in funcs:
                    if func not in self.valid_compat_functions:
                        result.issues.append(
                            ValidationIssue(
                                file=str(path.name),
                                severity="error",
                                message=f"Unknown compat function: '{func}' (valid: {sorted(self.valid_compat_functions)})",
                            )
                        )

        # Validate list fields
        list_fields = [
            "drop_buildrequires",
            "add_buildrequires",
            "drop_requires",
            "add_requires",
            "configure_disable",
            "configure_enable",
            "header_overlays",
            "remove_lines",
            "comment_conditionals",
            "prep_commands",
            "patches",
        ]
        for field_name in list_fields:
            if field_name in rules and not isinstance(rules[field_name], list):
                result.issues.append(
                    ValidationIssue(
                        file=str(path.name),
                        severity="error",
                        message=f"'{field_name}' must be a list",
                    )
                )

        # Validate dict fields
        dict_fields = ["ac_cv_overrides", "configure_flags"]
        for field_name in dict_fields:
            if field_name in rules and not isinstance(rules[field_name], dict):
                result.issues.append(
                    ValidationIssue(
                        file=str(path.name),
                        severity="error",
                        message=f"'{field_name}' must be a dictionary",
                    )
                )

        # Check for conflicts within the rule
        self._check_rule_conflicts(path, rules, result)

        # Check for rules that duplicate generic.yaml
        self._check_generic_duplicates(path, rules, result)

        # Check for inline C code in prep_commands
        self._check_inline_c_code(path, rules, result)

    def _check_rule_conflicts(
        self, path: Path, rules: dict, result: ValidationResult
    ) -> None:
        """Check for conflicting rules within a single rule file."""
        # Check drop_buildrequires vs add_buildrequires
        drop_br = set(rules.get("drop_buildrequires", []) or [])
        add_br = set(rules.get("add_buildrequires", []) or [])
        conflicts = drop_br & add_br
        if conflicts:
            result.issues.append(
                ValidationIssue(
                    file=str(path.name),
                    severity="warning",
                    message=f"Conflicting BuildRequires (both drop and add): {sorted(conflicts)}",
                )
            )

        # Check drop_requires vs add_requires
        drop_req = set(rules.get("drop_requires", []) or [])
        add_req = set(rules.get("add_requires", []) or [])
        conflicts = drop_req & add_req
        if conflicts:
            result.issues.append(
                ValidationIssue(
                    file=str(path.name),
                    severity="warning",
                    message=f"Conflicting Requires (both drop and add): {sorted(conflicts)}",
                )
            )

        # Check configure_disable vs configure_enable
        disable = set(rules.get("configure_disable", []) or [])
        enable = set(rules.get("configure_enable", []) or [])
        conflicts = disable & enable
        if conflicts:
            result.issues.append(
                ValidationIssue(
                    file=str(path.name),
                    severity="warning",
                    message=f"Conflicting configure flags (both disable and enable): {sorted(conflicts)}",
                )
            )

        # Check configure_flags add vs remove
        cfg_flags = rules.get("configure_flags", {})
        if isinstance(cfg_flags, dict):
            add_flags = set(cfg_flags.get("add", []) or [])
            remove_flags = set(cfg_flags.get("remove", []) or [])
            conflicts = add_flags & remove_flags
            if conflicts:
                result.issues.append(
                    ValidationIssue(
                        file=str(path.name),
                        severity="warning",
                        message=f"Conflicting configure_flags (both add and remove): {sorted(conflicts)}",
                    )
                )

    def _check_generic_duplicates(
        self, path: Path, rules: dict, result: ValidationResult
    ) -> None:
        """Check for rules that duplicate what generic.yaml already provides."""
        if not self._generic_rules:
            return

        g = self._generic_rules

        # Check boolean flags
        if rules.get("skip_check") and g.get("skip_check"):
            result.issues.append(
                ValidationIssue(
                    file=str(path.name),
                    severity="warning",
                    message="skip_check: true duplicates generic.yaml (remove it)",
                )
            )
        if rules.get("skip_find_lang") and g.get("skip_find_lang"):
            result.issues.append(
                ValidationIssue(
                    file=str(path.name),
                    severity="warning",
                    message="skip_find_lang: true duplicates generic.yaml (remove it)",
                )
            )

        # Check list-type rules for individual value overlap
        generic_sets = {
            "drop_buildrequires": set(g.get("drop_buildrequires", [])),
            "drop_requires": set(g.get("drop_requires", [])),
            "configure_disable": set(g.get("configure_disable", [])),
            "inject_compat_functions": set(g.get("inject_compat_functions", [])),
            "remove_lines": set(g.get("remove_lines", [])),
        }

        for key, generic_vals in generic_sets.items():
            pkg_vals = rules.get(key, [])
            if not isinstance(pkg_vals, list):
                continue
            dupes = [v for v in pkg_vals if v in generic_vals]
            if dupes:
                result.issues.append(
                    ValidationIssue(
                        file=str(path.name),
                        severity="warning",
                        message=(
                            f"{key}: {dupes} already in generic.yaml (remove from package)"
                        ),
                    )
                )

        # Check ac_cv_overrides for duplicate key=value pairs
        generic_ac = g.get("ac_cv_overrides", {})
        pkg_ac = rules.get("ac_cv_overrides", {})
        if isinstance(pkg_ac, dict) and generic_ac:
            dupes = [
                f"{k}={v}"
                for k, v in pkg_ac.items()
                if generic_ac.get(k) == v
            ]
            if dupes:
                result.issues.append(
                    ValidationIssue(
                        file=str(path.name),
                        severity="warning",
                        message=(
                            f"ac_cv_overrides: {dupes} already in generic.yaml (remove from package)"
                        ),
                    )
                )

    # Patterns that indicate inline C code in prep_commands.
    # These should be extracted to add_source files instead.
    _INLINE_C_PATTERNS = [
        # Heredoc writing a .c or .h file
        re.compile(r"cat\s+>\s*\S+\.(c|h)\s*<<"),
        # printf chain writing a .c or .h file
        re.compile(r"printf\s.*>\s*\S+\.(c|h)\s*$"),
    ]
    # Threshold: prep_commands with this many lines AND C-like content
    _INLINE_C_LINE_THRESHOLD = 10
    _INLINE_C_CONTENT_PATTERNS = [
        re.compile(r"#include\s+[<\"]"),
        re.compile(r"\b(int|void|char|pid_t|size_t|ssize_t)\s+\w+\s*\("),
        re.compile(r"\breturn\s+\(?\s*-?\d"),
    ]

    def _check_inline_c_code(
        self, path: Path, rules: dict, result: ValidationResult
    ) -> None:
        """Check for inline C code in prep_commands.

        C source code should live in patches/packages/<name>/ as standalone
        files referenced via add_source, not inline in YAML. Inline C is
        fragile (YAML escaping), hard to review, and untestable.
        """
        prep = rules.get("prep_commands", [])
        if not isinstance(prep, list):
            return

        for i, cmd in enumerate(prep):
            if not isinstance(cmd, str):
                continue

            # Check for direct file-generation patterns
            for pattern in self._INLINE_C_PATTERNS:
                if pattern.search(cmd):
                    result.issues.append(
                        ValidationIssue(
                            file=str(path.name),
                            severity="warning",
                            message=(
                                f"prep_commands[{i}]: inline C code generation detected. "
                                "Extract to patches/packages/<name>/ and use add_source instead."
                            ),
                        )
                    )
                    break

            # Check for long commands with C-like content
            lines = cmd.strip().splitlines()
            if len(lines) >= self._INLINE_C_LINE_THRESHOLD:
                c_indicators = sum(
                    1
                    for p in self._INLINE_C_CONTENT_PATTERNS
                    if p.search(cmd)
                )
                if c_indicators >= 2:
                    result.issues.append(
                        ValidationIssue(
                            file=str(path.name),
                            severity="warning",
                            message=(
                                f"prep_commands[{i}]: {len(lines)}-line command "
                                "contains C code patterns. Consider extracting to "
                                "a file in patches/packages/<name>/ via add_source."
                            ),
                        )
                    )

    def _validate_class_rule(self, path: Path, result: ValidationResult) -> None:
        """Validate a class rule file."""
        try:
            with open(path) as f:
                data = yaml.safe_load(f)

            if not isinstance(data, dict):
                result.issues.append(
                    ValidationIssue(
                        file=str(path.name),
                        severity="error",
                        message="Class rule must be a dictionary",
                    )
                )
                return

            if "class" not in data:
                result.issues.append(
                    ValidationIssue(
                        file=str(path.name),
                        severity="error",
                        message="Missing required 'class' key",
                    )
                )

        except yaml.YAMLError as e:
            result.issues.append(
                ValidationIssue(
                    file=str(path.name),
                    severity="error",
                    message=f"YAML parse error: {e}",
                )
            )

    def _validate_cross_references(self, result: ValidationResult) -> None:
        """Check for cross-reference issues between rules."""
        # Could check for:
        # - Dependencies between packages that need rules
        # - Conflicting drop/add for same package
        # - Header overlay references that don't exist
        pass

    def validate_file(self, path: Path) -> ValidationResult:
        """Validate a single rule file.

        Args:
            path: Path to the rule file

        Returns:
            ValidationResult for the single file
        """
        result = ValidationResult()
        result.files_checked = 1

        if path.name == "generic.yaml":
            self._validate_generic(path, result)
        elif path.parent.name == "packages":
            result.packages_checked = 1
            self._validate_package_rule(path, result)
        elif path.parent.name == "classes":
            self._validate_class_rule(path, result)
        else:
            result.issues.append(
                ValidationIssue(
                    file=str(path.name),
                    severity="warning",
                    message="Unknown rule file location",
                )
            )

        return result
