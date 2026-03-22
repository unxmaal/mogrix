"""IRIX platform model for mogrix.

Loads rules/platform.yaml and provides a query API for platform capabilities.
Used by the rule engine to derive transforms from platform knowledge instead
of hardcoding lists.
"""

import logging
from functools import lru_cache
from pathlib import Path
from typing import Any

import yaml

logger = logging.getLogger(__name__)


class PlatformModel:
    """Query interface for IRIX platform capabilities."""

    def __init__(self, data: dict[str, Any]):
        self._data = data.get("platform", data)

        # Build lookup sets for O(1) queries
        provides = self._data.get("provides", {})
        self._shimmed_functions: set[str] = set(
            provides.get("shimmed_functions", [])
        )

        lacks = self._data.get("lacks", {})
        self._lacking_subsystems: set[str] = set(
            lacks.get("subsystems", [])
        )
        self._lacking_buildrequires: set[str] = set(
            lacks.get("buildrequires", [])
        )
        self._lacking_functions: set[str] = set(
            lacks.get("functions", [])
        )

        self._quirks: dict[str, dict[str, str]] = self._data.get("quirks", {})
        self._defines: set[str] = set(self._data.get("defines", []))

    @property
    def name(self) -> str:
        return self._data.get("name", "IRIX 6.5")

    @property
    def arch(self) -> str:
        return self._data.get("arch", "mips")

    @property
    def abi(self) -> str:
        return self._data.get("abi", "n32")

    @property
    def prefix(self) -> str:
        return self._data.get("prefix", "/usr/sgug")

    def has_function(self, name: str) -> bool:
        """Check if a function is available (either native or shimmed).

        Returns True if the function is provided by the compat layer.
        Returns False if the function is in the 'lacks' list or unknown.
        """
        if name in self._shimmed_functions:
            return True
        if name in self._lacking_functions:
            return False
        # Unknown function — not in either list. Conservative: assume unavailable.
        return False

    def is_shimmed(self, name: str) -> bool:
        """Check if a function is provided by the mogrix compat layer."""
        return name in self._shimmed_functions

    def has_subsystem(self, name: str) -> bool:
        """Check if IRIX has a given subsystem (selinux, systemd, etc.)."""
        return name not in self._lacking_subsystems

    def has_library(self, name: str) -> bool:
        """Check if a library is available on IRIX.

        Currently checks against the lacking_buildrequires set.
        """
        return name not in self._lacking_buildrequires

    def get_quirk(self, name: str) -> dict[str, str] | None:
        """Get a platform quirk by name, or None if not found."""
        return self._quirks.get(name)

    def get_all_quirks(self) -> dict[str, dict[str, str]]:
        """Get all platform quirks."""
        return dict(self._quirks)

    def lacking_subsystems(self) -> set[str]:
        """Return set of subsystems IRIX lacks."""
        return set(self._lacking_subsystems)

    def shimmed_functions(self) -> set[str]:
        """Return set of functions provided by compat layer."""
        return set(self._shimmed_functions)

    def has_define(self, name: str) -> bool:
        """Check if a preprocessor macro is defined on IRIX."""
        return name in self._defines


@lru_cache(maxsize=1)
def load_platform(rules_dir: Path | None = None) -> PlatformModel:
    """Load the platform model from rules/platform.yaml.

    Cached — subsequent calls return the same instance.
    """
    if rules_dir is None:
        rules_dir = Path(__file__).parent.parent / "rules"

    platform_path = rules_dir / "platform.yaml"
    if not platform_path.exists():
        logger.warning("platform.yaml not found at %s, using empty model", platform_path)
        return PlatformModel({})

    with open(platform_path) as f:
        data = yaml.safe_load(f) or {}

    return PlatformModel(data)
