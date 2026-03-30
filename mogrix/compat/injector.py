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
        """Resolve function names to unique source files.

        When a file declares 'provides' in the catalog, any functions it
        provides are considered already supplied.  If a standalone file
        would only duplicate a symbol already provided by another included
        file, the standalone file is skipped.  This prevents duplicate
        symbol errors (e.g. openat-compat.c provides stpcpy, so the
        standalone runtime/stpcpy.c is not needed when openat is also
        requested).
        """
        functions = self.catalog.get("functions", {})

        # First pass: collect all files and what each provides
        file_for_func: dict[str, Path] = {}
        for func in function_names:
            path = self.get_source_file(func)
            if path:
                file_for_func[func] = path

        # Build set of symbols already provided by multi-symbol files
        provided_symbols: set[str] = set()
        for func in function_names:
            entry = functions.get(func, {})
            provides = entry.get("provides", [])
            if provides:
                provided_symbols.update(provides)

        # Second pass: include files, but skip standalone files for
        # symbols that are already provided by a multi-symbol file
        files = set()
        for func in function_names:
            path = file_for_func.get(func)
            if not path:
                continue
            # If this function is provided by another (multi-symbol) file,
            # check if that other file is also being included
            entry = functions.get(func, {})
            if not entry.get("provides"):
                # This is a standalone file — check if symbol is already
                # provided by a multi-symbol file that's in our set
                if func in provided_symbols:
                    # Find the providing file and check it's included
                    for other_func in function_names:
                        other_entry = functions.get(other_func, {})
                        other_provides = other_entry.get("provides", [])
                        if func in other_provides and other_func in file_for_func:
                            # The multi-symbol file is included — skip standalone
                            break
                    else:
                        files.add(path)
                    continue
            files.add(path)
        return sorted(files)

    def get_extra_files(self, function_names: list[str]) -> list[Path]:
        """Get extra files needed by compat functions (e.g., .inc files).

        Some compat sources need companion files (like dlmalloc.c needing
        dlmalloc-src.inc). These are listed in the catalog's extra_files field.
        They get copied alongside the .c files but are not compiled separately.
        """
        functions = self.catalog.get("functions", {})
        extras = []
        seen = set()
        for func in function_names:
            if func not in functions:
                continue
            for rel_path in functions[func].get("extra_files", []):
                if rel_path not in seen:
                    full_path = self.compat_dir / rel_path
                    if full_path.exists():
                        extras.append(full_path)
                        seen.add(rel_path)
        return extras

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
        extras = self.get_extra_files(function_names)
        all_files = list(files) + extras
        lines = []
        for i, path in enumerate(all_files):
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

        extras = self.get_extra_files(function_names)
        all_files = list(files) + extras

        lines = ["# Mogrix compat sources", "mkdir -p mogrix-compat"]
        for i, path in enumerate(all_files):
            source_num = start_num + i
            lines.append(f"cp %{{SOURCE{source_num}}} mogrix-compat/")

        return "\n".join(lines)

    def get_build_commands(self, function_names: list[str]) -> str:
        """Generate %build commands to compile compat sources.

        Creates a static archive to avoid glob expansion issues with libtool.
        The archive is linked via -L and -l flags which work reliably.

        Files with force_link: true in the catalog are compiled but NOT put
        in the archive. Instead they're linked directly via LDFLAGS as .o
        files, ensuring constructors and other unreferenced symbols are
        always included.

        Returns:
            String with commands for %build section
        """
        files = self.resolve_functions(function_names)
        if not files:
            return ""

        # Identify force-link files from catalog
        functions = self.catalog.get("functions", {})
        force_link_files: set[str] = set()
        for func in function_names:
            entry = functions.get(func, {})
            if entry.get("force_link"):
                source = entry.get("file", "")
                if source:
                    # Just the basename for matching in mogrix-compat/
                    force_link_files.add(source.rsplit("/", 1)[-1])

        lines = [
            "# Compile mogrix compat sources into static archive",
            "export MOGRIX_STAGING=\"%{_staging}\"",
            "export MOGRIX_ROOT=\"%{_mogrix_root}\"",
            "COMPAT_DIR=$(pwd)/mogrix-compat",
            "for f in mogrix-compat/*.c; do",
            '  %{__cc} %{optflags} -c "$f" -o "${f%.c}.o"',
            "done",
        ]

        if force_link_files:
            # Move force-link .o files out before archiving
            force_basenames = " ".join(
                f.replace(".c", ".o") for f in sorted(force_link_files)
            )
            lines.extend([
                f"# Force-link objects (constructors, unreferenced symbols)",
                f'FORCE_LINK=""',
                f'for obj in {force_basenames}; do',
                f'  if [ -f "$COMPAT_DIR/$obj" ]; then',
                f'    mv "$COMPAT_DIR/$obj" "$COMPAT_DIR/_force_$obj"',
                f'    FORCE_LINK="$FORCE_LINK $COMPAT_DIR/_force_$obj"',
                f'  fi',
                f'done',
            ])

        lines.extend([
            '%{__ar} rcs "$COMPAT_DIR/libmogrix-compat.a" "$COMPAT_DIR"/*.o',
            'export LIBS="-L$COMPAT_DIR -lmogrix-compat $LIBS"',
        ])

        if force_link_files:
            lines.append('export LDFLAGS="$FORCE_LINK $LDFLAGS"')

        return "\n".join(lines)

    def list_available_functions(self) -> list[str]:
        """List all available function names."""
        return sorted(self.catalog.get("functions", {}).keys())

    def list_available_sets(self) -> list[str]:
        """List all available function sets."""
        return sorted(self.catalog.get("sets", {}).keys())
