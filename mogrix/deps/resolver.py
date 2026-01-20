"""Dependency resolver for mogrix."""

import re
from dataclasses import dataclass, field
from pathlib import Path


@dataclass
class MissingDep:
    """A missing build dependency."""
    name: str
    version: str | None = None
    has_rule: bool = False
    srpm_url: str | None = None


@dataclass
class BuildResult:
    """Result of a build attempt."""
    success: bool
    missing_deps: list[MissingDep] = field(default_factory=list)
    output: str = ""
    return_code: int = 0


class DependencyResolver:
    """Resolves and tracks build dependencies."""

    def __init__(self, rules_dir: Path):
        """Initialize with path to rules directory."""
        self.rules_dir = rules_dir
        self._available_rules: set[str] | None = None

    @property
    def available_rules(self) -> set[str]:
        """Get set of package names that have rules."""
        if self._available_rules is None:
            self._available_rules = set()
            packages_dir = self.rules_dir / "packages"
            if packages_dir.exists():
                for rule_file in packages_dir.glob("*.yaml"):
                    self._available_rules.add(rule_file.stem)
        return self._available_rules

    def parse_rpmbuild_errors(self, output: str) -> list[MissingDep]:
        """Parse rpmbuild output to extract missing dependencies.

        Args:
            output: stderr/stdout from rpmbuild

        Returns:
            List of missing dependencies
        """
        missing = []

        # Pattern: "        package-name is needed by ..."
        # Or: "        package-name >= 1.0 is needed by ..."
        pattern = r"^\s+(\S+?)(?:\s*([<>=]+)\s*(\S+))?\s+is needed by"

        for line in output.splitlines():
            match = re.match(pattern, line)
            if match:
                name = match.group(1)
                version = match.group(3) if match.group(2) else None

                # Check if we have a rule for this package
                has_rule = self._check_has_rule(name)

                missing.append(MissingDep(
                    name=name,
                    version=version,
                    has_rule=has_rule,
                ))

        return missing

    def _check_has_rule(self, dep_name: str) -> bool:
        """Check if we have a rule for a dependency.

        Handles mapping from dep name to package name, e.g.:
        - "zlib-devel" -> "zlib"
        - "python3-devel" -> "python3"
        - "gcc" -> "gcc"
        """
        # Direct match
        if dep_name in self.available_rules:
            return True

        # Try stripping common suffixes
        for suffix in ["-devel", "-libs", "-static", "-doc"]:
            if dep_name.endswith(suffix):
                base = dep_name[:-len(suffix)]
                if base in self.available_rules:
                    return True

        return False

    def get_package_for_dep(self, dep_name: str) -> str | None:
        """Get the package name that provides a dependency.

        Args:
            dep_name: Dependency name (e.g., "zlib-devel")

        Returns:
            Package name (e.g., "zlib") or None if unknown
        """
        # Direct match
        if dep_name in self.available_rules:
            return dep_name

        # Try stripping common suffixes
        for suffix in ["-devel", "-libs", "-static", "-doc"]:
            if dep_name.endswith(suffix):
                base = dep_name[:-len(suffix)]
                if base in self.available_rules:
                    return base

        return None

    def categorize_deps(self, deps: list[MissingDep]) -> dict[str, list[MissingDep]]:
        """Categorize dependencies by whether we can handle them.

        Returns:
            Dict with keys:
            - "have_rules": Deps we have conversion rules for
            - "need_rules": Deps we don't have rules for
            - "system": System deps that should already exist (gcc, make, etc.)
        """
        system_packages = {
            "gcc", "gcc-c++", "make", "autoconf", "automake", "libtool",
            "pkgconfig", "pkg-config", "cmake", "ninja-build", "meson",
            "bison", "flex", "gettext", "perl", "python3",
        }

        result = {
            "have_rules": [],
            "need_rules": [],
            "system": [],
        }

        for dep in deps:
            if dep.name in system_packages:
                result["system"].append(dep)
            elif dep.has_rule:
                result["have_rules"].append(dep)
            else:
                result["need_rules"].append(dep)

        return result
