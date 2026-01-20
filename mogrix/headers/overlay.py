"""Header overlay manager for mogrix."""

from pathlib import Path


class HeaderOverlayManager:
    """Manages header overlay directories for IRIX compatibility."""

    def __init__(self, headers_dir: Path):
        """Initialize with path to headers directory."""
        self.headers_dir = Path(headers_dir)

    def list_headers(self, overlay_name: str) -> list[str]:
        """List all header files in an overlay."""
        overlay_path = self.headers_dir / overlay_name
        if not overlay_path.exists():
            return []

        headers = []
        for path in overlay_path.rglob("*.h"):
            # Get relative path from overlay root
            rel_path = path.relative_to(overlay_path)
            headers.append(str(rel_path))
        return sorted(headers)

    def get_overlay_path(self, overlay_name: str) -> Path | None:
        """Get the full path to an overlay directory."""
        overlay_path = self.headers_dir / overlay_name
        if overlay_path.exists():
            return overlay_path
        return None

    def get_cppflags(
        self,
        overlays: list[str],
        install_prefix: str = "/usr/sgug/include/mogrix-compat",
    ) -> str:
        """Generate CPPFLAGS for the specified overlays.

        Overlays are ordered so more specific ones come first:
        - packages/* overlays first
        - classes/* overlays next
        - generic overlays last

        Args:
            overlays: List of overlay names (e.g., ["generic", "packages/test"])
            install_prefix: Where overlays will be installed on target system

        Returns:
            String with -I flags for all overlays
        """

        # Sort overlays: packages first, then classes, then generic
        def sort_key(name: str) -> tuple[int, str]:
            if name.startswith("packages/"):
                return (0, name)
            elif name.startswith("classes/"):
                return (1, name)
            else:
                return (2, name)

        sorted_overlays = sorted(overlays, key=sort_key)

        flags = []
        for overlay in sorted_overlays:
            overlay_path = self.get_overlay_path(overlay)
            if overlay_path:
                # Use install prefix path for generated flags
                flags.append(f"-I{install_prefix}/{overlay}")

        return " ".join(flags)

    def get_build_cppflags(self, overlays: list[str]) -> str:
        """Generate CPPFLAGS using local paths for build-time use.

        This uses the actual header directory paths rather than
        the target install prefix.
        """

        def sort_key(name: str) -> tuple[int, str]:
            if name.startswith("packages/"):
                return (0, name)
            elif name.startswith("classes/"):
                return (1, name)
            else:
                return (2, name)

        sorted_overlays = sorted(overlays, key=sort_key)

        flags = []
        for overlay in sorted_overlays:
            overlay_path = self.get_overlay_path(overlay)
            if overlay_path:
                flags.append(f"-I{overlay_path}")

        return " ".join(flags)
