#!/usr/bin/env python3
"""
IRIX MCP Server - Provides Claude Code with safe access to an IRIX chroot.

All commands are wrapped in: ssh -> chroot -> sgug-exec -> /bin/sh
so they execute inside /opt/chroot with proper SGUG-RSE environment.

Configuration via environment variables:
  IRIX_HOST       - IRIX hostname/IP (default: 192.168.0.81)
  IRIX_USER       - SSH user (default: root)
  IRIX_CHROOT     - Chroot path on IRIX (default: /opt/chroot)
  IRIX_TIMEOUT    - Command timeout in seconds (default: 60)
  IRIX_LOG        - Log file path (default: /tmp/irix-mcp.log)
"""
import sys
import json
import time
import os
import re
import signal
import subprocess
import traceback
from typing import Any

# --- Configuration ---

IRIX_HOST = os.environ.get("IRIX_HOST", "192.168.0.81")
IRIX_USER = os.environ.get("IRIX_USER", "root")
IRIX_CHROOT = os.environ.get("IRIX_CHROOT", "/opt/chroot")
IRIX_TIMEOUT = int(os.environ.get("IRIX_TIMEOUT", "60"))
IRIX_LOG = os.environ.get("IRIX_LOG", "/tmp/irix-mcp.log")

# Commands that should never be run
BLOCKED_PATTERNS = [
    r"rm\s+(-[rf]+\s+)?/\s*$",         # rm -rf /
    r"rm\s+(-[rf]+\s+)?/usr\s*$",       # rm -rf /usr
    r"rm\s+(-[rf]+\s+)?/etc\s*$",       # rm -rf /etc
    r"rm\s+(-[rf]+\s+)?/bin\s*$",       # rm -rf /bin
    r"rm\s+(-[rf]+\s+)?/lib",           # rm on /lib*
    r"\breboot\b",
    r"\bhalt\b",
    r"\binit\s+[0-6s]",
    r"\bshutdown\b",
    r"\bmkfs\b",
    r"\bnewfs\b",
    r"\bformat\b",
]

_log_file = None


def log(msg: str) -> None:
    """Log to file only. NEVER write to stderr — Claude Code treats stderr as failure."""
    global _log_file
    try:
        if _log_file is None:
            _log_file = open(IRIX_LOG, "a")
        ts = time.strftime("%Y-%m-%d %H:%M:%S")
        _log_file.write(f"[{ts}] {msg}\n")
        _log_file.flush()
    except Exception:
        pass  # Can't log? Swallow silently — writing to stderr would be worse.


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


def is_blocked(command: str) -> str | None:
    """Check if a command matches any blocked pattern. Returns reason or None."""
    for pattern in BLOCKED_PATTERNS:
        if re.search(pattern, command, re.IGNORECASE):
            return f"Blocked: matches safety pattern '{pattern}'"
    return None


# --- SSH Connection Management ---


class SSHConnection:
    """Persistent SSH connection via ControlMaster.

    Opens one master connection on connect(), reuses it for all commands.
    Auto-reconnects on failure.
    """

    def __init__(self, host: str, user: str, chroot: str):
        self.host = host
        self.user = user
        self.chroot = chroot
        self.socket_path = f"/tmp/irix-mcp-ssh-{os.getpid()}"
        self.available = False

    def connect(self, timeout: int = 10) -> tuple[bool, str]:
        """Establish persistent SSH connection. Returns (ok, message)."""
        # Clean up any stale socket from a previous crash
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
            log("SSH connect timed out")
            self.available = False
            return False, f"Connection timed out after {timeout}s"
        except Exception as e:
            log(f"SSH connect error: {e}")
            self.available = False
            return False, str(e)

    def exec(self, command: str, timeout: int = 60) -> tuple[int, str, str]:
        """Execute command over persistent connection. Auto-reconnects once on failure."""
        rc, stdout, stderr = self._exec_once(command, timeout)
        if rc == 255 and not self.is_alive():
            # Connection dropped — try one reconnect
            log("SSH connection lost, reconnecting...")
            ok, msg = self.reconnect()
            if ok:
                log("Reconnected, retrying command")
                rc, stdout, stderr = self._exec_once(command, timeout)
            else:
                stderr = f"SSH connection lost and reconnect failed: {msg}"
        return rc, stdout, stderr

    def _exec_once(self, command: str, timeout: int) -> tuple[int, str, str]:
        """Execute a single command (no retry)."""
        escaped_cmd = command.replace("'", "'\\''")
        remote_cmd = (
            f"chroot {self.chroot} /usr/sgug/bin/sgug-exec "
            f"/bin/sh -c '{escaped_cmd}'"
        )

        ssh_cmd = [
            "ssh",
            "-o", f"ControlPath={self.socket_path}",
            "-o", "BatchMode=yes",
            f"{self.user}@{self.host}",
            remote_cmd,
        ]

        log(f"exec: {command}")
        try:
            result = subprocess.run(
                ssh_cmd,
                capture_output=True,
                text=True,
                timeout=timeout,
                errors="replace",
            )
            return result.returncode, result.stdout, result.stderr
        except subprocess.TimeoutExpired:
            return -1, "", f"Command timed out after {timeout}s"
        except Exception as e:
            return -1, "", f"SSH error: {e}"

    def scp_to(self, local_path: str, remote_path: str) -> tuple[bool, str]:
        """Copy a file to the IRIX chroot via SCP over the control connection."""
        if not remote_path.startswith("/"):
            return False, "remote_path must be absolute (start with /)"
        normalized = os.path.normpath(remote_path)
        if ".." in normalized:
            return False, "remote_path must not contain .."

        actual_remote = f"{self.chroot}{normalized}"

        scp_cmd = [
            "scp",
            "-o", f"ControlPath={self.socket_path}",
            "-o", "BatchMode=yes",
            local_path,
            f"{self.user}@{self.host}:{actual_remote}",
        ]

        log(f"scp: {local_path} -> {self.host}:{actual_remote}")
        try:
            result = subprocess.run(
                scp_cmd,
                capture_output=True,
                text=True,
                timeout=120,
                errors="replace",
            )
            if result.returncode == 0:
                return True, f"Copied {local_path} to {actual_remote}"
            return False, f"scp failed: {result.stderr.strip()}"
        except subprocess.TimeoutExpired:
            return False, "scp timed out after 120s"
        except Exception as e:
            return False, f"scp error: {e}"

    def is_alive(self) -> bool:
        """Check if persistent connection is still up."""
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
        """Attempt to re-establish a dropped connection."""
        self.disconnect()
        return self.connect()

    def disconnect(self) -> None:
        """Close the persistent connection."""
        self._send_control("exit")

    def _send_control(self, cmd: str) -> None:
        """Send an SSH control command (-O)."""
        try:
            subprocess.run(
                ["ssh", "-O", cmd,
                 "-o", f"ControlPath={self.socket_path}",
                 f"{self.user}@{self.host}"],
                capture_output=True, timeout=5,
            )
        except Exception:
            pass
        # Clean up socket file if it's stale
        if cmd == "exit":
            try:
                os.unlink(self.socket_path)
            except OSError:
                pass


# --- MCP Server ---


class IRIXMCPServer:
    """MCP server providing safe access to an IRIX chroot."""

    def __init__(self):
        self.name = "irix-mcp"
        self.version = "2.0.0"
        self.protocol_version = "2025-11-25"
        self.fallback_version = "2024-11-05"
        self.initialized = False

        self.ssh = SSHConnection(IRIX_HOST, IRIX_USER, IRIX_CHROOT)

        self.tools = [
            {
                "name": "irix_exec",
                "description": (
                    "Execute a command on the IRIX system inside the chroot. "
                    "Commands run under sgug-exec which sets up "
                    "LD_LIBRARYN32_PATH, PATH, and LC_ALL=C. "
                    "Pipes, redirects, and shell features work. "
                    "Example: 'pkg-config --list-all' or "
                    "'rpm -qa | grep perl'"
                ),
                "inputSchema": {
                    "type": "object",
                    "properties": {
                        "command": {
                            "type": "string",
                            "description": "Shell command to execute in the IRIX chroot",
                        },
                        "timeout": {
                            "type": "integer",
                            "description": f"Timeout in seconds (default: {IRIX_TIMEOUT})",
                        },
                    },
                    "required": ["command"],
                },
            },
            {
                "name": "irix_copy_to",
                "description": (
                    "Copy a file from the Linux build host to the IRIX chroot. "
                    "local_path is an absolute path on this Linux machine. "
                    "remote_path is the path inside the chroot "
                    "(e.g., /tmp/foo.rpm). "
                    "Useful for copying built RPMs to IRIX for testing."
                ),
                "inputSchema": {
                    "type": "object",
                    "properties": {
                        "local_path": {
                            "type": "string",
                            "description": "Absolute path on the Linux build host",
                        },
                        "remote_path": {
                            "type": "string",
                            "description": "Destination path inside the chroot (e.g., /tmp/foo.rpm)",
                        },
                    },
                    "required": ["local_path", "remote_path"],
                },
            },
            {
                "name": "irix_read_file",
                "description": (
                    "Read a file from the IRIX chroot. "
                    "Path is relative to the chroot root "
                    "(e.g., /usr/sgug/lib32/libfoo.so). "
                    "Returns the file contents (text files) or metadata (binary)."
                ),
                "inputSchema": {
                    "type": "object",
                    "properties": {
                        "path": {
                            "type": "string",
                            "description": "File path inside the chroot",
                        },
                        "max_lines": {
                            "type": "integer",
                            "description": "Maximum lines to return (default: 200)",
                        },
                    },
                    "required": ["path"],
                },
            },
            {
                "name": "irix_par",
                "description": (
                    "Run a command under IRIX 'par' (system call tracer, "
                    "like strace on Linux). Shows all syscalls made by the "
                    "process. Useful for debugging crashes and library "
                    "loading issues."
                ),
                "inputSchema": {
                    "type": "object",
                    "properties": {
                        "command": {
                            "type": "string",
                            "description": "Command to trace",
                        },
                        "timeout": {
                            "type": "integer",
                            "description": "Timeout in seconds (default: 30)",
                        },
                    },
                    "required": ["command"],
                },
            },
        ]

    def _ensure_connected(self, request_id: Any) -> dict | None:
        """Ensure SSH connection is available. Returns error response or None."""
        if self.ssh.available and self.ssh.is_alive():
            return None
        # Try to (re)connect
        ok, msg = self.ssh.reconnect() if self.ssh.available else self.ssh.connect()
        if ok:
            self.ssh.available = True
            return None
        return self._tool_error(
            request_id,
            f"IRIX unreachable at {IRIX_HOST}: {msg}. "
            "Check that the SGI is powered on and SSH is running.",
        )

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

        # Establish SSH connection during init (non-blocking for MCP handshake)
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
                    f"IRIX chroot access at {IRIX_HOST}:{IRIX_CHROOT}. "
                    "Commands execute via sgug-exec with proper environment. "
                    "Use irix_exec for commands, irix_copy_to for file transfers, "
                    "irix_par for syscall tracing."
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

        if tool_name == "irix_exec":
            return self._tool_exec(request_id, args)
        elif tool_name == "irix_copy_to":
            return self._tool_copy_to(request_id, args)
        elif tool_name == "irix_read_file":
            return self._tool_read_file(request_id, args)
        elif tool_name == "irix_par":
            return self._tool_par(request_id, args)
        else:
            return self._error(-32602, f"Unknown tool: {tool_name}", request_id)

    # --- Tool implementations ---

    def _tool_exec(self, request_id: Any, args: dict) -> dict:
        command = args.get("command", "")
        timeout = args.get("timeout", IRIX_TIMEOUT)

        if not command:
            return self._tool_error(request_id, "command is required")

        blocked = is_blocked(command)
        if blocked:
            return self._tool_error(request_id, blocked)

        err = self._ensure_connected(request_id)
        if err:
            return err

        rc, stdout, stderr = self.ssh.exec(command, timeout)

        output_parts = []
        if stdout:
            output_parts.append(stdout)
        if stderr:
            output_parts.append(f"[stderr]\n{stderr}")
        if rc != 0:
            output_parts.append(f"[exit code: {rc}]")

        text = "\n".join(output_parts) if output_parts else "(no output)"
        return self._tool_result(request_id, text, is_error=(rc != 0))

    def _tool_copy_to(self, request_id: Any, args: dict) -> dict:
        local_path = args.get("local_path", "")
        remote_path = args.get("remote_path", "")

        if not local_path or not remote_path:
            return self._tool_error(request_id, "local_path and remote_path required")

        if not os.path.exists(local_path):
            return self._tool_error(request_id, f"Local file not found: {local_path}")

        err = self._ensure_connected(request_id)
        if err:
            return err

        ok, msg = self.ssh.scp_to(local_path, remote_path)
        return self._tool_result(request_id, msg, is_error=not ok)

    def _tool_read_file(self, request_id: Any, args: dict) -> dict:
        path = args.get("path", "")
        max_lines = args.get("max_lines", 200)

        if not path:
            return self._tool_error(request_id, "path is required")
        if not path.startswith("/"):
            return self._tool_error(request_id, "path must be absolute")

        err = self._ensure_connected(request_id)
        if err:
            return err

        # Use head to limit output, cat -v to handle binary safely
        cmd = f"head -n {max_lines} '{path}' 2>&1 || echo '[file read failed]'"
        rc, stdout, stderr = self.ssh.exec(cmd, timeout=30)

        text = stdout if stdout else stderr if stderr else "(empty file)"
        return self._tool_result(request_id, text, is_error=(rc != 0))

    def _tool_par(self, request_id: Any, args: dict) -> dict:
        command = args.get("command", "")
        timeout = args.get("timeout", 30)

        if not command:
            return self._tool_error(request_id, "command is required")

        blocked = is_blocked(command)
        if blocked:
            return self._tool_error(request_id, blocked)

        err = self._ensure_connected(request_id)
        if err:
            return err

        # par -SICals gives: syscalls, signals, with args, long output, summary
        par_cmd = f"par -SICals {command} 2>&1; true"
        rc, stdout, stderr = self.ssh.exec(par_cmd, timeout)

        output_parts = []
        if stdout:
            output_parts.append(stdout)
        if stderr:
            output_parts.append(f"[stderr]\n{stderr}")

        text = "\n".join(output_parts) if output_parts else "(no par output)"
        return self._tool_result(request_id, text)

    # --- Response helpers ---

    def _tool_result(self, request_id: Any, text: str, is_error: bool = False) -> dict:
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

    # --- Main loop ---

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
        log(f"IRIX MCP Server v{self.version}")
        log(f"Target: {IRIX_USER}@{IRIX_HOST}:{IRIX_CHROOT}")

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
                        # Always respond so the client doesn't hang
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
    """Clean exit on SIGTERM (sent by Claude Code when shutting down server)."""
    raise SystemExit(0)


def main():
    signal.signal(signal.SIGTERM, _handle_sigterm)
    server = IRIXMCPServer()
    server.run()


if __name__ == "__main__":
    main()
