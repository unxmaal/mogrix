"""Post-build ELF dependency validation for mogrix cross-compiled RPMs.

Validates that all NEEDED sonames in built RPMs can be resolved at runtime
on IRIX — either from the RPMs themselves, existing mogrix RPMs, the IRIX
sysroot, or the toolchain.
"""

import os
import re
import subprocess
import tempfile
from pathlib import Path

from rich.console import Console

console = Console()

# Toolchain libs always available at runtime
TOOLCHAIN_SONAMES = {
    "libgcc_s.so.1",
    "libstdc++.so.6",
    "libatomic.so.1",
    "libmogrix_compat.so",
}

# Pre-existing libs in staging (from IRIX or pre-built, always available)
PREEXISTING_SONAMES = {
    "libz.so",
    "libbz2.so.1",
    "libncursesw.so",
    "libncurses.so",
    "libreadline.so.8",
    "liblzma.so.5",
    "libtinfo.so",
}


def _is_elf(filepath: Path) -> bool:
    """Check if file is an ELF binary."""
    try:
        with open(filepath, "rb") as f:
            magic = f.read(4)
        return magic == b"\x7fELF"
    except (OSError, PermissionError):
        return False


def _is_mips_elf(filepath: Path) -> bool:
    """Check if an ELF file is MIPS architecture (e_machine == 8).

    Cross-compiled packages may contain host (x86-64) build tools like
    qmake, moc, etc. These should be skipped during dep validation.
    """
    try:
        with open(filepath, "rb") as f:
            header = f.read(20)
        if len(header) < 20 or header[:4] != b"\x7fELF":
            return False
        # e_machine is at offset 18 (2 bytes), little-endian or big-endian
        ei_data = header[5]  # 1 = little-endian, 2 = big-endian
        if ei_data == 1:
            e_machine = int.from_bytes(header[18:20], "little")
        else:
            e_machine = int.from_bytes(header[18:20], "big")
        return e_machine == 8  # EM_MIPS
    except (OSError, PermissionError):
        return False


def _readelf_needed(elf_path: Path) -> list[str]:
    """Get NEEDED sonames from an ELF binary using readelf."""
    result = subprocess.run(
        ["readelf", "-d", str(elf_path)],
        capture_output=True,
        text=True,
    )
    if result.returncode != 0:
        return []

    needed = []
    for line in result.stdout.splitlines():
        match = re.search(r"\(NEEDED\)\s+Shared library:\s+\[(.+?)\]", line)
        if match:
            needed.append(match.group(1))
    return needed


def _scan_elf_needed(extract_dir: Path) -> tuple[set[str], set[str]]:
    """Find all NEEDED sonames and PROVIDED sonames from ELF files in a directory.

    Returns (needed_sonames, provided_sonames).
    """
    needed = set()
    provided = set()
    for root, _dirs, files in os.walk(extract_dir):
        for filename in files:
            filepath = Path(root) / filename
            if filepath.is_symlink():
                # Symlinks to .so files also count as provided
                if ".so" in filename:
                    provided.add(filename)
                continue
            if _is_elf(filepath):
                # Skip non-MIPS binaries (e.g. x86-64 host build tools)
                if not _is_mips_elf(filepath):
                    continue
                for soname in _readelf_needed(filepath):
                    needed.add(soname)
                # If this is a .so file, it provides its own soname
                if ".so" in filename:
                    provided.add(filename)
                    # Also check SONAME from ELF header
                    result = subprocess.run(
                        ["readelf", "-d", str(filepath)],
                        capture_output=True,
                        text=True,
                    )
                    for line in result.stdout.splitlines():
                        match = re.search(
                            r"\(SONAME\)\s+Library soname:\s+\[(.+?)\]", line
                        )
                        if match:
                            provided.add(match.group(1))
    return needed, provided


def _build_irix_sonames(irix_sysroot: Path) -> set[str]:
    """Collect sonames available in the IRIX sysroot."""
    sonames = set()
    for sysroot_dir in [
        irix_sysroot / "usr" / "lib32",
        irix_sysroot / "lib32",
    ]:
        if sysroot_dir.is_dir():
            for entry in sysroot_dir.iterdir():
                if ".so" in entry.name:
                    sonames.add(entry.name)
    return sonames


def _build_mogrix_sonames(rpms_dir: Path) -> set[str]:
    """Collect sonames from existing mogrix RPMs (already built packages)."""
    sonames = set()
    if not rpms_dir.is_dir():
        return sonames
    for rpm_path in rpms_dir.glob("*.rpm"):
        result = subprocess.run(
            ["rpm", "-qpl", str(rpm_path)],
            capture_output=True,
            text=True,
        )
        if result.returncode != 0:
            continue
        for line in result.stdout.splitlines():
            line = line.strip()
            if "/lib32/" in line and ".so" in line:
                sonames.add(os.path.basename(line))
    return sonames


def validate_rpm_deps(
    rpm_paths: list[Path],
    rpms_dir: Path,
    irix_sysroot: Path,
) -> dict[str, list[str]]:
    """Validate that all NEEDED sonames in the given RPMs can be resolved.

    Resolution order:
    1. Self-provided (libs within the RPMs being validated)
    2. Existing mogrix RPMs in rpms_dir
    3. IRIX sysroot native libs
    4. Toolchain allowlist
    5. Pre-existing staging libs

    Returns dict mapping RPM filename -> list of unresolved sonames.
    Empty dict means all deps resolve.
    """
    irix_sonames = _build_irix_sonames(irix_sysroot)
    mogrix_sonames = _build_mogrix_sonames(rpms_dir)

    # Extract all RPMs being validated and collect their needs/provides
    all_needed: dict[str, set[str]] = {}  # rpm_name -> needed sonames
    all_provided: set[str] = set()

    for rpm_path in rpm_paths:
        rpm_name = rpm_path.name
        with tempfile.TemporaryDirectory() as tmpdir:
            tmppath = Path(tmpdir)
            subprocess.run(
                f"cd {tmppath} && rpm2cpio {rpm_path.absolute()} | cpio -idm 2>/dev/null",
                shell=True,
                capture_output=True,
            )
            needed, provided = _scan_elf_needed(tmppath)
            all_needed[rpm_name] = needed
            all_provided.update(provided)

    # Build the full set of available sonames
    available = set()
    available.update(all_provided)       # self-provided
    available.update(mogrix_sonames)     # existing mogrix RPMs
    available.update(irix_sonames)       # IRIX sysroot
    available.update(TOOLCHAIN_SONAMES)  # toolchain
    available.update(PREEXISTING_SONAMES)  # pre-existing staging

    # Check each RPM's needs against available
    unresolved: dict[str, list[str]] = {}
    for rpm_name, needed in all_needed.items():
        missing = []
        for soname in sorted(needed):
            # Skip path-based references (build artifacts, not real deps)
            if "/" in soname:
                continue
            if soname not in available:
                missing.append(soname)
        if missing:
            unresolved[rpm_name] = missing

    return unresolved
