"""SRPM emitter for mogrix - packages converted specs into SRPMs."""

import subprocess
import tempfile
from pathlib import Path


class SRPMEmitter:
    """Emits SRPM files from converted specs and sources."""

    def __init__(self, rpmbuild_dir: Path | None = None):
        """Initialize SRPM emitter.

        Args:
            rpmbuild_dir: Base directory for rpmbuild (default: ~/rpmbuild)
        """
        self.rpmbuild_dir = Path(rpmbuild_dir) if rpmbuild_dir else Path.home() / "rpmbuild"

    def setup_rpmbuild_tree(self) -> None:
        """Create the rpmbuild directory structure."""
        for subdir in ["SOURCES", "SPECS", "BUILD", "RPMS", "SRPMS"]:
            (self.rpmbuild_dir / subdir).mkdir(parents=True, exist_ok=True)

    def emit_srpm(
        self,
        spec_content: str,
        spec_name: str,
        sources: list[Path] | None = None,
        patches: list[Path] | None = None,
        compat_sources: list[Path] | None = None,
        output_dir: Path | None = None,
    ) -> Path:
        """Create an SRPM from spec content and sources.

        Args:
            spec_content: The converted spec file content
            spec_name: Name for the spec file (e.g., "popt.spec")
            sources: List of source files to include
            patches: List of patch files to include
            compat_sources: List of compat source files to bundle
            output_dir: Where to copy the final SRPM (default: current dir)

        Returns:
            Path to the created SRPM file
        """
        import shutil

        self.setup_rpmbuild_tree()

        # Write spec to SPECS directory
        spec_path = self.rpmbuild_dir / "SPECS" / spec_name
        spec_path.write_text(spec_content)

        # Copy sources to SOURCES directory
        sources_dir = self.rpmbuild_dir / "SOURCES"

        if sources:
            for src in sources:
                if src.exists():
                    shutil.copy2(src, sources_dir / src.name)

        if patches:
            for patch in patches:
                if patch.exists():
                    shutil.copy2(patch, sources_dir / patch.name)

        if compat_sources:
            for compat in compat_sources:
                if compat.exists():
                    shutil.copy2(compat, sources_dir / compat.name)

        # Build SRPM using rpmbuild -bs
        cmd = [
            "rpmbuild",
            "-bs",
            "--define", f"_topdir {self.rpmbuild_dir}",
            str(spec_path),
        ]

        result = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
        )

        if result.returncode != 0:
            raise RuntimeError(f"rpmbuild failed: {result.stderr}")

        # Find the created SRPM
        srpms = list((self.rpmbuild_dir / "SRPMS").glob("*.src.rpm"))
        if not srpms:
            raise RuntimeError("No SRPM created")

        # Get the most recent SRPM
        srpm_path = max(srpms, key=lambda p: p.stat().st_mtime)

        # Copy to output directory if specified
        if output_dir:
            output_dir = Path(output_dir)
            output_dir.mkdir(parents=True, exist_ok=True)
            dest = output_dir / srpm_path.name
            shutil.copy2(srpm_path, dest)
            return dest

        return srpm_path

    def emit_from_converted(
        self,
        original_srpm: Path,
        converted_spec_content: str,
        compat_sources: list[Path] | None = None,
        output_dir: Path | None = None,
    ) -> Path:
        """Create a new SRPM from an original SRPM with converted spec.

        This extracts sources from the original SRPM and combines them
        with the converted spec to create a new SRPM.

        Args:
            original_srpm: Path to the original Fedora SRPM
            converted_spec_content: The converted spec file content
            compat_sources: Additional compat source files to bundle
            output_dir: Where to place the output SRPM

        Returns:
            Path to the created SRPM file
        """
        from mogrix.parser.srpm import SRPMExtractor
        import shutil

        # Extract original SRPM
        extractor = SRPMExtractor(original_srpm)
        with tempfile.TemporaryDirectory(prefix="mogrix-emit-") as tmpdir:
            extracted_dir = Path(tmpdir) / "extracted"
            extracted_dir.mkdir()

            extractor.extract(extracted_dir)

            # Find spec file and sources
            spec_files = list(extracted_dir.glob("*.spec"))
            if not spec_files:
                raise RuntimeError("No spec file found in original SRPM")

            spec_name = spec_files[0].name

            # Gather all non-spec files as sources
            sources = [
                f for f in extracted_dir.iterdir()
                if f.is_file() and not f.name.endswith(".spec")
            ]

            return self.emit_srpm(
                spec_content=converted_spec_content,
                spec_name=spec_name,
                sources=sources,
                compat_sources=compat_sources,
                output_dir=output_dir,
            )


def create_srpm(
    spec_content: str,
    spec_name: str,
    sources: list[Path] | None = None,
    output_dir: Path | None = None,
) -> Path:
    """Convenience function to create an SRPM.

    Args:
        spec_content: The spec file content
        spec_name: Name for the spec file
        sources: List of source files
        output_dir: Output directory

    Returns:
        Path to created SRPM
    """
    emitter = SRPMEmitter()
    return emitter.emit_srpm(
        spec_content=spec_content,
        spec_name=spec_name,
        sources=sources,
        output_dir=output_dir,
    )
