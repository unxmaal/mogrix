# IRIX Platform Quirks

> **For AI agents**: These are IRIX-specific issues that may affect package porting.
> Check this list when encountering unexpected build or runtime failures.

---

## Header Namespace Collisions

IRIX system headers define some symbols that conflict with common application code.

### `struct comment` in pwd.h

**Location**: `/usr/include/pwd.h` (line 66)

**Problem**: IRIX's pwd.h defines `struct comment` for an obscure password file feature. Applications that define their own `struct comment` will fail to compile.

**Symptom**:
```
error: redefinition of 'comment'
./app.h:XX: struct comment {
/opt/irix-sysroot/usr/include/pwd.h:66: note: previous definition is here
```

**Solution**: Rename the application's struct. Example patch:
```bash
sed -i 's/struct comment/struct app_comment/g' *.h *.c
```

**Affected packages**: tree-pkg (renamed to `struct tree_comment`)

---

## Platform Detection Macros

Different packages use different macros to detect Linux/Unix platforms.

| Macro | Common In | IRIX Equivalent |
|-------|-----------|-----------------|
| `LINUX` | Older code | `__sgi` |
| `__linux__` | Modern code | `__sgi` |
| `linux` | Legacy | `__sgi` or `sgi` |
| `__unix__` | Portable code | Already defined on IRIX |

**Note**: IRIX defines `__sgi`, `__sgi__`, `sgi`, `__unix__`, `__mips__`, and `_MIPS_SIM`.

When porting, check what platform macro the source uses and add appropriate `#elif defined(__sgi)` blocks or sed replacements.

---

## Missing/Different Functions

### vsnprintf C99 behavior

**Problem**: IRIX vsnprintf does NOT return required buffer size when passed NULL/0.

```c
// C99 (Linux): returns 5
int n = vsnprintf(NULL, 0, "hello");

// IRIX: returns -1
int n = vsnprintf(NULL, 0, "hello");
```

**Solution**: Use iterative buffer doubling or inject mogrix's vasprintf.

**Affected packages**: popt, rpm (both inject vasprintf compat function)

### `%zu` format specifier (CRITICAL — causes SIGSEGV)

**Problem**: IRIX libc's printf family does NOT support the C99 `z` length modifier (`%zu`, `%zd`, `%zx`). When printf encounters `%zu`, it does not correctly consume the `size_t` argument. This **corrupts the varargs parsing**, causing all subsequent format arguments to read from wrong positions. If a `%s` follows, it reads a garbage pointer and crashes with SIGSEGV.

**Symptom**: SIGSEGV inside snprintf/fprintf/printf when the format string contains `%zu` followed by `%s`. The crash appears to be in the function calling printf, not in printf itself. Debug markers using `write()` will work but `fprintf()` markers will also crash (since fprintf uses printf internally).

**Example**:
```c
// CRASHES on IRIX — %zu corrupts varargs, %s reads garbage → SIGSEGV
fprintf(stderr, "file %s line %zu func %s", "foo.c", (size_t)42, "main");

// WORKS — %u is understood by IRIX libc
fprintf(stderr, "file %s line %u func %s", "foo.c", (unsigned int)42, "main");
```

**Fix**: Replace `%zu` with `%u` (safe on MIPS n32 where `size_t` is 4 bytes = `unsigned int`). Also replace `%zd` with `%d` and `%zx` with `%x`.

**Debugging tip**: When debugging crashes on IRIX, use raw `write(2, msg, len)` for markers — never use fprintf/printf, as those will also crash if the bug is a `%zu` format string issue.

**Affected packages**: pkgconf (`SIZE_FMT_SPECIFIER` macro, patched in `pkgconf-irix-no-zu-format.patch`)

**WARNING**: Many modern C packages use `%zu`. This WILL be a recurring issue. Always search for `%zu`, `%zd`, `%zx` when porting a new package.

### dlmalloc + libc allocator mismatch (CRITICAL — causes SIGSEGV)

**Problem**: mogrix injects dlmalloc which replaces `malloc`/`calloc`/`realloc`/`free` in our binaries. However, IRIX libc functions like `strdup`, `asprintf`, `getline` internally call **libc's** malloc (not dlmalloc). If our code calls libc's `strdup()` and later calls dlmalloc's `free()` on the result, the heap is corrupted → SIGSEGV.

**Symptom**: Crash during normal operations after successful library loading and initialization. Often in `free()` or `realloc()` when processing data that was allocated by a libc function.

**Solution**: Any package using dlmalloc must also inject compat replacements for libc functions that allocate memory:
- `strdup` — calls our `malloc` instead of libc's
- `strndup` — same
- `asprintf` / `vasprintf` — if used
- `getline` / `getdelim` — if used

The compat versions call `malloc` (which resolves to dlmalloc), ensuring consistent allocator usage.

**Affected packages**: pkgconf (injects strdup + strndup compat)

### O_NOFOLLOW

**Problem**: IRIX doesn't support O_NOFOLLOW flag - returns EINVAL.

**Solution**: Strip the flag in openat-compat.c (already done in mogrix compat layer).

### Thread-local storage (__thread)

**Problem**: IRIX rld doesn't support `__tls_get_addr`.

**Symptom**: Undefined symbol `__tls_get_addr` at runtime (NOT a link error — it crashes at runtime because rld does lazy symbol resolution).

**Solution**: Disable TLS at configure time. Use `sed` to `#define` or `#undef` the TLS macro in config headers — **not** `CFLAGS -D`, because autoconf may override CFLAGS. For meson packages, use `-Dtls=disabled`.

**Affected packages**: rpm (patched to remove `static __thread`), gnutls, gdbm, pixman, cairo (GTK3 rendering crash traced to pixman TLS)

### IRIX X11R6.3 Missing APIs

**Problem**: IRIX ships X11R6.3, which lacks some APIs added in later X11 releases.

**Missing:** `XA_UTF8_STRING`, `Xutf8TextListToTextProperty`, `Xutf8LookupString`, `XICCallback` typedef.

**Solutions:**
- `XA_UTF8_STRING` → `XInternAtom(dpy, "UTF8_STRING", False)`
- `Xutf8*` functions → `Xmb*` equivalents (multi-byte, works for ASCII/Latin1)
- `XICCallback` → `XIMCallback` (identical struct, different typedef name)

**Affected packages**: st, xclip, xnedit, any X11 app using UTF-8 input methods

### dlopen of libpthread/libstdc++ Crashes

**Problem**: IRIX system libraries crash when loaded via `dlopen()` rather than being linked at load time as NEEDED dependencies.

**Solution**: Link the executable with `-lpthread -lstdc++` directly so rld loads them at startup. Don't rely on plugins or shared libraries pulling them in via dlopen.

---

## Linker / Loader Issues

### GNU ld linker scripts (rld can't load them)

**Problem**: Some Fedora packages install `.so` files that are actually GNU ld linker scripts (text files containing `INPUT(-lfoo)` or `GROUP(-lfoo -lbar)`). IRIX rld can ONLY load real ELF `.so` files and crashes when trying to parse linker scripts.

**Symptom**:
```
rld: Error: elfmap failed on /usr/sgug/lib32/libcurses.so : read 16 bytes too short to be elf header
```

**Solution**: Replace `echo "INPUT(-lfoo)"` with `ln -sf` symlinks in the package's install step.

**Affected packages**: ncurses (libcurses.so, libcursesw.so, libtermcap.so — fixed in ncurses.yaml)

**WARNING**: Check for this in any package that creates `.so` wrapper files. Search for `INPUT(` or `GROUP(` in installed `.so` files.

### `-z separate-code` for shared libraries

**Problem**: Without `-z separate-code`, GNU ld may produce shared libraries with a single RWE LOAD segment (when there's no `.data` section). IRIX rld crashes on these because it expects at least 3 segments (R/RE/RW).

**Solution**: Already fixed in `irix-ld` (2026-02-05) — the `-z separate-code` flag is passed automatically for all shared library links.

---

## Filesystem Differences

### /dev/fd behavior

**Problem**: IRIX has /dev/fd but `utimes("/dev/fd/N", ...)` doesn't work.

**Solution**: Use filename-based utimes instead of fd-based.

**Affected packages**: rpm (futimens disabled, uses utimensat with pathname)

### Long filenames in tar

**Problem**: IRIX tar corrupts GNU-style long filenames (>100 chars).

**Solution**: Use `createrepo_c --simple-md-filenames` for repositories.

---

## Type Size Differences

### mode_t

**Problem**: IRIX mode_t is `unsigned long` (4 bytes), Linux is `unsigned int`.

**Symptom**: Format warnings like:
```
warning: format specifies type 'unsigned int' but argument has type 'mode_t' (aka 'unsigned long')
```

**Solution**: Use `%lo` format specifier or cast to `unsigned int`.

---

## Kernel Primitives

### setcontext (syscall 1169)

Atomically restores all registers + sigmask from a `ucontext_t`. This is a kernel transition, not userspace — once called, the caller's context is gone. Used by Go runtime for async preemption signal recovery.

### sigreturn (syscall 1088)

Honors modified EPC in the ucontext. IRIX `_sigtramp` (in libc) saves/restores `errno` around the signal handler, then calls `sigreturn` to resume execution.

### PRDA at 0x200F80

The Process Data Area is per-pthread, not just per-process. Verified: 8 threads, 80K stress read-backs all correct. IRIX pthreads are 1:1 with sprocs. Safe to use as TLS-like storage per thread.

### libpthread reserved signals

IRIX libpthread reserves signals 47 (`SIGPTINTR`) and 48 (`SIGPTRESCHED`) for internal thread scheduling. **Never install signal handlers for these. Never block/unblock them.** Any code that installs handlers for a range of signals must skip 47-48.

### No pkill command

IRIX has no `pkill`. To kill processes by name: `ps -ef | grep name` + `kill <pid>`. This matters when debugging — stale processes can't be killed by name and may hold binary files open.

---

## Adding New Quirks

When you encounter a new IRIX-specific issue:

1. Document it here with:
   - Location/cause
   - Symptom (error message)
   - Solution
   - Affected packages

2. If it affects multiple packages, consider:
   - Adding a compat header in `cross/include/dicl-clang-compat/`
   - Adding a compat function in `compat/runtime/`
   - Creating a generic rule pattern

3. If it's package-specific:
   - Handle in the package's YAML rule file
   - Add a comment explaining why

---

## C++ Exception Handling is Broken

C++ exceptions compiled by clang for IRIX crash with SIGSEGV instead of unwinding. The DWARF unwinder in our clang-built runtime doesn't work correctly for throw/catch.

### std::map::at() / std::unordered_map::at() → SIGSEGV

**Problem**: `.at()` throws `std::out_of_range` when a key is missing. On IRIX, the throw triggers SIGSEGV (broken unwinder) instead of a catchable exception.

**Symptom**: SIGSEGV during normal operation when a map lookup fails — no backtrace, no error message.

**Solution**: Replace `.at()` with `.find()` + null check:
```cpp
// WRONG on IRIX:
auto& val = my_map.at(key);

// CORRECT:
auto it = my_map.find(key);
if (it == my_map.end()) return default_value;
auto& val = it->second;
```

**Affected packages**: btop (4 instances of `graph_symbols.at()` in btop_draw.cpp). Any C++ app using `.at()` with dynamic keys.

**Scope**: GLOBAL — affects any C++ code that could throw exceptions.

---

## Thread Safety: getpwuid → getpwuid_r

**Problem**: `getpwuid()` returns a pointer to static internal data. In multithreaded apps, concurrent calls from different threads corrupt each other's results, leading to crashes or wrong data.

**Symptom**: SIGSEGV or garbled usernames in process lists, user lookups, etc.

**Solution**: Use the POSIX reentrant version `getpwuid_r()`:
```cpp
struct passwd pwd_buf;
char pw_strbuf[256];
struct passwd *pwd_result = nullptr;
if (getpwuid_r(uid, &pwd_buf, pw_strbuf, sizeof(pw_strbuf), &pwd_result) == 0 && pwd_result)
    username = pwd_result->pw_name;
else
    username = std::to_string(uid);
```

Available on IRIX via `_SGI_REENTRANT_FUNCTIONS` (set by our `-D_SGI_REENTRANT_FUNCTIONS` flag).

**Note**: `getpwuid_r` on IRIX does NIS lookups for unknown UIDs which can block for seconds. Consider falling back to numeric UID if the lookup is slow or returns ENOENT.

**Affected packages**: btop (process collection thread). Any multithreaded app doing user lookups.

**Scope**: GLOBAL — affects any multithreaded app calling `getpwuid()`.

---

## Negative int → size_t Underflow in String Functions

**Problem**: Functions like `ljust()`, `rjust()`, `string::resize()` take `size_t` (unsigned) parameters. When computed int values go negative (e.g., terminal width calculations), the implicit conversion wraps to ~4GB, causing massive allocation → OOM SIGSEGV.

**Symptom**: SIGSEGV during TUI rendering when the terminal is narrow.

**Solution**: Clamp values before passing to size_t functions:
```cpp
prog_size = max(0, prog_size);
cmd_size = max(0, cmd_size);
```

**Affected packages**: btop (Proc::draw size calculations). Any TUI app computing layout sizes from terminal dimensions.

**Scope**: GLOBAL — affects any code computing sizes from terminal width/height.
