# Mogrix Cross-Compilation Handoff

**Last Updated**: 2026-01-23
**Status**: Validating mogrix workflow with tdnf dependency chain

## Current Session: Mogrix Workflow Validation

Testing the full mogrix fetch → convert → rpmbuild workflow for the tdnf dependency chain.

### Package Validation Status

| Package | Mogrix Workflow | Notes |
|---------|-----------------|-------|
| zlib | DONE | No issues |
| bzip2 | DONE | No issues |
| openssl | IN PROGRESS | 3.2.1 - multiple fixes needed (see below) |
| curl | PENDING | Next after openssl |
| rpm | PENDING | |
| tdnf | PENDING | |

### OpenSSL 3.2.1 Fixes Required

The OpenSSL 3.2.1 build from Fedora 40 required several new compat additions:

**Compat Headers Created:**
- `sys/socket.h` - Undefine `_SGI_SOURCE` to get XPG msghdr with msg_flags
- `sys/random.h` - Stub for getrandom() (returns -1/ENOSYS)
- `string.h` - Declaration for strerror_r()
- `ctype.h` - Inline isblank() function
- `complex.h` - Standalone implementation (avoids IRIX __c99 check and va_list conflicts)

**Compat Functions Added:**
- `strerror_r` (GNU-style, returns char*) - `compat/error/strerror_r.c`
- `_rld_new_interface` stub - `compat/rld/rld_new_interface.c`

**irix-ld Wrapper Updates:**
- Filter out `-static-libgcc` (GCC-specific flag)
- Convert `-soname,name` to `-soname name` (GNU ld format)

**openssl.yaml spec_replacements:**
- sslarch forced to irix-mips3-gcc with no-asm no-async
- FIPS disabled (requires Linux-specific link.h, dladdr)
- kTLS disabled (kernel TLS, Linux-specific)
- SCTP disabled (Linux SCTP not available)
- GCC-specific compiler flags removed
- EX_LIBS passed to make for compat library linking
- All FIPS-related patches commented out
- Perl `rename` command replaced with shell loop (pending test)

## Solution Summary

Cross-compiled dynamically-linked executables and shared libraries now work on IRIX. Two key discoveries:

1. **Shared libraries**: Use GNU ld (`mips-sgi-irix6.5-ld.bfd`) for correct 2-LOAD segment layout
2. **Executables**: Use LLD with `--dynamic-linker=/lib32/rld` interpreter (critical!)

### What Works

- **tdnf package manager**: Runs on IRIX (needs configuration files)
- **dumpsolv**: Dynamically linked executable runs on IRIX
- **Shared libraries**: Load and run correctly on IRIX via dlopen
- **Dynamically linked executables**: Work with `/lib32/rld` interpreter
- **popt package**: Successfully cross-compiled with working libpopt.so
- **expat package**: Successfully cross-compiled with working libexpat.so and xmlwf executable
- **libxml2 package**: Successfully cross-compiled with working libxml2.so and xmllint executable
- **zlib 1.2.13**: Cross-compiled, both direct linking and dlopen work
- **OpenSSL 1.1.1g**: Cross-compiled (libs only), dlopen works with RTLD_LAZY
- **sqlite 3.42.0**: Cross-compiled, direct linking works (dlopen crashes due to pthread issue)
- **curl 8.2.1**: Cross-compiled with OpenSSL/zlib, HTTP and HTTPS working
- **libgpg-error 1.47**: Cross-compiled using gen-posix-lock-obj generated on IRIX
- **libsolv 0.7.30**: Cross-compiled with shared libraries and RPM support
- **rpm 4.15.0**: Cross-compiled with shared libraries
- **Automatic linker selection**: irix-ld wrapper detects `-shared` and uses GNU ld
- **-Wl flag translation**: irix-cc wrapper translates `-Wl,xxx` to `xxx` for linker

### Key Technical Details

| Tool | Use Case | Notes |
|------|----------|-------|
| clang | Compilation | Via irix-cc wrapper |
| ld.lld-irix | Executables | LLD with `--dynamic-linker=/lib32/rld` |
| mips-sgi-irix6.5-ld.bfd | Shared libraries | GNU ld required for correct segment layout |
| llvm-ar | Static libraries | Standard archiver |

### Critical: Dynamic Linker Interpreter

**Problem**: Cross-compiled executables segfaulted even with correct shared libraries.

**Root Cause**: LLD defaults to `/usr/lib32/libc.so.1` as interpreter, but IRIX requires `/lib32/rld`.

**Solution**: Add `--dynamic-linker=/lib32/rld` to LLD link command for all executables.

```bash
# What didn't work
--dynamic-linker=/usr/lib32/libc.so.1  # Segfaults

# What works
--dynamic-linker=/lib32/rld  # IRIX runtime linker
```

### Segment Layout (Why It Matters)

IRIX rld requires a specific ELF segment layout:

| Linker | LOAD Segments | Result |
|--------|---------------|--------|
| GNU ld | 2 (R E + RW) | Works on IRIX |
| LLD | 3 (R + R E + RW) | Crashes IRIX rld |

## Implementation

### 1. Updated irix-ld Wrapper (`cross/bin/irix-ld`)

The linker wrapper now automatically selects the appropriate linker:

```bash
if [ "$is_shared" = "1" ]; then
    # Shared library: use GNU ld
    exec $GNULD "$@"
else
    # Executable: use LLD with CRT files
    exec $LLD ... "$@" ...
fi
```

### 2. Fixed -Wl Flag Translation (`cross/bin/irix-cc`)

The compiler wrapper now translates `-Wl,xxx` flags to direct linker flags:

```bash
# In link_only mode
-Wl,*)
    # Strip -Wl, prefix and add the actual linker flag
    ld_flag=$(echo "$arg" | sed 's/^-Wl,//')
    link_args="$link_args $ld_flag"
    ;;
```

This was critical for libtool which passes flags like `-Wl,--whole-archive` that need to become `--whole-archive` for the linker.

**Updates (2026-01-23)**:
- Translates comma-separated `-Wl,` flags: `-Wl,-soname,foo` → `--soname foo`
- Translates single-dash options to double-dash: `-soname` → `--soname`
- Skips GCC-specific flags: `-static-libgcc`, `-static-libstdc++`

### 2. Added Mogrix Rule Support

New rule types added to mogrix:
- `export_vars`: Set environment variables in %build (e.g., `LD: "/path/to/ld"`)
- `skip_find_lang`: Comment out %find_lang for cross-compilation
- `install_cleanup`: Run cleanup commands after %make_install

### 3. Fixed Compat Source Linking

Changed compat function injection to create a static archive instead of passing .o files directly, avoiding glob expansion issues with libtool.

## Validation

### popt
```bash
# On IRIX
$ /tmp/test-popt-irix.sh
Testing odump on libpopt.so.0.0.2...
GCC return code: 0
dlopen succeeded!
Found poptGetContext at 5ffe1258
Test return code: 0
```

### expat
```bash
# On IRIX
$ dltest-expat
dlopen succeeded!
Found XML_ParserCreate at 5ffe2440

$ xmlwf-real --version
xmlwf-real using expat_2.6.0

$ xmlwf-real valid.xml
(no output = success)

$ xmlwf-real invalid.xml
invalid.xml:2:0: no element found
```

### libxml2
```bash
# On IRIX
$ ./dlxml
Starting dlopen...
dlopen succeeded!
xmlReadMemory: 60026e80

$ ./xmllint --version
./xmllint: using libxml version 21205
   compiled with: Tree Output Push Reader Patterns Writer SAXv1 FTP HTTP...

$ ./xmllint --noout test.xml
(no output = success)
```

### zlib 1.2.13
```bash
# On IRIX
$ ./test-zlib  # direct link
Compressed 23 bytes to 31 bytes
Decompressed: Hello, IRIX zlib test!
zlib version: 1.2.13

$ ./dltest-zlib  # dlopen
dlopen succeeded!
zlibVersion: 1.2.13
```

### OpenSSL 1.1.1g
```bash
# On IRIX (dlopen with RTLD_LAZY)
$ ./test-openssl
Testing dlopen on cross-compiled libcrypto...
dlopen succeeded!
OpenSSL version: OpenSSL 1.1.1g  21 Apr 2020
```

Note: OpenSSL has unresolved symbol `ec_GF2m_simple_oct2point` when loaded with RTLD_NOW.
Using RTLD_LAZY works for basic SSL/TLS functions.

### sqlite 3.42.0
```bash
# On IRIX (direct link)
$ ./sqlite3 --version
3.42.0 2023-05-16 12:36:15 831d0fb2836b71c9bc51067c49fee4b8f18047814f2ff22d817d25195cf350b0

$ ./test-sqlite-link
sqlite3 version (compiled): 3.42.0
sqlite3 version (runtime): 3.42.0
In-memory database opened successfully!
```

Note: Built with `-DLONGDOUBLE_TYPE=double` to avoid libgcc 128-bit float symbols (`__floatditf`, etc.)
that are hidden in SGUG-RSE's libgcc. Direct linking works; dlopen crashes due to pthread issue.

### curl 8.2.1
```bash
# On IRIX
$ LD_LIBRARY_PATH=/tmp /tmp/curl --version
curl 8.2.1 (mips-sgi-irix6.5) libcurl/8.2.1 OpenSSL/1.1.1g zlib/1.2.13
Protocols: dict file ftp ftps gopher gophers http https imap imaps mqtt pop3 pop3s rtsp smb smbs smtp smtps telnet tftp
Features: alt-svc HSTS HTTPS-proxy IPv6 Largefile libz NTLM NTLM_WB SSL threadsafe TLS-SRP UnixSockets

# HTTP request
$ LD_LIBRARY_PATH=/tmp /tmp/curl -s http://example.com | head -1
<!doctype html>...

# HTTPS request (TLSv1.3 works)
$ LD_LIBRARY_PATH=/tmp /tmp/curl https://example.com
(fetches page successfully)
```

Note: Built with `--disable-threaded-resolver` to avoid pthread dlopen issues.

### libsolv 0.7.30
```bash
# On IRIX - dumpsolv (dynamically linked executable)
$ LD_LIBRARY_PATH=/tmp /tmp/dumpsolv
unexpected EOF
not a SOLV file
could not read repository: not a SOLV file
# (exit 1 is expected - no input file)

# dlopen test works despite pthread usage in fopencookie
$ /tmp/test-solvext
Testing libsolvext dlopen...
libsolv loaded OK
libsolvext loaded OK
Found solv_xfopen at 5c002a10
```

Note: Built with portable fopencookie implementation from mogrix compat library.
Both libsolv.so and libsolvext.so load and work via dlopen despite using pthreads
in the fopencookie proxy code - the pthread is only created when fopencookie() is
actually called, not at library load time.

### tdnf (GOAL ACHIEVED)
```bash
# On IRIX - tdnf runs!
$ LD_LIBRARY_PATH=/tmp /tmp/tdnf repolist
Error(1602) : No such file or directory
Exit: 66
# (Expected - no /etc/tdnf/tdnf.conf configuration file)

$ LD_LIBRARY_PATH=/tmp /tmp/tdnf --version
tdnf: 3.5.2
# (tdnf reports version successfully)
```

Note: tdnf is dynamically linked to libsolv.so, libsolvext.so, libtdnf.so, libtdnfcli.so,
librpm.so, librpmio.so, libcurl.so, and others. All cross-compiled shared libraries work.
Next step is creating configuration files and setting up a repository.

## Workflow

Standard mogrix workflow now produces working IRIX shared libraries:

```bash
# Fetch and convert
mogrix fetch popt
mogrix convert popt-*.src.rpm -o workdir/converted

# Build (uses updated irix-ld wrapper automatically)
mogrix build workdir/converted/popt-*.src.rpm --cross

# Test on IRIX
scp ~/rpmbuild/RPMS/mips/popt-*.rpm edodd@192.168.0.81:/tmp/
ssh edodd@192.168.0.81 'sudo rpm -Uvh /tmp/popt-*.rpm'
```

## Files Modified This Session

### Current Session (tdnf working on IRIX)

| File | Change |
|------|--------|
| `workdir/libsolv/libsolv-0.7.30/irix-toolchain-shared.cmake` | Added `-lpthread`, fixed interpreter |
| `workdir/tdnf/irix-toolchain.cmake` | Added interpreter, rpm libs, crypto libs |
| `/opt/sgug-staging/usr/sgug/include/irix-compat.h` | Added scandir/alphasort declarations |
| `plan.md` | Updated to Phase 6: tdnf RUNNING ON IRIX |
| `HANDOFF.md` | Updated with tdnf success |

### Previous Sessions

| File | Change |
|------|--------|
| `cross/bin/irix-ld` | Added GNU ld selection for shared libraries |
| `cross/bin/irix-cc` | Added `-Wl,xxx` to `xxx` translation for linker |
| `mogrix/rules/engine.py` | Added export_vars, skip_find_lang, install_cleanup |
| `mogrix/emitter/spec.py` | Added handling for new rule types |
| `mogrix/cli.py` | Wire new rule types to spec writer |
| `mogrix/compat/injector.py` | Build static archive instead of glob pattern |
| `rules/packages/popt.yaml` | Reference implementation with export_vars |
| `rules/packages/expat.yaml` | expat rules with strtof, skip_manpages, install_cleanup |
| `compat/strtof.c` | Added HUGE_VALF definition for IRIX |
| `compat/catalog.yaml` | Added strtof function entry |
| `rules/packages/sqlite.yaml` | sqlite rules for cross-compilation |
| `rules/packages/curl.yaml` | Updated with --disable-threaded-resolver |
| `rules/packages/libgpg-error.yaml` | Added skip_check |
| `compat/stdio/fopencookie.c` | Portable fopencookie() implementation for libsolv |
| `compat/stdio/fopencookie.h` | Header for fopencookie with cookie_io_functions_t |
| `compat/catalog.yaml` | Added fopencookie entry |

## Environment

- **IRIX Host**: `ssh edodd@192.168.0.81`
- **SGUG commands**: `/opt/sgug-exec <command>`
- **Cross toolchain**: `/opt/cross/bin/`
- **Cross GNU ld**: `/opt/cross/bin/mips-sgi-irix6.5-ld.bfd`
- **Staging**: `/opt/sgug-staging/usr/sgug/`

## Next Steps

**Primary Goal Achieved**: tdnf package manager runs on IRIX 6.5

| Package | Status | Notes |
|---------|--------|-------|
| zlib | DONE | 1.2.13 working |
| OpenSSL | DONE | 1.1.1g libs working (dlopen with RTLD_LAZY) |
| sqlite | DONE | 3.42.0 working (direct link; dlopen crashes) |
| curl | DONE | 8.2.1 working with HTTP and HTTPS |
| libgpg-error | DONE | 1.47 working |
| libassuan | DONE | 2.5.6 working |
| gpgme | DONE | 1.23.2 working |
| libsolv | DONE | 0.7.30 shared libs + RPM support working |
| rpm | DONE | 4.15.0 shared libs working |
| **tdnf** | **DONE** | **3.5.2 runs on IRIX** |

### Immediate Next Steps

1. **Configure tdnf on IRIX**
   - Create `/etc/tdnf/tdnf.conf` configuration file
   - Set up repository configuration in `/etc/yum.repos.d/`
   - Test basic operations: `tdnf repolist`, `tdnf search`, `tdnf info`

2. **Build RPM packages**
   - Package libsolv, tdnf, and dependencies as RPMs
   - Test rpm installation on IRIX
   - Create bootstrap repository

3. **Complete the dependency chain as RPMs**
   - All libraries need proper RPM packaging
   - Test full package management workflow

### Other Improvements
- Add global `install_cleanup` rule to remove .la files
- Integrate interpreter fix into mogrix toolchain templates
- Document the `/lib32/rld` interpreter requirement

## Known Issues

### Libtool Relinking
Cross-compiled executables linked against shared libs may have hardcoded build-directory
paths in RUNPATH. Workaround: set `LD_LIBRARY_PATH` on IRIX. Proper fix: use patchelf
or modify libtool behavior during install.

### IRIX pthread/dlopen Compatibility

**Important Discovery**: Cross-compiled libraries with pthread work fine when **directly linked**,
but crash when loaded via **dlopen()**.

```bash
# Direct linking works:
$ gcc -o test test.c -L/tmp -lpthread-test -Wl,-rpath,/tmp
$ ./test
pthread test in shared library
Result: 42

# dlopen crashes:
$ ./dlopen-test /tmp/libpthread-test.so
Memory fault (coredump)
```

**Impact**: Most packages use direct linking, so this is not a blocker. The dlopen issue
only affects:
- Plugin systems that load modules at runtime
- Certain test suites that use dlopen

**Workaround for dlopen-dependent packages**: Build with `--without-threads` where possible.

**Affected packages**: libxml2 (using --without-threads --without-tls for safety)

**Root cause investigation**: IRIX rld handles libraries without .liblist sections differently
for normal loading vs dlopen(). Native SGUG-RSE libraries also lack .liblist but work for
direct linking.

### libsolv POSIX.1-2008 Functions (SOLVED)

**Problem**: libsolv's libsolvext requires POSIX.1-2008 memory stream functions that IRIX lacks:
- `funopen()` (BSD custom stream I/O)
- `fopencookie()` (glibc custom stream I/O)
- `fmemopen()` (memory stream as FILE*)

**Solution**: Created a portable `fopencookie()` implementation based on the musl community's
`portable-fopencookie` project. The implementation uses pthreads and pipes to proxy I/O
between cookie functions and file descriptors.

Key files:
- `compat/stdio/fopencookie.c` - Pure C implementation (not C++17)
- `compat/stdio/fopencookie.h` - Header with `cookie_io_functions_t` type

The implementation was integrated into libsolv's `ext/` directory and cmake detects the
missing native functions and uses our portable version instead.

**Limitations of portable fopencookie**:
- No seeking support
- No r+/w+ mode support
- Callbacks run in separate thread (slightly different behavior than glibc)

These limitations are acceptable for libsolv's use case.

**irix-compat.h additions**:
- `isinf()` macro (for gpgme)
- `FNM_CASEFOLD` define (GNU fnmatch extension)
- `strcasestr()` inline implementation (GNU extension)
- `scandir()` and `alphasort()` declarations (hidden behind `_SGIAPI` when `_XOPEN_SOURCE` is defined)
