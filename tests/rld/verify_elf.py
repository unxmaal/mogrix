#!/usr/bin/env python3
"""
Static ELF verification for IRIX rld test suite.

Runs on the build host (Linux) using readelf. Checks structural properties
of cross-compiled binaries without needing IRIX access.

Usage:
    python3 tests/rld/verify_elf.py              # check all
    python3 tests/rld/verify_elf.py A04           # check specific test
"""

import json
import re
import subprocess
import sys
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent
OUT_DIR = SCRIPT_DIR / "out"
BIN_DIR = OUT_DIR / "bin"
LIB_DIR = OUT_DIR / "lib"

READELF = "readelf"


def run_readelf(path, flags):
    """Run readelf with given flags, return stdout."""
    cmd = [READELF] + flags + [str(path)]
    r = subprocess.run(cmd, capture_output=True, text=True)
    return r.stdout


def check_load_segments(so_path):
    """Verify .so has exactly 2 LOAD segments (RE text + RW data)."""
    out = run_readelf(so_path, ["-l"])
    loads = [line for line in out.splitlines() if line.strip().startswith("LOAD")]
    count = len(loads)
    if count == 2:
        return True, f"OK: {count} LOAD segments"
    return False, f"FAIL: {count} LOAD segments (expected 2)"


def check_no_verneed(so_path):
    """Verify no VERNEED/VERSYM dynamic tags."""
    out = run_readelf(so_path, ["-d"])
    for tag in ["VERNEED", "VERSYM", "VERNEEDNUM"]:
        if tag in out:
            return False, f"FAIL: found {tag} tag (should be stripped)"
    return True, "OK: no version tags"


def check_no_init_array(so_path):
    """Verify .init_array is absent, .ctors is present."""
    sections_out = run_readelf(so_path, ["-S"])
    has_init_array = ".init_array" in sections_out
    has_ctors = ".ctors" in sections_out

    dyn_out = run_readelf(so_path, ["-d"])
    has_dt_init = "INIT" in dyn_out and "INIT_ARRAY" not in dyn_out

    issues = []
    if has_init_array:
        issues.append("has .init_array (rld ignores this)")
    if not has_ctors:
        issues.append("missing .ctors section")
    if "INIT_ARRAY" in dyn_out:
        issues.append("has DT_INIT_ARRAY tag")

    if issues:
        return False, "FAIL: " + "; ".join(issues)
    return True, "OK: uses .ctors, no .init_array"


def check_dt_init(so_path):
    """Verify DT_INIT tag is present."""
    out = run_readelf(so_path, ["-d"])
    if re.search(r'\(INIT\)', out):
        return True, "OK: DT_INIT present"
    return False, "FAIL: no DT_INIT tag"


def check_got_global_count(so_path):
    """Check global GOT entry count: SYMTABNO - GOTSYM < 4370."""
    out = run_readelf(so_path, ["-d"])

    symtabno = None
    gotsym = None
    for line in out.splitlines():
        m = re.search(r'MIPS_SYMTABNO\)?\s+(\d+)', line)
        if m:
            symtabno = int(m.group(1))
        m = re.search(r'MIPS_GOTSYM\)?\s+(\d+)', line)
        if m:
            gotsym = int(m.group(1))

    if symtabno is None or gotsym is None:
        return None, "SKIP: no MIPS GOT tags (not a MIPS .so?)"

    global_got = symtabno - gotsym
    if global_got < 4370:
        return True, f"OK: {global_got} global GOT entries (< 4370)"
    return False, f"FAIL: {global_got} global GOT entries (>= 4370 threshold)"


def check_rld_obj_head(exe_path):
    """Check __rld_obj_head is in .dynsym."""
    out = run_readelf(exe_path, ["--dyn-syms"])
    if "__rld_obj_head" in out:
        return True, "OK: __rld_obj_head in .dynsym"
    return False, "FAIL: __rld_obj_head not in .dynsym"


# Map check names to functions and whether they apply to .so or exe
CHECKS = {
    "check_load_segments": (check_load_segments, "so"),
    "check_no_verneed": (check_no_verneed, "so"),
    "check_no_init_array": (check_no_init_array, "so"),
    "check_dt_init": (check_dt_init, "so"),
    "check_got_global_count": (check_got_global_count, "so"),
    "check_rld_obj_head": (check_rld_obj_head, "exe"),
}


def verify_test(test):
    """Run static checks for a test. Returns list of (check_name, passed, message)."""
    results = []
    checks = test.get("static_checks", [])

    for check_name in checks:
        if check_name not in CHECKS:
            results.append((check_name, None, f"SKIP: unknown check '{check_name}'"))
            continue

        fn, target = CHECKS[check_name]

        if target == "so":
            # Run check on each library
            for lib in test.get("libs", []):
                so_path = LIB_DIR / lib["name"]
                if not so_path.exists():
                    results.append((check_name, None, f"SKIP: {lib['name']} not built"))
                    continue
                passed, msg = fn(so_path)
                results.append((f"{check_name}({lib['name']})", passed, msg))
        elif target == "exe":
            exe_path = BIN_DIR / f"test_{test['name']}"
            if not exe_path.exists():
                results.append((check_name, None, f"SKIP: exe not built"))
                continue
            passed, msg = fn(exe_path)
            results.append((check_name, passed, msg))

    return results


def main():
    sys.path.insert(0, str(SCRIPT_DIR))
    from test_manifest import TESTS, get_test

    # Parse args
    test_ids = sys.argv[1:] if len(sys.argv) > 1 else None

    if test_ids:
        tests = [get_test(tid) for tid in test_ids]
        tests = [t for t in tests if t is not None]
    else:
        tests = TESTS

    all_results = {}
    total_pass = 0
    total_fail = 0
    total_skip = 0

    for test in tests:
        checks = test.get("static_checks", [])
        if not checks:
            continue

        tid = test["id"]
        print(f"{tid} ({test['name']}):")
        results = verify_test(test)
        all_results[tid] = []

        for name, passed, msg in results:
            if passed is None:
                total_skip += 1
                marker = "SKIP"
            elif passed:
                total_pass += 1
                marker = "PASS"
            else:
                total_fail += 1
                marker = "FAIL"
            print(f"  [{marker}] {name}: {msg}")
            all_results[tid].append({"check": name, "passed": passed, "message": msg})

    print(f"\nStatic checks: {total_pass} passed, {total_fail} failed, {total_skip} skipped")

    # Also run all checks on all .so files as a general sweep
    print("\n--- General .so sweep ---")
    for so in sorted(LIB_DIR.glob("*.so")) if LIB_DIR.exists() else []:
        issues = []
        for check_name in ["check_load_segments", "check_no_verneed", "check_no_init_array"]:
            fn, _ = CHECKS[check_name]
            passed, msg = fn(so)
            if passed is False:
                issues.append(msg)
        if issues:
            print(f"  {so.name}: " + "; ".join(issues))
        else:
            print(f"  {so.name}: all checks pass")

    # Save results
    results_file = OUT_DIR / "verify_results.json"
    if OUT_DIR.exists():
        with open(results_file, "w") as f:
            json.dump(all_results, f, indent=2)

    return 1 if total_fail > 0 else 0


if __name__ == "__main__":
    sys.exit(main())
