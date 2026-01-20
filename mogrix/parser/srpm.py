"""SRPM extraction for mogrix."""

import subprocess
import tempfile
from pathlib import Path


class SRPMExtractor:
    """Extracts contents from SRPM files."""

    def __init__(self, srpm_path: Path):
        """Initialize with path to SRPM.

        Args:
            srpm_path: Path to the .src.rpm file
        """
        self.srpm_path = Path(srpm_path)
        if not self.srpm_path.exists():
            raise FileNotFoundError(f"SRPM not found: {srpm_path}")

    def extract(self, dest_dir: Path | None = None) -> Path:
        """Extract SRPM contents.

        Args:
            dest_dir: Destination directory (creates temp if None)

        Returns:
            Path to directory containing extracted files
        """
        if dest_dir is None:
            dest_dir = Path(tempfile.mkdtemp(prefix="mogrix-srpm-"))
        else:
            dest_dir = Path(dest_dir)
            dest_dir.mkdir(parents=True, exist_ok=True)

        # Use rpm2cpio | cpio to extract
        try:
            # rpm2cpio outputs cpio archive to stdout
            rpm2cpio = subprocess.Popen(
                ["rpm2cpio", str(self.srpm_path)],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
            )

            # cpio extracts from stdin
            cpio = subprocess.Popen(
                ["cpio", "-idmv"],
                stdin=rpm2cpio.stdout,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                cwd=str(dest_dir),
            )

            # Close rpm2cpio stdout to allow it to receive SIGPIPE
            if rpm2cpio.stdout:
                rpm2cpio.stdout.close()

            _, cpio_err = cpio.communicate()
            _, rpm2cpio_err = rpm2cpio.communicate()

            # Check for errors
            if rpm2cpio.returncode != 0:
                err_msg = rpm2cpio_err.decode().strip() if rpm2cpio_err else f"exit code {rpm2cpio.returncode}"
                raise RuntimeError(f"rpm2cpio failed: {err_msg}")

            if cpio.returncode != 0:
                raise RuntimeError(f"cpio failed: {cpio_err.decode()}")

        except FileNotFoundError as e:
            raise RuntimeError(
                "rpm2cpio or cpio not found. Install rpm-tools or equivalent."
            ) from e

        return dest_dir

    def get_spec_path(self, extracted_dir: Path) -> Path | None:
        """Find the spec file in extracted directory.

        Args:
            extracted_dir: Directory with extracted SRPM contents

        Returns:
            Path to spec file or None if not found
        """
        specs = list(extracted_dir.glob("*.spec"))
        if len(specs) == 1:
            return specs[0]
        elif len(specs) > 1:
            # Multiple specs - try to find by SRPM name
            srpm_name = self.srpm_path.stem.rsplit("-", 2)[0]
            for spec in specs:
                if spec.stem == srpm_name:
                    return spec
            return specs[0]  # Return first if no match
        return None

    def extract_spec(self, dest_dir: Path | None = None) -> tuple[Path, Path]:
        """Extract SRPM and return spec file path.

        Args:
            dest_dir: Destination directory

        Returns:
            Tuple of (extracted_dir, spec_path)
        """
        extracted = self.extract(dest_dir)
        spec = self.get_spec_path(extracted)

        if spec is None:
            raise RuntimeError(f"No spec file found in {self.srpm_path}")

        return extracted, spec
