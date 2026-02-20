#!/usr/bin/env python3
"""
Mogrix Test MCP Server — Structured test harness for IRIX bundles.

Separate from the irix MCP server. Provides test-specific tools
that deploy bundles, run tests, check deps, trace syscalls,
capture screenshots, and store results.

Configuration via environment variables:
  IRIX_HOST         - IRIX hostname/IP (default: 192.168.0.81)
  IRIX_USER         - SSH user (default: root)
  IRIX_CHROOT       - Chroot path on IRIX (default: /opt/chroot)
  MOGRIX_TEST_LOG   - Log file path (default: /tmp/mogrix-test-mcp.log)
"""
import json
import os
import re
import signal
import subprocess
import sys
import tempfile
import time
import traceback
from datetime import datetime
from pathlib import Path
from typing import Any

# --- Project imports ---
# Add project root to path so we can import mogrix modules
PROJECT_ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(PROJECT_ROOT))
RULES_DIR = PROJECT_ROOT / "rules"
RESULTS_DIR = PROJECT_ROOT / "test-results"

# --- Configuration ---

IRIX_HOST = os.environ.get("IRIX_HOST", "192.168.0.81")
IRIX_USER = os.environ.get("IRIX_USER", "root")
IRIX_CHROOT = os.environ.get("IRIX_CHROOT", "/opt/chroot")
MOGRIX_TEST_LOG = os.environ.get("MOGRIX_TEST_LOG", "/tmp/mogrix-test-mcp.log")

# IRIX system libraries — always present, never need bundling
IRIX_SYSTEM_LIBS = {
    "libc.so.1", "libm.so", "libpthread.so", "libdl.so",
    "libnsl.so", "libgen.so", "librld.so", "libmalloc.so",
    "libcurses.so", "libtermcap.so", "libw.so",
    "libX11.so.1", "libXext.so", "libXt.so", "libXm.so",
    "libGL.so", "libGLcore.so.1", "libGLU.so",
}

# --- Logging ---

_log_file = None


def log(msg: str) -> None:
    """Log to file only. NEVER write to stderr."""
    global _log_file
    try:
        if _log_file is None:
            _log_file = open(MOGRIX_TEST_LOG, "a")
        ts = time.strftime("%Y-%m-%d %H:%M:%S")
        _log_file.write(f"[{ts}] {msg}\n")
        _log_file.flush()
    except Exception:
        pass


# --- JSON-RPC over stdio ---


def read_message() -> dict | None:
    line = sys.stdin.readline()
    if not line:
        return None
    return json.loads(line)


def write_message(msg: dict) -> None:
    try:
        json.dump(msg, sys.stdout, ensure_ascii=False)
        sys.stdout.write("\n")
        sys.stdout.flush()
    except (BrokenPipeError, IOError) as e:
        log(f"stdout broken: {e} — client gone, exiting")
        sys.exit(0)


# --- SSH Connection (adapted from irix-mcp-server.py) ---


class SSHConnection:
    """Persistent SSH connection via ControlMaster."""

    def __init__(self, host: str, user: str, chroot: str):
        self.host = host
        self.user = user
        self.chroot = chroot
        self.socket_path = f"/tmp/mogrix-test-ssh-{os.getpid()}"
        self.available = False

    def connect(self, timeout: int = 10) -> tuple[bool, str]:
        if os.path.exists(self.socket_path):
            self._send_control("exit")

        cmd = [
            "ssh", "-MNf",
            "-o", f"ControlPath={self.socket_path}",
            "-o", f"ConnectTimeout={timeout}",
            "-o", "ServerAliveInterval=15",
            "-o", "ServerAliveCountMax=4",
            "-o", "BatchMode=yes",
            "-o", "StrictHostKeyChecking=no",
            f"{self.user}@{self.host}",
        ]

        log(f"SSH connecting to {self.user}@{self.host}...")
        try:
            result = subprocess.run(
                cmd, capture_output=True, text=True, timeout=timeout + 5,
            )
            if result.returncode == 0:
                self.available = True
                log("SSH ControlMaster established")
                return True, "Connected"
            msg = result.stderr.strip() or f"exit code {result.returncode}"
            log(f"SSH connect failed: {msg}")
            self.available = False
            return False, msg
        except subprocess.TimeoutExpired:
            self.available = False
            return False, f"Connection timed out after {timeout}s"
        except Exception as e:
            self.available = False
            return False, str(e)

    def exec_chroot(self, command: str, timeout: int = 60) -> tuple[int, str, str]:
        """Execute command inside chroot via sgug-exec."""
        rc, stdout, stderr = self._exec_once(command, timeout, chroot=True)
        if rc == 255 and not self.is_alive():
            log("SSH connection lost, reconnecting...")
            ok, _ = self.reconnect()
            if ok:
                rc, stdout, stderr = self._exec_once(command, timeout, chroot=True)
        return rc, stdout, stderr

    def exec_host(self, command: str, timeout: int = 60) -> tuple[int, str, str]:
        """Execute command on IRIX host (outside chroot)."""
        rc, stdout, stderr = self._exec_once(command, timeout, chroot=False)
        if rc == 255 and not self.is_alive():
            log("SSH connection lost, reconnecting...")
            ok, _ = self.reconnect()
            if ok:
                rc, stdout, stderr = self._exec_once(command, timeout, chroot=False)
        return rc, stdout, stderr

    def _exec_once(self, command: str, timeout: int, chroot: bool) -> tuple[int, str, str]:
        escaped_cmd = command.replace("'", "'\\''")
        if chroot:
            remote_cmd = (
                f"chroot {self.chroot} /usr/sgug/bin/sgug-exec "
                f"/bin/sh -c '{escaped_cmd}'"
            )
        else:
            remote_cmd = f"/bin/sh -c '{escaped_cmd}'"

        ssh_cmd = [
            "ssh",
            "-o", f"ControlPath={self.socket_path}",
            "-o", "BatchMode=yes",
            f"{self.user}@{self.host}",
            remote_cmd,
        ]

        log(f"{'chroot' if chroot else 'host'}-exec: {command}")
        try:
            result = subprocess.run(
                ssh_cmd, capture_output=True, text=True,
                timeout=timeout, errors="replace",
            )
            return result.returncode, result.stdout, result.stderr
        except subprocess.TimeoutExpired:
            return -1, "", f"Command timed out after {timeout}s"
        except Exception as e:
            return -1, "", f"SSH error: {e}"

    def scp_to(self, local_path: str, remote_path: str) -> tuple[bool, str]:
        """Copy file TO IRIX host."""
        scp_cmd = [
            "scp",
            "-o", f"ControlPath={self.socket_path}",
            "-o", "BatchMode=yes",
            local_path,
            f"{self.user}@{self.host}:{remote_path}",
        ]
        log(f"scp-to: {local_path} -> {self.host}:{remote_path}")
        try:
            result = subprocess.run(
                scp_cmd, capture_output=True, text=True,
                timeout=120, errors="replace",
            )
            if result.returncode == 0:
                return True, "Copied"
            return False, f"scp failed: {result.stderr.strip()}"
        except subprocess.TimeoutExpired:
            return False, "scp timed out after 120s"
        except Exception as e:
            return False, str(e)

    def scp_from(self, remote_path: str, local_path: str) -> tuple[bool, str]:
        """Copy file FROM IRIX host to Linux."""
        scp_cmd = [
            "scp",
            "-o", f"ControlPath={self.socket_path}",
            "-o", "BatchMode=yes",
            f"{self.user}@{self.host}:{remote_path}",
            local_path,
        ]
        log(f"scp-from: {self.host}:{remote_path} -> {local_path}")
        try:
            result = subprocess.run(
                scp_cmd, capture_output=True, text=True,
                timeout=120, errors="replace",
            )
            if result.returncode == 0:
                return True, "Copied"
            return False, f"scp failed: {result.stderr.strip()}"
        except subprocess.TimeoutExpired:
            return False, "scp timed out after 120s"
        except Exception as e:
            return False, str(e)

    def deploy_dir(self, local_dir: Path, remote_dir: str) -> tuple[bool, str]:
        """Deploy a directory to IRIX host via tar pipe."""
        # Create remote dir
        rc, _, stderr = self.exec_host(f"/sbin/mkdir -p {remote_dir}")
        if rc != 0:
            return False, f"mkdir failed: {stderr}"

        full_cmd = (
            f"tar cf - -C {local_dir.parent} {local_dir.name} | "
            f"ssh -o ControlPath={self.socket_path} -o BatchMode=yes "
            f"{self.user}@{self.host} "
            f"'cd {remote_dir} && /sbin/tar xf -'"
        )
        log(f"deploy: {local_dir} -> {self.host}:{remote_dir}/")
        try:
            result = subprocess.run(
                full_cmd, shell=True, capture_output=True, text=True,
                timeout=120, errors="replace",
            )
            if result.returncode == 0:
                return True, "Deployed"
            return False, result.stderr.strip() or f"exit {result.returncode}"
        except subprocess.TimeoutExpired:
            return False, "Deploy timed out after 120s"
        except Exception as e:
            return False, str(e)

    def is_alive(self) -> bool:
        try:
            result = subprocess.run(
                ["ssh", "-O", "check",
                 "-o", f"ControlPath={self.socket_path}",
                 f"{self.user}@{self.host}"],
                capture_output=True, text=True, timeout=5,
            )
            return result.returncode == 0
        except Exception:
            return False

    def reconnect(self) -> tuple[bool, str]:
        self.disconnect()
        return self.connect()

    def disconnect(self) -> None:
        self._send_control("exit")

    def _send_control(self, cmd: str) -> None:
        try:
            subprocess.run(
                ["ssh", "-O", cmd,
                 "-o", f"ControlPath={self.socket_path}",
                 f"{self.user}@{self.host}"],
                capture_output=True, timeout=5,
            )
        except Exception:
            pass
        if cmd == "exit":
            try:
                os.unlink(self.socket_path)
            except OSError:
                pass


# --- Par Output Parser ---


class ParParser:
    """Parse IRIX par output into structured summary."""

    def parse(self, raw_output: str) -> dict:
        lib_loads: list[dict] = []
        file_errors: list[dict] = []
        signals: list[dict] = []
        crash_info = ""
        rld_errors: list[str] = []

        # Track open fds for .so files
        open_fds: dict[int, str] = {}  # fd -> path

        lines = raw_output.splitlines()
        last_lines = lines[-20:] if len(lines) > 20 else lines

        for line in lines:
            # Library open success: open("...libfoo.so...", O_RDONLY...) = N
            m = re.search(
                r'open\("([^"]*\.so[^"]*)",\s*O_RDONLY[^)]*\)\s*=\s*(\d+)',
                line,
            )
            if m:
                path, fd = m.group(1), int(m.group(2))
                open_fds[fd] = path
                soname = os.path.basename(path)
                lib_loads.append({
                    "soname": soname, "path": path, "status": "loaded",
                })
                continue

            # Library open failure: open("...libfoo.so...", ...) errno = N
            m = re.search(
                r'open\("([^"]*\.so[^"]*)",[^)]*\)\s*errno\s*=\s*(\d+)\s*\(([^)]*)\)',
                line,
            )
            if m:
                path, errno_num, errno_desc = m.group(1), m.group(2), m.group(3)
                soname = os.path.basename(path)
                # Only record if this soname hasn't already been loaded
                if not any(
                    l["soname"] == soname and l["status"] == "loaded"
                    for l in lib_loads
                ):
                    lib_loads.append({
                        "soname": soname, "path": path, "status": "failed",
                        "errno": f"{errno_num} ({errno_desc})",
                    })
                continue

            # General file open failure (non-.so files)
            m = re.search(
                r'open\("([^"]*)",[^)]*\)\s*errno\s*=\s*(\d+)\s*\(([^)]*)\)',
                line,
            )
            if m:
                path, errno_num, errno_desc = m.group(1), m.group(2), m.group(3)
                if not path.endswith(".so") and ".so." not in path:
                    file_errors.append({
                        "path": path,
                        "errno": f"{errno_num} ({errno_desc})",
                    })
                continue

            # Signals
            m = re.search(r'received signal (\w+)', line)
            if m:
                signals.append({"signal": m.group(1), "context": line.strip()})
                continue

            m = re.search(r'was sent signal (\w+)', line)
            if m:
                signals.append({"signal": m.group(1), "context": line.strip()})
                continue

            # rld errors
            if "rld:" in line and ("Fatal" in line or "Error" in line):
                rld_errors.append(line.strip())

        # Detect crash from signals
        crash_signals = {"SIGSEGV", "SIGBUS", "SIGABRT", "SIGFPE", "SIGILL"}
        for sig in signals:
            if sig["signal"] in crash_signals:
                crash_info = f"Crashed with {sig['signal']}\n" + "\n".join(
                    l.strip() for l in last_lines
                )
                break

        # Deduplicate lib_loads: keep only final status per soname
        seen = {}
        for load in lib_loads:
            seen[load["soname"]] = load
        lib_loads = list(seen.values())

        # Filter out noise from file_errors (very common /dev/fd etc.)
        file_errors = [
            e for e in file_errors
            if not e["path"].startswith("/dev/fd")
            and not e["path"].startswith("/proc")
            and "/locale/" not in e["path"]
        ]

        return {
            "library_loads": lib_loads,
            "file_errors": file_errors[:20],  # cap noise
            "signals": signals,
            "crash_info": crash_info,
            "rld_errors": rld_errors,
        }


# --- .run File Extraction ---


def extract_run_file(run_path: Path) -> tuple[Path, str, Path]:
    """Extract a .run bundle on Linux.

    Returns (bundle_dir, bundle_name, temp_dir).
    Caller must clean up temp_dir.
    """
    skip = None
    bundle_name = None

    with open(run_path, "rb") as f:
        for _ in range(30):
            raw_line = f.readline()
            if not raw_line:
                break
            text = raw_line.decode("ascii", errors="ignore").strip()
            if text.startswith("SKIP="):
                skip = int(text.split("=")[1])
            if text.startswith('BUNDLE="'):
                bundle_name = text.split('"')[1]

    if skip is None:
        raise ValueError(f"No SKIP= line found in {run_path}")
    if bundle_name is None:
        raise ValueError(f"No BUNDLE= line found in {run_path}")

    tmp_dir = Path(tempfile.mkdtemp(prefix="mogrix-test-"))

    # Extract: tail skips the shell header, gunzip decompresses, tar extracts
    cmd = f"tail -n +{skip} '{run_path}' | gunzip | tar xf - -C '{tmp_dir}'"
    result = subprocess.run(cmd, shell=True, capture_output=True, text=True, timeout=60)
    if result.returncode != 0:
        raise RuntimeError(f"Extraction failed: {result.stderr}")

    bundle_dir = tmp_dir / bundle_name
    if not bundle_dir.is_dir():
        raise RuntimeError(f"Expected bundle dir {bundle_dir} not found after extraction")

    return bundle_dir, bundle_name, tmp_dir


# --- Result Storage ---


def save_results(
    package: str,
    bundle_name: str,
    results: list[dict],
    summary: dict,
) -> Path:
    """Save test results to test-results/<package>.json."""
    RESULTS_DIR.mkdir(parents=True, exist_ok=True)
    out_path = RESULTS_DIR / f"{package}.json"
    data = {
        "package": package,
        "bundle_name": bundle_name,
        "timestamp": datetime.now().isoformat(timespec="seconds"),
        "summary": summary,
        "tests": results,
    }
    with open(out_path, "w") as f:
        json.dump(data, f, indent=2)
    return out_path


def load_results(package: str) -> dict | None:
    """Load stored results for a package."""
    path = RESULTS_DIR / f"{package}.json"
    if not path.exists():
        return None
    with open(path) as f:
        return json.load(f)


def list_all_results() -> list[dict]:
    """List summary of all stored test results."""
    if not RESULTS_DIR.is_dir():
        return []
    summaries = []
    for f in sorted(RESULTS_DIR.glob("*.json")):
        try:
            with open(f) as fh:
                data = json.load(fh)
            summaries.append({
                "package": data.get("package", f.stem),
                "timestamp": data.get("timestamp", "?"),
                "summary": data.get("summary", {}),
            })
        except Exception:
            pass
    return summaries


# --- Dep Checking ---


def check_deps_linux(path: Path) -> list[dict]:
    """Check library deps of MIPS ELF binaries using readelf on Linux."""
    staging_lib = Path("/opt/sgug-staging/usr/sgug/lib32")
    results = []

    # Find ELF binaries
    binaries = []
    if path.is_file():
        binaries = [path]
    else:
        # Bundle directory — scan _bin/ and _sbin/
        for subdir in ["_bin", "_sbin"]:
            d = path / subdir
            if d.is_dir():
                binaries.extend(sorted(f for f in d.iterdir() if f.is_file()))
        # Also check root-level wrapper scripts? No — those are shell scripts.

    lib32_dir = path / "_lib32" if path.is_dir() else None

    for binary in binaries:
        try:
            result = subprocess.run(
                ["readelf", "-d", str(binary)],
                capture_output=True, text=True, timeout=10,
            )
        except Exception:
            results.append({
                "binary": binary.name,
                "error": "readelf failed",
                "deps": [],
            })
            continue

        deps = []
        for line in result.stdout.splitlines():
            m = re.search(r"\(NEEDED\)\s+Shared library:\s+\[([^\]]+)\]", line)
            if m:
                soname = m.group(1)
                status = "UNKNOWN"

                # Check bundle _lib32/
                if lib32_dir and (lib32_dir / soname).exists():
                    status = "bundle"
                elif soname in IRIX_SYSTEM_LIBS:
                    status = "system"
                elif staging_lib.is_dir() and (staging_lib / soname).exists():
                    status = "staging"
                else:
                    status = "MISSING"

                deps.append({"soname": soname, "status": status})

        results.append({"binary": binary.name, "deps": deps})

    return results


# --- MCP Server ---


class MogrixTestServer:
    """MCP server providing test harness tools for IRIX bundles."""

    def __init__(self):
        self.name = "mogrix-test"
        self.version = "1.0.0"
        self.protocol_version = "2025-11-25"
        self.fallback_version = "2024-11-05"
        self.initialized = False
        self.ssh = SSHConnection(IRIX_HOST, IRIX_USER, IRIX_CHROOT)
        self.par_parser = ParParser()

        self.tools = [
            {
                "name": "test_bundle",
                "description": (
                    "Deploy a .run bundle to IRIX, run all discovered tests "
                    "(auto --version + YAML smoke_tests), return structured "
                    "pass/fail report. Results saved to test-results/<pkg>.json. "
                    "Example: test_bundle {\"bundle_path\": \"/path/to/nano.run\"}"
                ),
                "inputSchema": {
                    "type": "object",
                    "properties": {
                        "bundle_path": {
                            "type": "string",
                            "description": (
                                "Absolute path on Linux to .run file or "
                                "extracted bundle directory"
                            ),
                        },
                        "keep": {
                            "type": "boolean",
                            "description": (
                                "Don't clean up test bundle on IRIX after "
                                "testing (default: false)"
                            ),
                        },
                    },
                    "required": ["bundle_path"],
                },
            },
            {
                "name": "test_binary",
                "description": (
                    "Run a single binary on IRIX with optional args. "
                    "Returns exit code, stdout, stderr, crash/signal info. "
                    "Runs in chroot by default. For bundle binaries, set "
                    "host_mode=true. "
                    "Example: test_binary {\"binary\": \"/usr/sgug/bin/grep\", "
                    "\"args\": \"--version\"}"
                ),
                "inputSchema": {
                    "type": "object",
                    "properties": {
                        "binary": {
                            "type": "string",
                            "description": "Path to binary on IRIX",
                        },
                        "args": {
                            "type": "string",
                            "description": "Arguments (default: --version)",
                        },
                        "timeout": {
                            "type": "integer",
                            "description": "Timeout in seconds (default: 10)",
                        },
                        "env": {
                            "type": "object",
                            "description": (
                                "Extra env vars, e.g. {\"DISPLAY\": \":0\"}"
                            ),
                        },
                        "host_mode": {
                            "type": "boolean",
                            "description": (
                                "Run on host (not chroot). Use for bundle "
                                "binaries. Default: false"
                            ),
                        },
                    },
                    "required": ["binary"],
                },
            },
            {
                "name": "check_deps",
                "description": (
                    "Check library dependencies of a binary or bundle "
                    "WITHOUT running it. Reports each NEEDED soname and "
                    "whether it resolves (bundle, staging, system, MISSING). "
                    "Example: check_deps {\"path\": \"/path/to/bundle_dir\"}"
                ),
                "inputSchema": {
                    "type": "object",
                    "properties": {
                        "path": {
                            "type": "string",
                            "description": (
                                "Path to bundle dir or binary on Linux, "
                                "or binary path in IRIX chroot"
                            ),
                        },
                        "on_irix": {
                            "type": "boolean",
                            "description": (
                                "Check on IRIX via elfdump (default: false, "
                                "checks on Linux via readelf)"
                            ),
                        },
                    },
                    "required": ["path"],
                },
            },
            {
                "name": "par_trace",
                "description": (
                    "Run command under IRIX par (syscall tracer) with "
                    "intelligent output parsing. Returns summary of library "
                    "loads, file errors, signals, and crash info. "
                    "Example: par_trace {\"command\": "
                    "\"/usr/sgug/bin/grep --version\"}"
                ),
                "inputSchema": {
                    "type": "object",
                    "properties": {
                        "command": {
                            "type": "string",
                            "description": "Command to trace on IRIX",
                        },
                        "timeout": {
                            "type": "integer",
                            "description": "Timeout in seconds (default: 30)",
                        },
                        "raw": {
                            "type": "boolean",
                            "description": (
                                "Include raw par output (default: false)"
                            ),
                        },
                        "host_mode": {
                            "type": "boolean",
                            "description": (
                                "Run on host not chroot (default: false)"
                            ),
                        },
                        "env": {
                            "type": "object",
                            "description": (
                                "Extra env vars, e.g. {\"DISPLAY\": \":0\"}"
                            ),
                        },
                    },
                    "required": ["command"],
                },
            },
            {
                "name": "test_report",
                "description": (
                    "Return stored test results. If package given, return "
                    "full results. If omitted, list all packages with their "
                    "last test date and pass/fail status. "
                    "Example: test_report {} or "
                    "test_report {\"package\": \"nano\"}"
                ),
                "inputSchema": {
                    "type": "object",
                    "properties": {
                        "package": {
                            "type": "string",
                            "description": (
                                "Package name. Omit to list all results."
                            ),
                        },
                    },
                },
            },
            {
                "name": "screenshot",
                "description": (
                    "Capture the IRIX X11 display as a PNG image. Returns "
                    "the local path to the PNG — use Read tool to view it. "
                    "Requires xwd on IRIX (ships with X11) and ImageMagick "
                    "convert on Linux. "
                    "Example: screenshot {\"delay\": 2}"
                ),
                "inputSchema": {
                    "type": "object",
                    "properties": {
                        "display": {
                            "type": "string",
                            "description": "X display (default: :0)",
                        },
                        "delay": {
                            "type": "integer",
                            "description": (
                                "Seconds to wait before capture (default: 0)"
                            ),
                        },
                    },
                },
            },
        ]

    # --- Connectivity ---

    def _ensure_connected(self, request_id: Any) -> dict | None:
        if self.ssh.available and self.ssh.is_alive():
            return None
        ok, msg = (
            self.ssh.reconnect() if self.ssh.available else self.ssh.connect()
        )
        if ok:
            self.ssh.available = True
            return None
        return self._tool_error(
            request_id,
            f"IRIX unreachable at {IRIX_HOST}: {msg}. "
            "Check that the SGI is powered on and SSH is running.",
        )

    # --- Tool Implementations ---

    def _tool_test_bundle(self, request_id: Any, args: dict) -> dict:
        bundle_path = Path(args.get("bundle_path", ""))
        keep = args.get("keep", False)

        if not bundle_path.exists():
            return self._tool_error(request_id, f"Not found: {bundle_path}")

        err = self._ensure_connected(request_id)
        if err:
            return err

        tmp_dir = None
        try:
            # Extract .run file if needed
            if bundle_path.suffix == ".run" or str(bundle_path).endswith(".run"):
                bundle_dir, bundle_name, tmp_dir = extract_run_file(bundle_path)
            elif bundle_path.is_dir():
                bundle_dir = bundle_path
                bundle_name = bundle_path.name
            else:
                return self._tool_error(
                    request_id,
                    f"Expected .run file or directory, got: {bundle_path}",
                )

            # Discover tests
            try:
                from mogrix.test import TestDiscovery, ScriptGenerator
                discovery = TestDiscovery(RULES_DIR)
                tests = discovery.discover(bundle_dir)
            except Exception as e:
                return self._tool_error(
                    request_id, f"Test discovery failed: {e}"
                )

            if not tests:
                return self._tool_result(
                    request_id,
                    f"No tests discovered for {bundle_name}. "
                    "Add smoke_test entries to the package YAML.",
                )

            # Generate test script
            gen = ScriptGenerator()
            script = gen.generate(tests)

            # Deploy to IRIX (host mode — bundles are self-contained)
            remote_dir = "/usr/people/edodd/apps"
            ok, msg = self.ssh.deploy_dir(bundle_dir, remote_dir)
            if not ok:
                return self._tool_error(
                    request_id, f"Deploy failed: {msg}"
                )

            remote_bundle = f"{remote_dir}/{bundle_name}"

            # Run test script via SSH stdin (host mode)
            ssh_cmd = [
                "ssh",
                "-o", f"ControlPath={self.ssh.socket_path}",
                "-o", "BatchMode=yes",
                f"{self.ssh.user}@{self.ssh.host}",
                f"/bin/sh -s {remote_bundle}",
            ]

            log(f"Running tests for {bundle_name}...")
            try:
                result = subprocess.run(
                    ssh_cmd, input=script, capture_output=True,
                    text=True, timeout=300, errors="replace",
                )
                stdout = result.stdout
                stderr = result.stderr
            except subprocess.TimeoutExpired:
                stdout = ""
                stderr = "Test execution timed out after 300s"

            # Parse results
            test_results = []
            total = passed = failed = skipped = 0
            for line in stdout.splitlines():
                if not line.startswith("MOGRIX_RESULT|"):
                    continue
                parts = line.split("|", 4)
                if len(parts) < 5:
                    continue
                name, status, rc_str, message = (
                    parts[1], parts[2], parts[3], parts[4]
                )
                total += 1
                if status == "PASS":
                    passed += 1
                elif status == "FAIL":
                    failed += 1
                elif status == "SKIP":
                    skipped += 1

                test_results.append({
                    "name": name,
                    "status": status,
                    "exit_code": int(rc_str) if rc_str.lstrip("-").isdigit() else -1,
                    "message": message,
                })

            # Save results
            # Derive package name from bundle name (strip version-release suffix)
            pkg_name = bundle_name.split("-")[0]
            # Better: match against known package names
            for part_count in range(1, 5):
                candidate = "-".join(bundle_name.split("-")[:part_count])
                yaml_path = RULES_DIR / "packages" / f"{candidate}.yaml"
                if yaml_path.exists():
                    pkg_name = candidate
                    break

            summary = {
                "total": total,
                "passed": passed,
                "failed": failed,
                "skipped": skipped,
            }
            saved_path = save_results(pkg_name, bundle_name, test_results, summary)

            # Clean up on IRIX
            if not keep:
                self.ssh.exec_host(f"rm -rf {remote_bundle}")

            # Format output
            lines = [f"Test Results: {bundle_name}\n"]
            for r in test_results:
                marker = {"PASS": "+", "FAIL": "X", "SKIP": "~"}.get(
                    r["status"], "?"
                )
                lines.append(
                    f"  [{marker}] {r['name']}: {r['status']} "
                    f"(rc={r['exit_code']}) {r['message']}"
                )

            lines.append(
                f"\nSummary: {total} tests — "
                f"{passed} passed, {failed} failed, {skipped} skipped"
            )
            if failed > 0:
                lines.append("BUNDLE HAS TEST FAILURES")
            else:
                lines.append("All tests passed.")

            lines.append(f"\nResults saved to: {saved_path}")

            return self._tool_result(
                request_id, "\n".join(lines), is_error=(failed > 0)
            )

        except Exception as e:
            log(f"test_bundle error: {e}\n{traceback.format_exc()}")
            return self._tool_error(request_id, f"test_bundle failed: {e}")
        finally:
            if tmp_dir and tmp_dir.exists():
                import shutil
                shutil.rmtree(tmp_dir, ignore_errors=True)

    def _tool_test_binary(self, request_id: Any, args: dict) -> dict:
        binary = args.get("binary", "")
        cmd_args = args.get("args", "--version")
        timeout = args.get("timeout", 10)
        env_vars = args.get("env", {})
        host_mode = args.get("host_mode", False)

        if not binary:
            return self._tool_error(request_id, "binary is required")

        err = self._ensure_connected(request_id)
        if err:
            return err

        # Build command with env vars (double-quote values for sh safety)
        env_prefix = ""
        if env_vars:
            parts = []
            for k, v in env_vars.items():
                # Escape any double quotes or backslashes in the value
                escaped_v = str(v).replace("\\", "\\\\").replace('"', '\\"')
                parts.append(f'{k}="{escaped_v}"; export {k}')
            env_prefix = "; ".join(parts) + "; "

        command = f"{env_prefix}{binary} {cmd_args}"

        if host_mode:
            rc, stdout, stderr = self.ssh.exec_host(command, timeout)
        else:
            rc, stdout, stderr = self.ssh.exec_chroot(command, timeout)

        # Analyze results
        result_data = {
            "exit_code": rc,
            "stdout": stdout.strip() if stdout else "",
            "stderr": stderr.strip() if stderr else "",
            "crashed": False,
            "signal": None,
            "rld_error": None,
        }

        # Check for signal (exit code > 128)
        if rc > 128:
            sig = rc - 128
            sig_names = {
                6: "SIGABRT", 10: "SIGBUS", 11: "SIGSEGV",
                9: "SIGKILL", 15: "SIGTERM", 8: "SIGFPE",
                4: "SIGILL", 13: "SIGPIPE",
            }
            result_data["signal"] = sig_names.get(sig, f"signal {sig}")
            if sig not in (9, 15, 13):  # KILL/TERM/PIPE aren't crashes
                result_data["crashed"] = True

        # Check for rld errors
        combined = f"{stdout} {stderr}"
        if "rld:" in combined:
            for line in combined.splitlines():
                if "rld:" in line:
                    result_data["rld_error"] = line.strip()
                    result_data["crashed"] = True
                    break

        # Format output
        lines = [f"binary: {binary} {cmd_args}"]
        lines.append(f"exit_code: {rc}")
        if result_data["signal"]:
            lines.append(f"signal: {result_data['signal']}")
        if result_data["crashed"]:
            lines.append("CRASHED: yes")
        if result_data["rld_error"]:
            lines.append(f"rld_error: {result_data['rld_error']}")
        if stdout.strip():
            lines.append(f"stdout:\n{stdout.strip()}")
        if stderr.strip():
            lines.append(f"stderr:\n{stderr.strip()}")

        return self._tool_result(
            request_id,
            "\n".join(lines),
            is_error=result_data["crashed"],
        )

    def _tool_check_deps(self, request_id: Any, args: dict) -> dict:
        path_str = args.get("path", "")
        on_irix = args.get("on_irix", False)

        if not path_str:
            return self._tool_error(request_id, "path is required")

        if not on_irix:
            path = Path(path_str)
            if not path.exists():
                return self._tool_error(request_id, f"Not found: {path}")

            # Handle .run files by extracting first
            tmp_dir = None
            if path.suffix == ".run" or str(path).endswith(".run"):
                try:
                    path, _, tmp_dir = extract_run_file(path)
                except Exception as e:
                    return self._tool_error(
                        request_id, f"Failed to extract .run: {e}"
                    )

            try:
                results = check_deps_linux(path)

                lines = [f"Dependencies for: {path_str} (Linux readelf)\n"]
                any_missing = False
                for r in results:
                    if "error" in r:
                        lines.append(f"  {r['binary']}: {r['error']}")
                        continue
                    lines.append(f"  {r['binary']}:")
                    for d in r["deps"]:
                        marker = "X" if d["status"] == "MISSING" else "+"
                        lines.append(
                            f"    [{marker}] {d['soname']}: {d['status']}"
                        )
                        if d["status"] == "MISSING":
                            any_missing = True

                if any_missing:
                    lines.append("\nWARNING: Some libraries are MISSING")
                else:
                    lines.append("\nAll dependencies resolved.")

                return self._tool_result(
                    request_id, "\n".join(lines), is_error=any_missing
                )
            finally:
                if tmp_dir and Path(tmp_dir).exists():
                    import shutil
                    shutil.rmtree(tmp_dir, ignore_errors=True)

        else:
            # IRIX mode
            err = self._ensure_connected(request_id)
            if err:
                return err

            # Use elfdump on IRIX
            rc, stdout, stderr = self.ssh.exec_chroot(
                f"elfdump -Dl '{path_str}' 2>&1", timeout=15
            )

            deps = []
            for line in stdout.splitlines():
                m = re.search(r"NEEDED\s+(\S+)", line)
                if m:
                    soname = m.group(1)
                    # Check if library exists
                    check_rc, _, _ = self.ssh.exec_chroot(
                        f"ls /usr/sgug/lib32/{soname} /usr/lib32/{soname} "
                        f"2>/dev/null | head -1",
                        timeout=5,
                    )
                    status = "found" if check_rc == 0 else "MISSING"
                    if soname in IRIX_SYSTEM_LIBS:
                        status = "system"
                    deps.append({"soname": soname, "status": status})

            lines = [f"Dependencies for: {path_str} (IRIX elfdump)\n"]
            for d in deps:
                marker = "+" if d["status"] != "MISSING" else "X"
                lines.append(f"  [{marker}] {d['soname']}: {d['status']}")

            missing = [d for d in deps if d["status"] == "MISSING"]
            if missing:
                lines.append(
                    f"\nMISSING: {len(missing)} libraries unresolved"
                )
            else:
                lines.append(f"\nAll {len(deps)} dependencies resolved.")

            return self._tool_result(
                request_id, "\n".join(lines), is_error=bool(missing)
            )

    def _tool_par_trace(self, request_id: Any, args: dict) -> dict:
        command = args.get("command", "")
        timeout = args.get("timeout", 30)
        raw = args.get("raw", False)
        host_mode = args.get("host_mode", False)
        env_vars = args.get("env", {})

        if not command:
            return self._tool_error(request_id, "command is required")

        err = self._ensure_connected(request_id)
        if err:
            return err

        # Build env prefix if env vars provided
        env_prefix = ""
        if env_vars:
            parts = []
            for k, v in env_vars.items():
                escaped_v = str(v).replace("\\", "\\\\").replace('"', '\\"')
                parts.append(f'{k}="{escaped_v}"; export {k}')
            env_prefix = "; ".join(parts) + "; "

        par_cmd = f"{env_prefix}par -s -SS -l -i {command} 2>&1; true"

        if host_mode:
            rc, stdout, stderr = self.ssh.exec_host(par_cmd, timeout)
        else:
            rc, stdout, stderr = self.ssh.exec_chroot(par_cmd, timeout)

        raw_output = stdout + (f"\n{stderr}" if stderr else "")

        # Parse
        parsed = self.par_parser.parse(raw_output)

        # Format summary
        lines = [f"par trace: {command}\n"]

        if parsed["rld_errors"]:
            lines.append("RLD ERRORS:")
            for e in parsed["rld_errors"]:
                lines.append(f"  {e}")
            lines.append("")

        loaded = [
            l for l in parsed["library_loads"] if l["status"] == "loaded"
        ]
        failed_libs = [
            l for l in parsed["library_loads"] if l["status"] == "failed"
        ]

        if failed_libs:
            lines.append(f"FAILED library loads ({len(failed_libs)}):")
            for l in failed_libs:
                lines.append(f"  {l['soname']}: {l.get('errno', '?')}")
            lines.append("")

        lines.append(f"Libraries loaded: {len(loaded)}")
        for l in loaded:
            lines.append(f"  {l['soname']} ({l['path']})")
        lines.append("")

        if parsed["signals"]:
            lines.append("Signals:")
            for s in parsed["signals"]:
                lines.append(f"  {s['signal']}: {s['context']}")
            lines.append("")

        if parsed["crash_info"]:
            lines.append(f"CRASH:\n{parsed['crash_info']}")
            lines.append("")

        if parsed["file_errors"]:
            lines.append(f"File errors ({len(parsed['file_errors'])}):")
            for e in parsed["file_errors"][:10]:
                lines.append(f"  {e['path']}: {e['errno']}")

        if raw:
            lines.append(f"\n--- Raw par output ---\n{raw_output}")

        return self._tool_result(
            request_id,
            "\n".join(lines),
            is_error=bool(parsed["crash_info"] or parsed["rld_errors"]),
        )

    def _tool_test_report(self, request_id: Any, args: dict) -> dict:
        package = args.get("package")

        if package:
            data = load_results(package)
            if data is None:
                return self._tool_result(
                    request_id, f"No test results for '{package}'"
                )
            return self._tool_result(
                request_id, json.dumps(data, indent=2)
            )
        else:
            summaries = list_all_results()
            if not summaries:
                return self._tool_result(
                    request_id, "No test results stored yet."
                )

            lines = ["Stored test results:\n"]
            lines.append(
                f"{'Package':<25} {'Date':<22} "
                f"{'Pass':>5} {'Fail':>5} {'Skip':>5} {'Total':>5}"
            )
            lines.append("-" * 75)
            for s in summaries:
                sm = s.get("summary", {})
                lines.append(
                    f"{s['package']:<25} {s['timestamp']:<22} "
                    f"{sm.get('passed', '?'):>5} {sm.get('failed', '?'):>5} "
                    f"{sm.get('skipped', '?'):>5} {sm.get('total', '?'):>5}"
                )
            return self._tool_result(request_id, "\n".join(lines))

    def _tool_screenshot(self, request_id: Any, args: dict) -> dict:
        display = args.get("display", ":0")
        delay = args.get("delay", 0)

        err = self._ensure_connected(request_id)
        if err:
            return err

        if delay > 0:
            time.sleep(delay)

        ts = datetime.now().strftime("%Y%m%d-%H%M%S")
        remote_xwd = f"/tmp/mogrix-screenshot-{ts}.xwd"
        local_xwd = f"/tmp/mogrix-screenshot-{ts}.xwd"
        local_png = f"/tmp/mogrix-screenshot-{ts}.png"

        # Capture on IRIX host (not chroot)
        cmd = f"DISPLAY={display} /usr/bin/X11/xwd -root -out {remote_xwd}"
        rc, stdout, stderr = self.ssh.exec_host(cmd, timeout=15)
        if rc != 0:
            # Try alternate xwd path
            cmd = f"DISPLAY={display} xwd -root -out {remote_xwd}"
            rc, stdout, stderr = self.ssh.exec_host(cmd, timeout=15)
            if rc != 0:
                return self._tool_error(
                    request_id,
                    f"xwd capture failed (rc={rc}): {stderr.strip()}",
                )

        # Copy back to Linux
        ok, msg = self.ssh.scp_from(remote_xwd, local_xwd)
        if not ok:
            return self._tool_error(request_id, f"scp failed: {msg}")

        # Clean up remote
        self.ssh.exec_host(f"rm -f {remote_xwd}")

        # Convert xwd -> PNG
        # Try ImageMagick convert
        try:
            result = subprocess.run(
                ["convert", local_xwd, local_png],
                capture_output=True, text=True, timeout=30,
            )
            if result.returncode == 0 and Path(local_png).exists():
                os.unlink(local_xwd)
                return self._tool_result(
                    request_id,
                    f"Screenshot saved: {local_png}\n"
                    f"Use Read tool to view the image.",
                )
        except FileNotFoundError:
            pass  # convert not available

        # Try ffmpeg as fallback
        try:
            result = subprocess.run(
                ["ffmpeg", "-y", "-i", local_xwd, local_png],
                capture_output=True, text=True, timeout=30,
            )
            if result.returncode == 0 and Path(local_png).exists():
                os.unlink(local_xwd)
                return self._tool_result(
                    request_id,
                    f"Screenshot saved: {local_png}\n"
                    f"Use Read tool to view the image.",
                )
        except FileNotFoundError:
            pass

        # No converter available — return xwd path
        return self._tool_result(
            request_id,
            f"Screenshot captured as XWD (no PNG converter found): "
            f"{local_xwd}\n"
            f"Install ImageMagick (apt install imagemagick) for PNG conversion.",
        )

    # --- JSONRPC Handlers ---

    def handle_initialize(self, request_id: Any, params: dict) -> dict:
        client_version = params.get("protocolVersion", self.fallback_version)
        client_info = params.get("clientInfo", {})
        log(
            f"Initialize from {client_info.get('name', '?')} "
            f"v{client_info.get('version', '?')} "
            f"(protocol {client_version})"
        )

        if client_version in [self.protocol_version, self.fallback_version]:
            version = client_version
        else:
            version = self.protocol_version

        ok, msg = self.ssh.connect(timeout=10)
        if ok:
            log(f"SSH connection to {IRIX_HOST} ready")
        else:
            log(f"WARNING: Cannot reach IRIX at {IRIX_HOST}: {msg}")

        return {
            "jsonrpc": "2.0",
            "id": request_id,
            "result": {
                "protocolVersion": version,
                "capabilities": {
                    "tools": {"listChanged": False},
                },
                "serverInfo": {
                    "name": self.name,
                    "version": self.version,
                },
                "instructions": (
                    "Mogrix test harness for IRIX bundles. "
                    "test_bundle: full bundle testing. "
                    "test_binary: quick single binary check. "
                    "check_deps: library dependency verification. "
                    "par_trace: parsed syscall tracing. "
                    "test_report: stored test results. "
                    "screenshot: capture IRIX X11 display as PNG."
                ),
            },
        }

    def handle_tools_list(self, request_id: Any) -> dict:
        return {
            "jsonrpc": "2.0",
            "id": request_id,
            "result": {"tools": self.tools},
        }

    def handle_tools_call(self, request_id: Any, params: dict) -> dict:
        tool_name = params.get("name")
        args = params.get("arguments", {})

        dispatch = {
            "test_bundle": self._tool_test_bundle,
            "test_binary": self._tool_test_binary,
            "check_deps": self._tool_check_deps,
            "par_trace": self._tool_par_trace,
            "test_report": self._tool_test_report,
            "screenshot": self._tool_screenshot,
        }

        handler = dispatch.get(tool_name)
        if handler:
            return handler(request_id, args)
        return self._error(-32602, f"Unknown tool: {tool_name}", request_id)

    # --- Response Helpers ---

    def _tool_result(
        self, request_id: Any, text: str, is_error: bool = False
    ) -> dict:
        return {
            "jsonrpc": "2.0",
            "id": request_id,
            "result": {
                "content": [{"type": "text", "text": text}],
                "isError": is_error,
            },
        }

    def _tool_error(self, request_id: Any, message: str) -> dict:
        return self._tool_result(request_id, f"Error: {message}", is_error=True)

    def _error(self, code: int, message: str, request_id: Any) -> dict:
        return {
            "jsonrpc": "2.0",
            "id": request_id,
            "error": {"code": code, "message": message},
        }

    # --- Main Loop ---

    def handle_request(self, msg: dict) -> dict | None:
        method = msg.get("method")
        request_id = msg.get("id")
        params = msg.get("params", {})

        if method == "initialize":
            return self.handle_initialize(request_id, params)
        elif method == "notifications/initialized":
            self.initialized = True
            log("Initialized — ready")
            return None
        elif method == "notifications/cancelled":
            log(f"Client cancelled request {params.get('requestId')}")
            return None
        elif method == "tools/list":
            return self.handle_tools_list(request_id)
        elif method == "tools/call":
            return self.handle_tools_call(request_id, params)
        elif method == "ping":
            return {"jsonrpc": "2.0", "id": request_id, "result": {}}
        elif request_id is not None:
            return self._error(-32601, f"Method not found: {method}", request_id)
        return None

    def run(self) -> None:
        log(f"Mogrix Test MCP Server v{self.version}")
        log(f"Target: {IRIX_USER}@{IRIX_HOST}:{IRIX_CHROOT}")
        log(f"Rules: {RULES_DIR}")
        log(f"Results: {RESULTS_DIR}")

        try:
            while True:
                try:
                    msg = read_message()
                    if msg is None:
                        log("stdin closed, exiting")
                        break

                    request_id = msg.get("id")

                    try:
                        response = self.handle_request(msg)
                    except Exception as e:
                        log(f"Error handling request: {e}")
                        log(traceback.format_exc())
                        if request_id is not None:
                            response = self._error(
                                -32603, f"Internal error: {e}", request_id,
                            )
                        else:
                            response = None

                    if response is not None:
                        write_message(response)

                except json.JSONDecodeError as e:
                    log(f"JSON parse error: {e}")
                    write_message({
                        "jsonrpc": "2.0",
                        "id": None,
                        "error": {"code": -32700, "message": "Parse error"},
                    })
                except KeyboardInterrupt:
                    log("Interrupted")
                    break
        finally:
            log("Shutting down — closing SSH connection")
            self.ssh.disconnect()
            if _log_file:
                _log_file.close()

        log("Shutdown complete")


def _handle_sigterm(signum, frame):
    raise SystemExit(0)


def main():
    signal.signal(signal.SIGTERM, _handle_sigterm)
    server = MogrixTestServer()
    server.run()


if __name__ == "__main__":
    main()
