# WebKit IPC Connection Debugging

**Problem:** MiniBrowser reports "WebProcess CRASHED" repeatedly. WebKitWebProcess launches, all 70+ libraries load, it runs for 25-47 seconds, then the IPC connection drops. MiniBrowser detects EOF/error on the AF_UNIX SOCK_STREAM socket, calls `connectionDidClose()` → `processDidTerminateOrFailedToLaunch(Crash)` → g_warning "WebProcess CRASHED" → sends SIGTERM to WebProcess.

**What was eliminated:**
- Memory fault (SIGSEGV/SIGBUS) in WebProcess — signal catcher confirmed only SIGTERM
- IRIX IPC primitive failures — comprehensive test of socketpair, SCM_RIGHTS, poll, shm_open all pass
- GOT overflow — fixed with `-Bsymbolic` (global GOT 4661→3515, under 4370 limit)
- SharedMemory/shm_open — works in chroot (as root) but FAILS on host as non-root user (see "IRIX shm_open creates files at /" above). Fixed with compat shm_open in libmogrix_compat.so
- Par tracing — too heavyweight, causes IPC timeouts, can't trace fork+exec children

**Current approach:** Compile-time IPC debug logging via `ipc_debug_log.h` macros. 12 injection points in `ConnectionUnix.cpp`:
- `IPC_LOG_OPEN` — connection established (sock fd, pid)
- `IPC_LOG_MSG` — message processed (count every 50th, pid)
- `IPC_LOG_CLOSE_*` — 5 variants: EOF, ECONNRESET, RECV_ERR, GIO_HUP, SEND_CONNRESET
- `IPC_LOG_RECV_ERR` — recvmsg failure (errno, sock)
- `IPC_LOG_CTRUNC` — MSG_CTRUNC (control data truncated)
- `IPC_LOG_SEND_ERR` — sendmsg failure
- `IPC_LOG_INVALIDATE` — intentional connection close

Each process writes to `/usr/people/edodd/ipc_<pid>.log` using unbuffered `write()` — safe even on crash. Both MiniBrowser and WebProcess use the same code, so both sides log.

**Files:** `patches/packages/webkitgtk/ipc_debug_log.h`, prep_commands in `rules/packages/webkitgtk.yaml` (marked TEMPORARY).

**IPC log results (2026-02-21):**
Only MiniBrowser logs exist — WebProcess never creates an ipc log, meaning it dies before IPC init.
MiniBrowser log shows: OPEN → RECV_ERR errno=11 (EAGAIN) → CLOSE:EOF → INVALIDATE, repeating across sockets 15/20/21. WebProcess opens and immediately closes.

**Root cause hypothesis: GLib g_fdwalk_set_cloexec + missing /proc/self/fd.**
IRIX has no `/proc/self/fd` (confirmed). GLib's `g_subprocess_launcher` uses `g_fdwalk_set_cloexec` to close-on-exec all fds in the child except those passed via `take_fd`. Without `/proc/self/fd`, GLib falls back to brute-force fd iteration. If this fallback is buggy (off-by-one, wrong exemption list), it will FD_CLOEXEC the IPC socket passed to WebProcess, causing it to be closed on exec. WebProcess then starts with an invalid socket fd, can't read anything, exits immediately. MiniBrowser sees EOF.

**Evidence:**
- WebProcess runs fine standalone (GTK warning only, exit 1)
- WebProcess never produces IPC log (dies before IPC init)
- All connections show EOF (other end hung up = fd was invalid)
- RECV_ERR errno=11 is EAGAIN (normal for non-blocking socket)
- `/proc/self` doesn't exist on IRIX, `/proc/self/fd` definitely doesn't
- `ProcessLauncherGLib.cpp` sets `SetCloexecOnClient | SetCloexecOnServer` and relies on `take_fd` to preserve the client socket through exec

**Proposed fix:** Patch `ProcessLauncherGLib.cpp` `connectionOptions()` to NOT set `SetCloexecOnClient` when `defined(__sgi)`. This matches the LIBWPE path which also avoids client CLOEXEC. Alternative: fix/replace GLib's fdwalk for IRIX.
