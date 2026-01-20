"""Compat source injector for mogrix."""

from pathlib import Path
from typing import Any

import yaml


class CompatInjector:
    """Manages injection of compat source files into packages."""

    def __init__(self, compat_dir: Path):
        """Initialize with path to compat directory."""
        self.compat_dir = Path(compat_dir)
        self.catalog = self._load_catalog()

    def _load_catalog(self) -> dict[str, Any]:
        """Load the function catalog."""
        catalog_path = self.compat_dir / "catalog.yaml"
        if catalog_path.exists():
            with open(catalog_path) as f:
                return yaml.safe_load(f)
        return {"functions": {}, "sets": {}}

    def get_source_file(self, function_name: str) -> Path | None:
        """Get the source file path for a function."""
        functions = self.catalog.get("functions", {})
        if function_name not in functions:
            return None

        rel_path = functions[function_name].get("file")
        if rel_path:
            full_path = self.compat_dir / rel_path
            if full_path.exists():
                return full_path
        return None

    def resolve_set(self, set_name: str) -> list[Path]:
        """Resolve a function set to list of source files."""
        sets = self.catalog.get("sets", {})
        if set_name not in sets:
            return []

        functions = sets[set_name]
        return self.resolve_functions(functions)

    def resolve_functions(self, function_names: list[str]) -> list[Path]:
        """Resolve function names to unique source files."""
        files = set()
        for func in function_names:
            path = self.get_source_file(func)
            if path:
                files.add(path)
        return sorted(files)

    def get_source_entries(
        self,
        function_names: list[str],
        start_num: int = 100,
    ) -> str:
        """Generate Source entries for spec file.

        Args:
            function_names: List of function names to include
            start_num: Starting Source number (e.g., 100 for Source100:)

        Returns:
            String with Source entries for the spec file
        """
        files = self.resolve_functions(function_names)
        lines = []
        for i, path in enumerate(files):
            source_num = start_num + i
            lines.append(f"Source{source_num}: {path.name}")
        return "\n".join(lines)

    def get_prep_commands(
        self,
        function_names: list[str],
        start_num: int = 100,
    ) -> str:
        """Generate %prep commands to copy compat sources.

        Args:
            function_names: List of function names to include
            start_num: Starting Source number

        Returns:
            String with commands for %prep section
        """
        files = self.resolve_functions(function_names)
        if not files:
            return ""

        lines = ["# Mogrix compat sources", "mkdir -p mogrix-compat"]
        for i, path in enumerate(files):
            source_num = start_num + i
            lines.append(f"cp %{{SOURCE{source_num}}} mogrix-compat/")

        return "\n".join(lines)

    def get_build_commands(self, function_names: list[str]) -> str:
        """Generate %build commands to compile compat sources.

        Returns:
            String with commands for %build section
        """
        files = self.resolve_functions(function_names)
        if not files:
            return ""

        lines = [
            "# Compile mogrix compat sources",
            "for f in mogrix-compat/*.c; do",
            '  %{__cc} %{optflags} -c "$f" -o "${f%.c}.o"',
            "done",
            'export LIBS="mogrix-compat/*.o $LIBS"',
        ]

        return "\n".join(lines)

    def list_available_functions(self) -> list[str]:
        """List all available function names."""
        return sorted(self.catalog.get("functions", {}).keys())

    def list_available_sets(self) -> list[str]:
        """List all available function sets."""
        return sorted(self.catalog.get("sets", {}).keys())
