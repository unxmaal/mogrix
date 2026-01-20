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
        """Load a package-specific rule file."""
        path = self.rules_dir / "packages" / f"{package_name}.yaml"
        if path.exists():
            return self._load_yaml(path)
        return None

    def _load_yaml(self, path: Path) -> dict[str, Any]:
        """Load and parse a YAML file."""
        with open(path) as f:
            return yaml.safe_load(f)
