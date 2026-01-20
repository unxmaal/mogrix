"""Rule validation for mogrix."""

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
    "ac_cv_overrides",
    "header_overlays",
    "remove_lines",
    "comment_conditionals",
    "patches",
    "notes",
}

# Valid top-level keys for package rule files
VALID_PACKAGE_TOP_KEYS = {"package", "rules", "version", "notes"}


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
        self._load_compat_catalog()

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
