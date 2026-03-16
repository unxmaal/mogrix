#!/usr/bin/env python3
"""Scan all built RPMs for known defect patterns without rebuilding."""

import subprocess, sys, os, tempfile, shutil
from pathlib import Path
from collections import defaultdict

READELF = "/opt/cross/bin/mips-sgi-irix6.5-readelf"
NM = "/opt/cross/bin/mips-sgi-irix6.5-nm"

# Known bad patterns
BAD_IMAGE_BASE = 0x5ffe0000
GOOD_IMAGE_BASE = 0x0f800000
POISON_SYMBOLS = {"__stack_chk_fail", "__stack_chk_guard"}

def extract_rpm(rpm_path, tmpdir):
    """Extract RPM to tmpdir, return list of ELF files."""
    subprocess.run(
        f"cd {tmpdir} && rpm2cpio {rpm_path} | cpio -idm 2>/dev/null",
        shell=True, capture_output=True
    )
    elfs = []
    for root, dirs, files in os.walk(tmpdir):
        for f in files:
            fp = Path(root) / f
            if fp.is_symlink():
                continue
            if fp.suffix in ('.o', '.a', '.la', '.h', '.pc', '.py', '.pl', '.sh'):
                continue
            try:
                with open(fp, 'rb') as fh:
                    if fh.read(4) == b'\x7fELF':
                        elfs.append(fp)
            except:
                pass
    return elfs

def check_elf(elf_path, rpm_name):
    """Check a single ELF for defects. Returns list of (severity, message)."""
    issues = []
    basename = elf_path.name
    is_so = ".so" in basename
    
    # Check dynamic section
    result = subprocess.run([READELF, "-d", str(elf_path)], capture_output=True, text=True)
    dyn = result.stdout
    
    if is_so:
        # Check image base
        for line in dyn.splitlines():
            if "MIPS_BASE_ADDRESS" in line:
                parts = line.strip().split()
                try:
                    base = int(parts[-1], 16)
                    if base == BAD_IMAGE_BASE:
                        issues.append(("CRITICAL", f"old image-base 0x{base:x} (should be 0x{GOOD_IMAGE_BASE:x})"))
                except:
                    pass
    
    # Check for poison symbols (undefined)
    result = subprocess.run([NM, "-D", str(elf_path)], capture_output=True, text=True)
    for line in result.stdout.splitlines():
        parts = line.strip().split()
        if len(parts) >= 2 and parts[-2] == "U":
            sym = parts[-1]
            if sym in POISON_SYMBOLS:
                issues.append(("ERROR", f"needs {sym} (IRIX has no stack protector)"))
    
    # Check for GNU version sections (crash rld)
    result = subprocess.run([READELF, "-S", str(elf_path)], capture_output=True, text=True)
    for line in result.stdout.splitlines():
        if ".gnu.version_r" in line or ".gnu.version_d" in line:
            issues.append(("ERROR", f"has GNU version section (crashes IRIX rld)"))
            break
    
    # Check NEEDED for common mismatches
    needed = set()
    for line in dyn.splitlines():
        if "(NEEDED)" in line:
            soname = line.split("[")[1].split("]")[0] if "[" in line else ""
            if soname:
                needed.add(soname)
    
    return issues, needed

def main():
    rpms_dir = Path(sys.argv[1]) if len(sys.argv) > 1 else Path.home() / "mogrix_outputs" / "RPMS"
    rpms = sorted(rpms_dir.glob("*.mips.rpm"))
    print(f"Scanning {len(rpms)} RPMs in {rpms_dir}...\n")
    
    all_issues = []
    all_sonames = set()  # sonames provided by our RPMs
    all_needed = defaultdict(set)  # soname -> set of RPMs that need it
    
    # First pass: collect all provided sonames
    for rpm in rpms:
        filelist = subprocess.run(
            ["rpm", "-qpl", str(rpm)], capture_output=True, text=True
        ).stdout.strip().split("\n")
        for f in filelist:
            if "/lib32/" in f and ".so" in f:
                all_sonames.add(os.path.basename(f))
    
    # Second pass: scan ELFs
    scanned = 0
    for rpm in rpms:
        tmpdir = tempfile.mkdtemp()
        try:
            elfs = extract_rpm(rpm, tmpdir)
            for elf in elfs:
                issues, needed = check_elf(elf, rpm.name)
                scanned += 1
                for sev, msg in issues:
                    all_issues.append((sev, rpm.name, elf.name, msg))
                for soname in needed:
                    all_needed[soname].add(rpm.name)
        finally:
            shutil.rmtree(tmpdir, ignore_errors=True)
    
    # Report
    crits = [(s, r, e, m) for s, r, e, m in all_issues if s == "CRITICAL"]
    errors = [(s, r, e, m) for s, r, e, m in all_issues if s == "ERROR"]
    warns = [(s, r, e, m) for s, r, e, m in all_issues if s == "WARNING"]
    
    if crits:
        print(f"{'='*60}")
        print(f"CRITICAL ({len(crits)})")
        print(f"{'='*60}")
        for _, rpm, elf, msg in sorted(crits):
            print(f"  {rpm}: {elf}: {msg}")
    
    if errors:
        print(f"\n{'='*60}")
        print(f"ERRORS ({len(errors)})")
        print(f"{'='*60}")
        for _, rpm, elf, msg in sorted(errors):
            print(f"  {rpm}: {elf}: {msg}")
    
    if warns:
        print(f"\nWARNINGS ({len(warns)})")
        for _, rpm, elf, msg in sorted(warns):
            print(f"  {rpm}: {elf}: {msg}")
    
    if not all_issues:
        print("No defects found!")
    
    print(f"\n{'='*60}")
    print(f"Scanned {scanned} ELF files across {len(rpms)} RPMs")
    print(f"  Critical: {len(crits)}")
    print(f"  Errors: {len(errors)}")
    print(f"  Warnings: {len(warns)}")

if __name__ == "__main__":
    main()
