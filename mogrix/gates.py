"""Quality gates for mogrix build pipeline.

Gate 2: Build output validation — shebangs, hardcoded paths, ELF ABI.
"""

import re
import struct
import subprocess
import tempfile
from dataclasses import dataclass, field
from pathlib import Path


# IRIX native binaries — these shebangs are valid, don't flag them
IRIX_NATIVE_SHEBANGS = {
    "/bin/sh", "/sbin/sh",
}

# Shebangs that must be rewritten to /usr/sgug/bin/
BAD_SHEBANG_PATTERNS = [
    (re.compile(r"^#!\s*/usr/bin/env\s+(.+)"), "#!/usr/sgug/bin/{0}"),
    (re.compile(r"^#!\s*/usr/bin/(.+)"), "#!/usr/sgug/bin/{0}"),
    (re.compile(r"^#!\s*/bin/(.+)"), "#!/usr/sgug/bin/{0}"),
]

# Known binaries that have /usr/sgug/bin equivalents and should NOT use /usr/bin
SGUG_BINARIES = {
    "perl", "python3", "python", "bash",
    "sed", "awk", "grep", "gawk", "mawk",
}

# Hardcoded path patterns that break relocatability
BAD_PATH_PATTERNS = [
    re.compile(r'use lib "/usr/sgug/share/'),
    re.compile(r"unshift\(@INC,\s*'/usr/sgug/share/"),
]

# ELF constants for MIPS n32
ELF_MAGIC = b"\x7fELF"
ELF_CLASS_32 = 1
ELF_MACHINE_MIPS = 8
ELF_FLAGS_N32 = 0x20  # EF_MIPS_ABI2 (n32)


@dataclass
class GateIssue:
    severity: str  # "error" or "warning"
    file: str
    message: str


@dataclass
class GateResult:
    passed: bool = True
    issues: list[GateIssue] = field(default_factory=list)

    def error(self, file: str, message: str):
        self.issues.append(GateIssue("error", file, message))
        self.passed = False

    def warning(self, file: str, message: str):
        self.issues.append(GateIssue("warning", file, message))


def check_shebang(filepath: Path, content_first_line: str) -> GateIssue | None:
    """Check if a script has a bad shebang."""
    line = content_first_line.strip()
    if not line.startswith("#!"):
        return None

    # Extract the interpreter path (strip flags like "#!/bin/sh -")
    interp = line[2:].strip().split()[0] if len(line) > 2 else ""

    # IRIX native shebangs are fine
    if interp in IRIX_NATIVE_SHEBANGS:
        return None

    for pattern, _ in BAD_SHEBANG_PATTERNS:
        m = pattern.match(line)
        if m:
            binary = m.group(1).split()[0]  # handle "env perl" -> "perl"
            # /usr/bin/env shebangs work if PATH includes /usr/sgug/bin/ (bundles do this)
            if interp == "/usr/bin/env":
                return GateIssue(
                    "warning",
                    str(filepath),
                    f"env shebang: {line} (works in bundles, but direct /usr/sgug/bin/ is safer)",
                )
            if binary in SGUG_BINARIES:
                return GateIssue(
                    "error",
                    str(filepath),
                    f"Bad shebang: {line} (should use /usr/sgug/bin/)",
                )
    return None


def check_elf_abi(filepath: Path) -> GateIssue | None:
    """Check if an ELF binary is MIPS n32."""
    try:
        with open(filepath, "rb") as f:
            magic = f.read(4)
            if magic != ELF_MAGIC:
                return None
            ei_class = struct.unpack("B", f.read(1))[0]
            if ei_class != ELF_CLASS_32:
                return GateIssue(
                    "error", str(filepath),
                    f"Wrong ELF class: {ei_class} (expected 32-bit/n32)",
                )
            f.seek(18)  # e_machine offset
            e_machine = struct.unpack(">H", f.read(2))[0]  # big-endian MIPS
            if e_machine != ELF_MACHINE_MIPS:
                return GateIssue(
                    "error", str(filepath),
                    f"Wrong ELF machine: {e_machine} (expected MIPS/{ELF_MACHINE_MIPS})",
                )
    except (OSError, struct.error):
        pass
    return None


def scan_rpm(rpm_path: Path) -> GateResult:
    """Scan a single RPM for Gate 2 issues."""
    result = GateResult()

    with tempfile.TemporaryDirectory() as tmpdir:
        # Extract RPM
        try:
            subprocess.run(
                f"cd {tmpdir} && rpm2cpio {rpm_path} | cpio -idm 2>/dev/null",
                shell=True, check=True, capture_output=True,
            )
        except subprocess.CalledProcessError:
            result.error(str(rpm_path), "Failed to extract RPM")
            return result

        tmppath = Path(tmpdir)

        # Scan all files
        for filepath in tmppath.rglob("*"):
            if not filepath.is_file() or filepath.is_symlink():
                continue

            rel = filepath.relative_to(tmppath)

            # ELF check for .so files and binaries in bin/sbin
            # Skip known host-native tool directories (e.g., qt5/bin/ has x86_64 build tools)
            if filepath.suffix in (".so", "") or ".so." in filepath.name:
                rel_str = str(rel)
                if "/qt5/bin/" not in rel_str and "/native/" not in rel_str:
                    issue = check_elf_abi(filepath)
                    if issue:
                        issue.file = rel_str
                        result.issues.append(issue)
                        result.passed = False

            # Script check — look at first line for shebang
            # Only check files in bin/ or sbin/ directories
            parts = rel.parts
            if any(p in ("bin", "sbin") for p in parts):
                try:
                    with open(filepath, "r", errors="replace") as f:
                        first_line = f.readline()
                except (OSError, UnicodeDecodeError):
                    continue

                issue = check_shebang(rel, first_line)
                if issue:
                    result.issues.append(issue)
                    result.passed = False

            # Hardcoded path check for scripts
            if filepath.suffix in (".pl", ".pm", ".py", ".sh", ""):
                try:
                    content = filepath.read_text(errors="replace")
                except OSError:
                    continue
                for pat in BAD_PATH_PATTERNS:
                    if pat.search(content):
                        result.warning(
                            str(rel),
                            f"Hardcoded path pattern: {pat.pattern}",
                        )

    return result


def pre_scan_rpms(rpm_dir: Path) -> dict[str, GateResult]:
    """Scan all RPMs in a directory for Gate 2 issues."""
    results = {}
    rpms = sorted(rpm_dir.glob("*.rpm"))

    for rpm_path in rpms:
        if rpm_path.name.endswith(".src.rpm"):
            continue
        results[rpm_path.name] = scan_rpm(rpm_path)

    return results
