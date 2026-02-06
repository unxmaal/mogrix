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
  IRIX_LOG        - Log file path (default: stderr only)
"""
import sys
import json
import time
import os
import re
import subprocess
import shlex
from typing import Any

# --- Configuration ---

IRIX_HOST = os.environ.get("IRIX_HOST", "192.168.0.81")
IRIX_USER = os.environ.get("IRIX_USER", "root")
IRIX_CHROOT = os.environ.get("IRIX_CHROOT", "/opt/chroot")
IRIX_TIMEOUT = int(os.environ.get("IRIX_TIMEOUT", "60"))
IRIX_LOG = os.environ.get("IRIX_LOG", "")

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
    """Log to stderr and optionally to a file."""
    global _log_file
    ts = time.strftime("%Y-%m-%d %H:%M:%S")
    line = f"[{ts}] {msg}"
    print(line, file=sys.stderr, flush=True)
    if IRIX_LOG:
        if _log_file is None:
            _log_file = open(IRIX_LOG, "a")
        _log_file.write(line + "\n")
        _log_file.flush()


def read_message() -> dict | None:
    line = sys.stdin.readline()
    if not line:
        return None
    return json.loads(line)


def write_message(msg: dict) -> None:
    json.dump(msg, sys.stdout, ensure_ascii=False)
    sys.stdout.write("\n")
    sys.stdout.flush()


def is_blocked(command: str) -> str | None:
    """Check if a command matches any blocked pattern. Returns reason or None."""
    for pattern in BLOCKED_PATTERNS:
        if re.search(pattern, command, re.IGNORECASE):
            return f"Blocked: matches safety pattern '{pattern}'"
    return None


def ssh_exec(command: str, timeout: int | None = None) -> tuple[int, str, str]:
    """Execute a command on IRIX inside the chroot via sgug-exec.

    Returns (returncode, stdout, stderr).
    """
    if timeout is None:
        timeout = IRIX_TIMEOUT

    # Build the remote command:
    #   chroot /opt/chroot /usr/sgug/bin/sgug-exec /bin/sh -c '<command>'
    #
    # We use /bin/sh -c so pipes, redirects, etc. work naturally.
    # sgug-exec sets up PATH, LD_LIBRARYN32_PATH, LC_ALL=C.
    escaped_cmd = command.replace("'", "'\\''")
    remote_cmd = (
        f"chroot {IRIX_CHROOT} /usr/sgug/bin/sgug-exec "
        f"/bin/sh -c '{escaped_cmd}'"
    )

    ssh_cmd = [
        "ssh",
        "-o", "ConnectTimeout=10",
        "-o", "ServerAliveInterval=15",
        "-o", "ServerAliveCountMax=4",
        "-o", "BatchMode=yes",
        "-o", "StrictHostKeyChecking=no",
        f"{IRIX_USER}@{IRIX_HOST}",
        remote_cmd,
    ]

    log(f"SSH exec: {command}")
    try:
        result = subprocess.run(
            ssh_cmd,
            capture_output=True,
            text=True,
            timeout=timeout,
        )
        return result.returncode, result.stdout, result.stderr
    except subprocess.TimeoutExpired:
        return -1, "", f"Command timed out after {timeout}s"
    except Exception as e:
        return -1, "", f"SSH error: {e}"


def scp_to_irix(local_path: str, remote_path: str) -> tuple[bool, str]:
    """Copy a file from the Linux build host to the IRIX chroot.

    remote_path is relative to the chroot (e.g., /tmp/foo.rpm).
    The actual destination is {IRIX_CHROOT}{remote_path}.
    """
    # Validate remote_path is absolute and doesn't escape
    if not remote_path.startswith("/"):
        return False, "remote_path must be absolute (start with /)"
    # Resolve to prevent ../ escapes
    normalized = os.path.normpath(remote_path)
    if ".." in normalized:
        return False, "remote_path must not contain .."

    actual_remote = f"{IRIX_CHROOT}{normalized}"

    scp_cmd = [
        "scp",
        "-o", "ConnectTimeout=10",
        "-o", "BatchMode=yes",
        "-o", "StrictHostKeyChecking=no",
        local_path,
        f"{IRIX_USER}@{IRIX_HOST}:{actual_remote}",
    ]

    log(f"SCP: {local_path} -> {IRIX_HOST}:{actual_remote}")
    try:
        result = subprocess.run(
            scp_cmd,
            capture_output=True,
            text=True,
            timeout=120,
        )
        if result.returncode == 0:
            return True, f"Copied {local_path} to {actual_remote}"
        return False, f"scp failed: {result.stderr}"
    except subprocess.TimeoutExpired:
        return False, "scp timed out after 120s"
    except Exception as e:
        return False, f"scp error: {e}"


class IRIXMCPServer:
    """MCP server providing safe access to an IRIX chroot."""

    def __init__(self):
        self.name = "irix-mcp"
        self.version = "1.0.0"
        self.protocol_version = "2025-11-25"
        self.fallback_version = "2024-11-05"
        self.initialized = False

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

        rc, stdout, stderr = ssh_exec(command, timeout)

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

        ok, msg = scp_to_irix(local_path, remote_path)
        return self._tool_result(request_id, msg, is_error=not ok)

    def _tool_read_file(self, request_id: Any, args: dict) -> dict:
        path = args.get("path", "")
        max_lines = args.get("max_lines", 200)

        if not path:
            return self._tool_error(request_id, "path is required")
        if not path.startswith("/"):
            return self._tool_error(request_id, "path must be absolute")

        # Use head to limit output, cat -v to handle binary safely
        cmd = f"head -n {max_lines} '{path}' 2>&1 || echo '[file read failed]'"
        rc, stdout, stderr = ssh_exec(cmd, timeout=30)

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

        # par -SICals gives: syscalls, signals, with args, long output, summary
        par_cmd = f"par -SICals {command} 2>&1; true"
        rc, stdout, stderr = ssh_exec(par_cmd, timeout)

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
            log("Initialized - ready")
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

        while True:
            try:
                msg = read_message()
                if msg is None:
                    log("stdin closed, exiting")
                    break

                response = self.handle_request(msg)
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
            except Exception as e:
                log(f"Error: {e}")
                import traceback
                traceback.print_exc(file=sys.stderr)

        log("Shutdown")


def main():
    server = IRIXMCPServer()
    server.run()


if __name__ == "__main__":
    main()
