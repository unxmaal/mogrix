"""Staging environment management for mogrix cross-compilation.

This module ensures the staging environment has all base resources needed
for cross-compilation before any build takes place.
"""

import shutil
import subprocess
from dataclasses import dataclass, field
from pathlib import Path

from rich.console import Console

console = Console()


@dataclass
class StagingConfig:
    """Configuration for the staging environment."""

    staging_dir: Path = field(default_factory=lambda: Path("/opt/sgug-staging/usr/sgug"))
    cross_bindir: Path = field(default_factory=lambda: Path("/opt/cross/bin"))
    mogrix_dir: Path = field(default_factory=lambda: Path(__file__).parent.parent)

    @property
    def lib32_dir(self) -> Path:
        return self.staging_dir / "lib32"

    @property
    def include_dir(self) -> Path:
        return self.staging_dir / "include"

    @property
    def bin_dir(self) -> Path:
        return self.staging_dir / "bin"

    @property
    def compat_runtime_dir(self) -> Path:
        return self.mogrix_dir / "compat" / "runtime"

    @property
    def cross_include_dir(self) -> Path:
        return self.mogrix_dir / "cross" / "include"

    @property
    def compat_include_dir(self) -> Path:
        return self.mogrix_dir / "compat" / "include"


@dataclass
class StagingStatus:
    """Status of the staging environment."""

    is_ready: bool = False
    missing_resources: list[str] = field(default_factory=list)
    created_resources: list[str] = field(default_factory=list)
    errors: list[str] = field(default_factory=list)


class StagingManager:
    """Manages the staging environment for cross-compilation.

    Ensures all base resources (runtime libraries, compat headers) are
    present before builds can proceed.
    """

    # Base resources that must exist for any build
    REQUIRED_LIBS = [
        "libsoft_float_stubs.a",
        "libatomic.a",
    ]

    REQUIRED_HEADERS = [
        "dicl-clang-compat",
        "mogrix-compat",
        "irix-compat.h",
        "getopt.h",
    ]

    # Runtime source files and their output libraries
    RUNTIME_SOURCES = {
        "soft_float_stubs.c": "libsoft_float_stubs.a",
        "libatomic_stub.c": "libatomic.a",
    }

    def __init__(self, config: StagingConfig | None = None):
        self.config = config or StagingConfig()

    def check_status(self) -> StagingStatus:
        """Check if staging environment has all required resources.

        Returns:
            StagingStatus with details about what's missing
        """
        status = StagingStatus()
        status.is_ready = True

        # Check libraries
        for lib in self.REQUIRED_LIBS:
            lib_path = self.config.lib32_dir / lib
            if not lib_path.exists():
                status.missing_resources.append(f"lib32/{lib}")
                status.is_ready = False

        # Check headers
        for header in self.REQUIRED_HEADERS:
            header_path = self.config.include_dir / header
            if not header_path.exists():
                status.missing_resources.append(f"include/{header}")
                status.is_ready = False

        return status

    def ensure_ready(self, verbose: bool = False) -> StagingStatus:
        """Ensure staging environment is ready, creating missing resources.

        This is the main entry point - call this before any build operation.

        Args:
            verbose: If True, print detailed progress

        Returns:
            StagingStatus with details about what was created
        """
        status = self.check_status()

        if status.is_ready:
            if verbose:
                console.print("[dim]Staging environment is ready[/dim]")
            return status

        if verbose:
            console.print("[bold]Setting up staging environment...[/bold]")

        # Ensure directories exist
        self.config.lib32_dir.mkdir(parents=True, exist_ok=True)
        self.config.include_dir.mkdir(parents=True, exist_ok=True)

        # Copy headers first - compiler needs them to build runtime libs
        self._ensure_headers(status, verbose)

        # Build missing runtime libraries (requires headers to be in place)
        self._ensure_runtime_libs(status, verbose)

        # Re-check status
        final_status = self.check_status()
        final_status.created_resources = status.created_resources
        final_status.errors = status.errors

        if verbose:
            if final_status.is_ready:
                console.print("[green]Staging environment ready[/green]")
            else:
                console.print("[red]Staging environment setup incomplete[/red]")
                for err in final_status.errors:
                    console.print(f"  [red]![/red] {err}")

        return final_status

    def _ensure_runtime_libs(self, status: StagingStatus, verbose: bool) -> None:
        """Build and install missing runtime libraries."""
        irix_cc = self.config.bin_dir / "irix-cc"
        llvm_ar = self.config.cross_bindir / "llvm-ar"

        if not irix_cc.exists():
            status.errors.append(f"Compiler wrapper not found: {irix_cc}")
            return

        if not llvm_ar.exists():
            status.errors.append(f"llvm-ar not found: {llvm_ar}")
            return

        for src_name, lib_name in self.RUNTIME_SOURCES.items():
            lib_path = self.config.lib32_dir / lib_name
            if lib_path.exists():
                continue

            src_path = self.config.compat_runtime_dir / src_name
            if not src_path.exists():
                status.errors.append(f"Runtime source not found: {src_path}")
                continue

            if verbose:
                console.print(f"  Building {lib_name}...")

            try:
                # Compile to object file
                obj_path = self.config.lib32_dir / src_name.replace(".c", ".o")
                compile_result = subprocess.run(
                    [str(irix_cc), "-c", str(src_path), "-o", str(obj_path)],
                    capture_output=True,
                    text=True,
                )

                if compile_result.returncode != 0:
                    status.errors.append(f"Failed to compile {src_name}: {compile_result.stderr}")
                    continue

                # Create static library
                ar_result = subprocess.run(
                    [str(llvm_ar), "rcs", str(lib_path), str(obj_path)],
                    capture_output=True,
                    text=True,
                )

                if ar_result.returncode != 0:
                    status.errors.append(f"Failed to archive {lib_name}: {ar_result.stderr}")
                    continue

                # Clean up object file
                obj_path.unlink(missing_ok=True)

                status.created_resources.append(f"lib32/{lib_name}")
                if verbose:
                    console.print(f"    [green]Created {lib_name}[/green]")

            except Exception as e:
                status.errors.append(f"Error building {lib_name}: {e}")

    def _ensure_headers(self, status: StagingStatus, verbose: bool) -> None:
        """Copy missing compat headers to staging."""
        # dicl-clang-compat headers
        src_dicl = self.config.cross_include_dir / "dicl-clang-compat"
        dst_dicl = self.config.include_dir / "dicl-clang-compat"

        if src_dicl.exists() and not dst_dicl.exists():
            if verbose:
                console.print("  Copying dicl-clang-compat headers...")
            try:
                shutil.copytree(src_dicl, dst_dicl)
                status.created_resources.append("include/dicl-clang-compat")
            except Exception as e:
                status.errors.append(f"Failed to copy dicl-clang-compat: {e}")

        # mogrix-compat headers
        src_mogrix = self.config.compat_include_dir / "mogrix-compat"
        dst_mogrix = self.config.include_dir / "mogrix-compat"

        if src_mogrix.exists() and not dst_mogrix.exists():
            if verbose:
                console.print("  Copying mogrix-compat headers...")
            try:
                shutil.copytree(src_mogrix, dst_mogrix)
                status.created_resources.append("include/mogrix-compat")
            except Exception as e:
                status.errors.append(f"Failed to copy mogrix-compat: {e}")

        # Individual header files (from cross/include/)
        individual_headers = ["irix-compat.h", "getopt.h"]
        for header in individual_headers:
            src_path = self.config.cross_include_dir / header
            dst_path = self.config.include_dir / header

            if src_path.exists() and not dst_path.exists():
                if verbose:
                    console.print(f"  Copying {header}...")
                try:
                    shutil.copy2(src_path, dst_path)
                    status.created_resources.append(f"include/{header}")
                except Exception as e:
                    status.errors.append(f"Failed to copy {header}: {e}")


def ensure_staging_ready(
    staging_dir: str | Path | None = None,
    verbose: bool = False,
) -> StagingStatus:
    """Convenience function to ensure staging is ready.

    Args:
        staging_dir: Optional custom staging directory
        verbose: If True, print progress

    Returns:
        StagingStatus with details about the staging environment
    """
    config = StagingConfig()
    if staging_dir:
        config.staging_dir = Path(staging_dir)

    manager = StagingManager(config)
    return manager.ensure_ready(verbose=verbose)
