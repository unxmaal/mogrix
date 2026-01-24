"""YAML rule loader for mogrix."""

from pathlib import Path
from typing import Any

import yaml


class RuleLoader:
    """Loads transformation rules from YAML files."""

    def __init__(self, rules_dir: Path):
        """Initialize with path to rules directory."""
        self.rules_dir = Path(rules_dir)

    def load_generic(self) -> dict[str, Any]:
        """Load generic.yaml rules."""
        path = self.rules_dir / "generic.yaml"
        return self._load_yaml(path)

    def load_class(self, class_name: str) -> dict[str, Any] | None:
        """Load a class rule file."""
        path = self.rules_dir / "classes" / f"{class_name}.yaml"
        if path.exists():
            return self._load_yaml(path)
        return None

    def load_package(self, package_name: str) -> dict[str, Any] | None:
        """Load a package-specific rule file.

        Also handles common macro patterns like lib%{libname} -> libFOO
        by checking all package yaml files for matching 'package:' fields.
        """
        # Try direct match first
        path = self.rules_dir / "packages" / f"{package_name}.yaml"
        if path.exists():
            return self._load_yaml(path)

        # If name contains unexpanded macros, search all package files
        if "%{" in package_name:
            packages_dir = self.rules_dir / "packages"
            if packages_dir.exists():
                for yaml_file in packages_dir.glob("*.yaml"):
                    try:
                        rules = self._load_yaml(yaml_file)
                        if rules and rules.get("package") == yaml_file.stem:
                            # Check if this could be a match by pattern
                            # e.g., lib%{libname} might match libsolv
                            pass
                        # Also check 'aliases' field
                        if rules and package_name in rules.get("aliases", []):
                            return rules
                    except Exception:
                        pass

        return None

    def _load_yaml(self, path: Path) -> dict[str, Any]:
        """Load and parse a YAML file."""
        with open(path) as f:
            return yaml.safe_load(f)
