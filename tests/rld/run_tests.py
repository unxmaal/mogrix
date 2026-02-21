#!/usr/bin/env python3
"""
Test runner for IRIX rld test suite.

Deploys built binaries to IRIX and runs them. Designed to be called from
the mogrix project with MCP tools, or standalone via SSH.

Usage (standalone):
    python3 tests/rld/run_tests.py              # run all
    python3 tests/rld/run_tests.py A04 D21      # run specific tests
    python3 tests/rld/run_tests.py --priority 1  # priority 1 only

Agent-driven mode:
    Import and call run_single_test() or run_all_tests() from orchestrator code,
    or use the MCP tools directly following the deploy/run pattern below.
"""

import json
import os
import subprocess
import sys
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent
PROJECT_ROOT = SCRIPT_DIR.parent.parent
OUT_DIR = SCRIPT_DIR / "out"
BIN_DIR = OUT_DIR / "bin"
LIB_DIR = OUT_DIR / "lib"

# IRIX deployment target
IRIX_HOST = "192.168.0.81"
IRIX_BASE = "/usr/people/edodd/rld-tests"
IRIX_BIN = f"{IRIX_BASE}/bin"
IRIX_LIB = f"{IRIX_BASE}/lib"

# SSH/SCP commands for standalone mode
SSH = ["ssh", f"edodd@{IRIX_HOST}"]
SCP = ["scp"]

RESULTS_FILE = PROJECT_ROOT / "test-results" / "rld-suite.json"


def deploy_standalone():
    """Deploy all built files to IRIX via scp."""
    print("Deploying to IRIX...")

    # Create dirs
    subprocess.run(SSH + [f"mkdir -p {IRIX_BIN} {IRIX_LIB}"], check=True)

    # Copy binaries
    bins = list(BIN_DIR.glob("test_*"))
    if bins:
        subprocess.run(SCP + [str(b) for b in bins] + [f"edodd@{IRIX_HOST}:{IRIX_BIN}/"], check=True)

    # Copy libraries
    libs = list(LIB_DIR.glob("*.so"))
    if libs:
        subprocess.run(SCP + [str(l) for l in libs] + [f"edodd@{IRIX_HOST}:{IRIX_LIB}/"], check=True)

    print(f"  Deployed {len(bins)} binaries, {len(libs)} libraries")


def run_test_standalone(test):
    """Run a single test on IRIX via SSH. Returns (passed, output, exit_code)."""
    tid = test["id"]
    name = test["name"]
    exe = f"{IRIX_BIN}/test_{name}"
    expect = test.get("expect", "exit0")

    # Build environment
    env_str = f"LD_LIBRARYN32_PATH={IRIX_LIB}:/usr/sgug/lib32:/usr/lib32"
    extra_env = test.get("env", {})
    for k, v in extra_env.items():
        # For _RLDN32_LIST, prepend with lib path
        if k == "_RLDN32_LIST":
            parts = v.split(":")
            parts[0] = f"{IRIX_LIB}/{parts[0]}"
            v = ":".join(parts)
        env_str += f" {k}={v}"

    cmd = SSH + [f"cd {IRIX_BASE} && {env_str} {exe}"]
    try:
        r = subprocess.run(cmd, capture_output=True, text=True, timeout=30)
        exit_code = r.returncode
        output = r.stdout + r.stderr
    except subprocess.TimeoutExpired:
        return False, "TIMEOUT", -1

    # Evaluate result
    if expect == "exit0":
        passed = exit_code == 0
    elif expect == "exit1":
        passed = exit_code == 1
    elif expect == "crash":
        passed = exit_code < 0 or exit_code >= 128
    elif expect == "rld_error":
        passed = exit_code != 0 and ("rld" in output.lower() or exit_code >= 128)
    elif isinstance(expect, dict) and "exit" in expect:
        passed = exit_code == expect["exit"]
    else:
        passed = exit_code == 0

    return passed, output.strip(), exit_code


def get_deploy_manifest():
    """Return lists of files to deploy."""
    bins = sorted(BIN_DIR.glob("test_*")) if BIN_DIR.exists() else []
    libs = sorted(LIB_DIR.glob("*.so")) if LIB_DIR.exists() else []
    return bins, libs


def format_results(results):
    """Format results dict for display."""
    lines = []
    for tid, r in sorted(results.items()):
        status = "PASS" if r["passed"] else "FAIL"
        detail = ""
        if not r["passed"]:
            detail = f" (exit={r['exit_code']}"
            if r.get("output"):
                # First line of output
                first = r["output"].split("\n")[0][:60]
                detail += f", {first}"
            detail += ")"
        lines.append(f"  [{status}] {tid}: {r['name']}{detail}")
    return "\n".join(lines)


def main():
    import argparse
    parser = argparse.ArgumentParser(description="Run IRIX rld tests")
    parser.add_argument("tests", nargs="*", help="Specific test IDs")
    parser.add_argument("--priority", type=int, help="Max priority level")
    parser.add_argument("--deploy-only", action="store_true", help="Just deploy, don't run")
    args = parser.parse_args()

    sys.path.insert(0, str(SCRIPT_DIR))
    from test_manifest import TESTS, get_test

    # Deploy
    deploy_standalone()
    if args.deploy_only:
        return 0

    # Select tests
    if args.tests:
        tests = [get_test(tid) for tid in args.tests]
        tests = [t for t in tests if t is not None]
    elif args.priority:
        tests = [t for t in TESTS if t["priority"] <= args.priority]
    else:
        tests = TESTS

    # Run tests
    results = {}
    for test in tests:
        tid = test["id"]
        name = test["name"]
        print(f"Running {tid} ({name})...", end=" ", flush=True)
        passed, output, exit_code = run_test_standalone(test)
        status = "PASS" if passed else "FAIL"
        print(f"{status} (exit={exit_code})")
        if not passed and output:
            for line in output.split("\n")[:3]:
                print(f"  | {line}")

        results[tid] = {
            "name": name,
            "passed": passed,
            "exit_code": exit_code,
            "output": output,
            "expect": test["expect"],
        }

    # Summary
    passed = sum(1 for r in results.values() if r["passed"])
    total = len(results)
    print(f"\nResults: {passed}/{total} passed")

    # Save results
    RESULTS_FILE.parent.mkdir(parents=True, exist_ok=True)
    with open(RESULTS_FILE, "w") as f:
        json.dump({
            "suite": "rld",
            "total": total,
            "passed": passed,
            "failed": total - passed,
            "tests": results,
        }, f, indent=2)
    print(f"Results saved to {RESULTS_FILE}")

    return 0 if passed == total else 1


if __name__ == "__main__":
    sys.exit(main())
