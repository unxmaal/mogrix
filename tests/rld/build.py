#!/usr/bin/env python3
"""
Build system for IRIX rld test suite.

Uses irix-cc/irix-ld to cross-compile test binaries for IRIX MIPS n32.
Each test builds independently — partial failures don't block others.

Usage:
    python3 tests/rld/build.py              # build all
    python3 tests/rld/build.py A04 D21      # build specific tests
    python3 tests/rld/build.py --priority 1  # build priority 1 only
"""

import json
import os
import subprocess
import sys
from pathlib import Path

# Resolve paths relative to this script
SCRIPT_DIR = Path(__file__).resolve().parent
PROJECT_ROOT = SCRIPT_DIR.parent.parent
SRC_DIR = SCRIPT_DIR / "src"
OUT_DIR = SCRIPT_DIR / "out"
BIN_DIR = OUT_DIR / "bin"
LIB_DIR = OUT_DIR / "lib"
OBJ_DIR = OUT_DIR / "obj"

# Cross-compilation tools
CROSS_DIR = PROJECT_ROOT / "cross" / "bin"
CC = str(CROSS_DIR / "irix-cc")
CXX = str(CROSS_DIR / "irix-cxx")
LD = str(CROSS_DIR / "irix-ld")
STRIP_VERNEED = str(CROSS_DIR / "strip-verneed")

STAGING = "/opt/sgug-staging/usr/sgug"

# Common flags
CFLAGS = ["-O1", "-g", "-Wall"]
# For shared libs: PIC required
CFLAGS_SHARED = CFLAGS + ["-fPIC"]


def run(cmd, desc=""):
    """Run a command, return (success, stdout, stderr)."""
    if os.environ.get("VERBOSE"):
        print(f"  $ {' '.join(cmd)}")
    try:
        r = subprocess.run(cmd, capture_output=True, text=True, timeout=60)
        if r.returncode != 0:
            return False, r.stdout, r.stderr
        return True, r.stdout, r.stderr
    except subprocess.TimeoutExpired:
        return False, "", "TIMEOUT"
    except FileNotFoundError as e:
        return False, "", str(e)


def compile_obj(src, obj, shared=False):
    """Compile a .c file to .o"""
    flags = CFLAGS_SHARED if shared else CFLAGS
    cmd = [CC] + flags + ["-c", str(src), "-o", str(obj)]
    ok, out, err = run(cmd, f"compile {src.name}")
    if not ok:
        print(f"  FAIL compile {src.name}: {err.strip()}")
    return ok


def link_shared(objs, output, deps=None, extra_link=None):
    """Link object files into a shared library."""
    cmd = [CC, "-shared", "-fPIC"]
    cmd += [str(o) for o in objs]
    cmd += ["-o", str(output)]
    if deps:
        cmd += [f"-L{LIB_DIR}"]
        for d in deps:
            # strip lib prefix and .so suffix for -l flag
            libname = d
            if libname.startswith("lib"):
                libname = libname[3:]
            if libname.endswith(".so"):
                libname = libname[:-3]
            cmd += [f"-l{libname}"]
    if extra_link:
        cmd += extra_link
    ok, out, err = run(cmd, f"link {output.name}")
    if not ok:
        print(f"  FAIL link {output.name}: {err.strip()}")
        return False
    # Strip VERNEED tags
    run([STRIP_VERNEED, str(output)])
    return True


def link_exe(objs, output, libs=None, extra_link=None):
    """Link object files into an executable."""
    cmd = [CC]
    cmd += [str(o) for o in objs]
    cmd += ["-o", str(output)]
    cmd += [f"-L{LIB_DIR}"]
    if libs:
        for lib in libs:
            libname = lib["name"]
            if libname.startswith("lib"):
                libname = libname[3:]
            if libname.endswith(".so"):
                libname = libname[:-3]
            cmd += [f"-l{libname}"]
    if extra_link:
        cmd += extra_link
    cmd += [f"-Wl,-rpath,{STAGING}/lib32"]
    ok, out, err = run(cmd, f"link {output.name}")
    if not ok:
        print(f"  FAIL link {output.name}: {err.strip()}")
    return ok


def build_test(test):
    """Build a single test. Returns (test_id, success, error_msg)."""
    tid = test["id"]
    name = test["name"]
    print(f"Building {tid} ({name})...")

    # Build shared libraries first (in order, so deps resolve)
    for lib in test.get("libs", []):
        lib_objs = []
        for src_name in lib["sources"]:
            src = SRC_DIR / src_name
            if not src.exists():
                return tid, False, f"missing source: {src_name}"
            obj = OBJ_DIR / (src.stem + ".o")
            if not compile_obj(src, obj, shared=True):
                return tid, False, f"compile failed: {src_name}"
            lib_objs.append(obj)

        lib_out = LIB_DIR / lib["name"]
        deps = lib.get("deps", [])
        extra = lib.get("link", [])
        if not link_shared(lib_objs, lib_out, deps=deps, extra_link=extra):
            return tid, False, f"link failed: {lib['name']}"

    # Build main executable
    main_objs = []
    for src_name in test["sources"]:
        src = SRC_DIR / src_name
        if not src.exists():
            return tid, False, f"missing source: {src_name}"
        obj = OBJ_DIR / (src.stem + ".o")
        if not compile_obj(src, obj, shared=False):
            return tid, False, f"compile failed: {src_name}"
        main_objs.append(obj)

    exe_out = BIN_DIR / f"test_{name}"
    extra_link = test.get("link", [])
    if not link_exe(main_objs, exe_out, libs=test.get("libs", []), extra_link=extra_link):
        return tid, False, "link failed: executable"

    return tid, True, ""


def main():
    import argparse
    parser = argparse.ArgumentParser(description="Build IRIX rld test suite")
    parser.add_argument("tests", nargs="*", help="Specific test IDs to build")
    parser.add_argument("--priority", type=int, help="Build tests up to this priority")
    parser.add_argument("--json", action="store_true", help="Output results as JSON")
    args = parser.parse_args()

    # Import manifest
    sys.path.insert(0, str(SCRIPT_DIR))
    from test_manifest import TESTS, get_test

    # Create output dirs
    for d in [BIN_DIR, LIB_DIR, OBJ_DIR]:
        d.mkdir(parents=True, exist_ok=True)

    # Select tests
    if args.tests:
        tests = [get_test(tid) for tid in args.tests]
        tests = [t for t in tests if t is not None]
    elif args.priority:
        tests = [t for t in TESTS if t["priority"] <= args.priority]
    else:
        tests = TESTS

    results = {}
    for test in tests:
        tid, ok, err = build_test(test)
        results[tid] = {"success": ok, "error": err}
        status = "OK" if ok else f"FAIL: {err}"
        print(f"  {tid}: {status}")

    # Summary
    passed = sum(1 for r in results.values() if r["success"])
    total = len(results)
    print(f"\nBuild: {passed}/{total} passed")

    if args.json:
        print(json.dumps(results, indent=2))

    # Write results to file
    results_file = OUT_DIR / "build_results.json"
    with open(results_file, "w") as f:
        json.dump(results, f, indent=2)

    return 0 if passed == total else 1


if __name__ == "__main__":
    sys.exit(main())
