# Linux-ism Detection Guide

> **When to use:** EARLY in troubleshooting, before instrumenting code. If a feature works on Linux but silently fails on IRIX, a Linux-only API is the most likely cause.

## Quick Check Commands

Cross-compiler does NOT define `__linux__`. Verify:
```bash
echo | $MOGRIX_STAGING/bin/irix-cc -dM -E - | grep __linux  # Should be empty
```

Search C++ source for Linux-only patterns (compile-time guards):
```bash
grep -rn 'OS(LINUX)\|#ifdef __linux\|HAVE(LINUX\|PLATFORM(LINUX)' Source/
```

Search for Linux-only syscalls/APIs that compile WITHOUT guards (the dangerous ones):
```bash
grep -rn 'memfd_create\|eventfd\|signalfd\|timerfd\|epoll_\|inotify_\|accept4\|pipe2\|sendfile\|splice\|tee(' Source/
grep -rn 'recvmmsg\|sendmmsg\|fallocate\|O_TMPFILE\|readahead\|getrandom\|copy_file_range' Source/
grep -rn '/proc/self\|/proc/net\|/proc/[0-9]\|/sys/class\|/sys/devices' Source/
```

Search for abstract Unix domain sockets (Linux-only, path starts with `\0`):
```bash
grep -rn 'sun_path\[0\].*=.*0\|abstract.*socket\|\\\\0.*AF_UNIX' Source/
```

## Known Linux-only APIs and IRIX Status

### Syscalls (not in IRIX libc)

| API | Purpose | IRIX status | Fix |
|-----|---------|-------------|-----|
| `memfd_create` | Anonymous memory fd | Missing. `HAVE(LINUX_MEMFD_H)` guards it | Falls through to `shm_open` compat |
| `eventfd` | Event signaling fd | Missing. `#if OS(LINUX)` guards it | Pipe-based replacement (webkitgtk.yaml) |
| `signalfd` | Signal delivery fd | Missing | Not used by WebKit GTK |
| `timerfd_create` | Timer fd | Missing | Not used by WebKit GTK |
| `epoll_*` | I/O multiplexing | Missing | GLib abstracts to `poll()` |
| `inotify_*` | File monitoring | Missing | GLib uses FAM on IRIX |
| `accept4` | Accept with CLOEXEC | Missing | Not critical, use accept()+fcntl |
| `pipe2` | Pipe with flags | Missing | compat/sys/pipe2.c |
| `getrandom` | Random bytes | Missing | compat/sys/getrandom.c (/dev/urandom) |

### POSIX APIs with IRIX quirks

| API | Purpose | IRIX quirk | Fix |
|-----|---------|------------|-----|
| `shm_open` | POSIX shared memory | Creates files at `/` root dir! | compat/sys/shm_open.c redirects to /tmp |
| `shm_unlink` | Remove shared memory | Same path issue | compat/sys/shm_open.c |
| `SOCK_SEQPACKET` | Sequenced packets | Not supported on AF_UNIX | Use SOCK_STREAM |
| `MSG_NOSIGNAL` | Suppress SIGPIPE | Not defined | Define as 0, use SIG_IGN |
| `bsearch(nmemb=0)` | Binary search | Crashes (underflow) | libmogrix_compat.so override |
| `mmap(0,...)` | Memory map | Can return 0x0 (valid!) | dlmalloc wrapper rejects 0 |
| `O_CLOEXEC` | Close-on-exec flag | Not defined. NEVER use as value (=O_WRONLY=1!) | fcntl(fd, F_SETFD, FD_CLOEXEC) |

### Filesystem paths (Linux-only)

| Path | Purpose | IRIX status |
|------|---------|-------------|
| `/proc/self/fd` | File descriptor enumeration | Missing — GLib fdwalk brute-force fallback |
| `/proc/net/` | Network info | Missing — g_network_monitor returns NULL |
| `/dev/shm/` | POSIX shared memory tmpfs | Missing — compat uses /tmp |
| `/sys/class/` | sysfs device info | Missing |

### CMAKE_SYSTEM_NAME Trap

CMake is told `CMAKE_SYSTEM_NAME=Linux` because cmake has no IRIX support. This means CMake-level `if(CMAKE_SYSTEM_NAME MATCHES "Linux")` guards will be TRUE. Our fix:
```yaml
- $MOGRIX_ROOT/tools/safepatch Source/cmake/OptionsGTK.cmake --old 'CMAKE_SYSTEM_NAME MATCHES "Linux"' --new 'FALSE' --count 0
```
But C++ `#if OS(LINUX)` is correct (false) because `__linux__` is not defined by irix-cc.

**Danger zone:** Code that uses CMake to set compile definitions like `-DHAVE_MEMFD=1` — the CMake check runs on the BUILD host (Linux) not the target (IRIX), so feature checks pass incorrectly. Always verify `HAVE_*` defines match IRIX reality.

## Process for New Packages

1. After unpacking source, grep for the patterns above
2. For each hit: check if it's guarded by `#if OS(LINUX)` or similar
3. UNGUARDED Linux-only calls are the dangerous ones — they compile fine but fail at runtime
4. Check our compat library: `grep 'symbol_name' compat/catalog.yaml`
5. If compat exists, ensure `inject_compat_functions` includes it
6. If no compat, add one or use prep_commands to replace the call
