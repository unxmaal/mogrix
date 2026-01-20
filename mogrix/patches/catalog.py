"""Patch catalog for mogrix.

Manages IRIX-specific patches organized by package and version.
"""

from pathlib import Path


class PatchCatalog:
    """Manages patch files for IRIX compatibility."""

    def __init__(self, patches_dir: Path):
        """Initialize with path to patches directory."""
        self.patches_dir = patches_dir

    def list_generic(self) -> list[Path]:
        """List all generic patches that apply to all packages."""
        generic_dir = self.patches_dir / "generic"
        if not generic_dir.exists():
            return []
        return sorted(generic_dir.glob("*.patch"))

    def list_package(self, package: str, version: str | None = None) -> list[Path]:
        """List patches for a specific package.

        Args:
            package: Package name
            version: Optional version string (e.g., "3.8")

        Returns:
            List of patch file paths
        """
        pkg_dir = self.patches_dir / "packages" / package
        if not pkg_dir.exists():
            return []

        patches = []

        # Get unversioned patches (apply to all versions)
        patches.extend(sorted(pkg_dir.glob("*.patch")))

        # Get versioned patches if version specified
        if version:
            ver_dir = pkg_dir / version
            if ver_dir.exists():
                patches.extend(sorted(ver_dir.glob("*.patch")))

        return patches

    def get_content(self, relative_path: str) -> str:
        """Get the content of a patch file.

        Args:
            relative_path: Path relative to patches directory

        Returns:
            Patch file content
        """
        patch_path = self.patches_dir / relative_path
        return patch_path.read_text()

    def get_spec_entries(self, patches: list[Path], start_num: int = 100) -> str:
        """Generate spec file Patch entries.

        Args:
            patches: List of patch file paths
            start_num: Starting patch number

        Returns:
            Spec file lines for patch declarations
        """
        lines = []
        for i, patch in enumerate(patches):
            num = start_num + i
            lines.append(f"Patch{num}: {patch.name}")
        return "\n".join(lines)

    def get_prep_commands(
        self, patches: list[Path], start_num: int = 100, strip: int = 1
    ) -> str:
        """Generate %prep section patch application commands.

        Args:
            patches: List of patch file paths
            start_num: Starting patch number (must match get_spec_entries)
            strip: -p level for patch command (default: 1)

        Returns:
            Commands to apply patches
        """
        lines = []
        for i, _ in enumerate(patches):
            num = start_num + i
            # Use modern RPM %patch macro syntax
            lines.append(f"%patch -P{num} -p{strip}")
        return "\n".join(lines)

    def resolve_patches(
        self,
        package: str,
        version: str | None = None,
        include_generic: bool = False,
    ) -> list[Path]:
        """Resolve all applicable patches for a package.

        Args:
            package: Package name
            version: Optional version string
            include_generic: Whether to include generic patches

        Returns:
            List of all applicable patch paths
        """
        patches = []

        if include_generic:
            patches.extend(self.list_generic())

        patches.extend(self.list_package(package, version))

        return patches
