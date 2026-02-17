# Mogrix Rules Index

> **Do NOT read this entire file.** Grep for specific problem keywords. See `rules/GENERIC_SUMMARY.md` for what generic.yaml already handles.

---

## Per-Package Problem Reference

| Problem class | Symptoms / triggers | Rule mechanism | Rule location | Notes / constraints |
|---------------|---------------------|----------------|---------------|---------------------|
| Before you start | Stuck, confused, debugging | Read checklist | methods/before-you-start.md | Check SGUG-RSE, git history, clean build |
| Batch builds / agents | Building multiple packages, parallel agents | Wave orchestration | methods/task-tracking.md | Max 2-3 agents, report to disk, orchestrator owns INDEX.md |
| Upstream (non-Fedora) package | Package only in git/tarball, not FC40 | upstream: block + create-srpm | methods/upstream-packages.md | Generates SRPM from git clone or tarball download |
| Suite bundle | Combine multiple apps in one bundle | mogrix bundle pkg1 pkg2 --name X | methods/upstream-packages.md | Shared libs, single install/uninstall |
| Package-specific build deps | dbus-devel, gpgme-devel, etc. | drop_buildrequires | rules/packages/* | Only deps NOT in generic.yaml |
| Package-specific runtime deps | systemd-libs, rpm-libs, etc. | drop_requires | rules/packages/* | Cross-compiled pkgs need explicit deps |
| Autoconf cross-detect | configure misdetects headers/funcs | ac_cv_overrides | rules/packages/* | malloc/realloc already in generic |
| Missing libc functions | setenv, strsep, getline, asprintf | inject_compat_functions | rules/packages/* | Check compat/catalog.yaml for available |
| Missing POSIX.1-2008 | openat, fstatat, mkdirat, utimensat | inject_compat_functions | rules/packages/* | dicl/openat-compat.c has 17 *at funcs |
| Missing BSD functions | funopen, strlcpy, getprogname | inject_compat_functions | rules/packages/* | funopen works, fopencookie crashes |
| Libtool cross-detect | libtool says "unknown platform" | spec_replacements | rules/packages/* | Use fix-libtool-irix.sh after configure |
| CMake cross-compile | find_package fails, wrong paths | spec_replacements | rules/packages/* | Set CMAKE_FIND_ROOT_PATH, disable tests |
| RPM macro pollution | %configure clobbers cross flags | configure_flags: remove | rules/packages/* | Or use spec_replacements to fix |
| TLS not supported | __thread / _Thread_local, rld Fatal Error `__tls_get_addr` | prep_commands | rules/packages/* | Sed `#define _Thread_local __thread` → `#define _Thread_local` in headers. `-D` from CFLAGS WON'T WORK (source `#define` overrides). For autoconf `__thread` detection, some configure scripts (e.g. gdbm) unconditionally overwrite cache vars — `ac_cv_overrides` won't help; must sed configure directly. See gnutls.yaml, gdbm.yaml |
| Missing forkpty / PTY | forkpty undefined, tmux "server exited" immediately | prep_commands (create compat file) | rules/packages/tmux.yaml | IRIX has `_getpty()` not `forkpty()`. Must re-open PTY slave after `setsid()` WITHOUT `O_NOCTTY` — on SVR4/IRIX the first terminal open after setsid() sets controlling terminal. Without this, spawned shell has no ctty → exits → server exits cleanly (status 0). See tmux.yaml forkpty-unknown.c |
| libevent devpoll crash | Server crashes silently, log ends abruptly, `using libevent ... devpoll` | bundle wrapper env | mogrix/bundle.py | IRIX `/dev/poll` backend in libevent is broken — crashes inside event loop. Auto-detected: if `libevent*.so` in `_lib32/`, wrapper sets `EVENT_NODEVPOLL=1` to force `poll()` backend. Confirmed working with tmux. |
| Plugin dlopen path | App has plugins in non-standard path, commands missing | bundle wrapper env | mogrix/bundle.py | Set `WEECHAT_EXTRA_LIBDIR` etc. via `{extra_env_block}` in wrapper |
| gnutls CA certs | "certificate issuer unknown", TLS handshake fails | configure_flags: add | rules/packages/gnutls.yaml | `--with-default-trust-store-file` compiles CA path into libgnutls. Bundles include CA bundle at `etc/pki/tls/certs/ca-bundle.crt`. See [gnutls CA trust store](#gnutls-ca-trust-store) |
| extra_cflags dead code | Rule validated but never applied | use prep_commands or export_vars | rules/packages/* | `extra_cflags` in validator.py but NOT in engine.py. Workaround: `prep_commands` sed or `export_vars: CFLAGS` |
| vsnprintf pre-C99 | garbled output, truncation | inject_compat_functions: vasprintf | rules/packages/* | IRIX vsnprintf(NULL,0) returns -1 |
| Linker selection | .so crashes rld, bad relocations | export_vars: LD | rules/packages/* | GNU ld for -shared, LLD 18 for exe |
| Path conventions | /etc vs /usr/sgug/etc | spec_replacements | rules/packages/* | SGUG uses /usr/sgug prefix |
| Scriptlet failures | ldconfig, systemd macros | spec_replacements | rules/packages/* | Make ldconfig no-op via macros.ldconfig |
| Man page compression | .1.gz vs .1 in %files | spec_replacements | rules/packages/* | Cross-build doesn't compress |
| NLS/gettext | %find_lang fails | skip_find_lang: true | **generic.yaml** | Now generic — all packages skip %find_lang (locale files removed by cleanup) |
| Missing runtime deps | tdnf install says "not found" | add_requires | rules/packages/* | AutoReq disabled for cross-compile |
| Disable features | ldap, nls, tests not needed | configure_disable | rules/packages/* | Only features NOT in generic.yaml |
| Add configure flags | Need --with-X or custom flags | configure_flags: add | rules/packages/* | Adds flags to %configure |
| Unwanted install files | .la files, duplicate docs | install_cleanup | rules/packages/* | Commands run after %make_install |
| Legacy libdicl lines | CPPFLAGS/LIBS with libdicl | remove_lines | rules/generic.yaml | Already handled globally |
| GNU ld linker scripts | `INPUT(-lfoo)` text files as .so | spec_replacements | rules/packages/* | IRIX rld can't load linker scripts; replace with symlinks |
| `%zu` format specifier | SIGSEGV in printf/snprintf/fprintf | add_patch or prep_commands | rules/packages/* | IRIX libc is pre-C99; `%zu` corrupts varargs → crash. Use `%u` instead. See methods/irix-quirks.md |
| dlmalloc + libc strdup | SIGSEGV in free/realloc | inject_compat_functions: strdup | rules/packages/* | Packages with dlmalloc must also inject strdup/strndup compat |
| Spec conditionals | %if blocks for wrong platform | comment_conditionals | rules/packages/* | Comments out entire %if...%endif block |
| Extra compiler flags | Need -D or -I flags for build | extra_cflags | rules/packages/* | Added to CFLAGS in %build |
| Make environment | Need env vars for make | make_env | rules/packages/* | Exported before make commands |
| Skip man pages | Man pages cause install errors | skip_manpages | rules/packages/* | Removes man install from spec |
| Skip lang files | %files -f lang.txt fails | files_no_lang | rules/packages/* | Removes -f lang from %files |
| X11 path detection | configure AC_PATH_XTRA fails cross | spec_replacements | rules/packages/* | Remove --x-includes/--x-libraries, sysroot autodetects |
| IRIX X11R6.3 missing APIs | XICCallback, XSetIMValues, Xutf8TextListToTextProperty, XUTF8StringStyle undeclared | prep_commands | rules/packages/* | XICCallback→XIMCallback, XSetIMValues→no-op macro, Xutf8→Xmb, XUTF8→XCompound. See st.yaml |
| _XOPEN_SOURCE hides IRIX defs | struct winsize, TIOCSWINSZ, TIOCGWINSZ missing | prep_commands | rules/packages/* | Remove `-D_XOPEN_SOURCE=600`; `_SGI_SOURCE` provides all POSIX/XSI |
| _XOPEN_SOURCE hides wprintf | Bare `#define _XOPEN_SOURCE` (no value) → `_XOPEN_SOURCE+0` = 0 → wprintf/fwprintf hidden by `_WCHAR_CORE_EXTENSIONS_1` | prep_commands | rules/packages/lolcat.yaml | Change to `#define _XOPEN_SOURCE 500` to unlock C99 wide-char I/O. Bare define evaluates as 0 in IRIX preprocessor math. |
| Host build-tool cross-compile | `./fbc: cannot execute binary file` — build generates MIPS helper that must run on x86 host | spec_replacements | rules/packages/bc.yaml | Build fbc with host gcc + stub readline headers before cross-compile, neuter Makefile rule with sed. Pattern: create host-stubs dir, compile with `gcc -I host-stubs`, link with -lm only. |
| XA_UTF8_STRING missing (X11R6.3) | `XA_UTF8_STRING` undeclared — IRIX Xmu/Atoms.h is X11R6.3, lacks UTF8 atom | prep_commands | rules/packages/xclip.yaml | `#define XA_UTF8_STRING(dpy) XInternAtom(dpy, "UTF8_STRING", False)` after `#include <X11/Xmu/Atoms.h>`. Functionally identical to X11R6.4+ definition. |
| Makefile compat ordering | compat undefined symbols in plain Makefile builds | spec_replacements | rules/packages/* | Build compat archive BEFORE make, pass via LDFLAGS. Autotools injects between configure/make; Makefile must be explicit. See st.yaml, figlet.yaml |
| PKG_CONFIG_SYSROOT_DIR | pkg-config returns IRIX paths, not host staging paths | spec_replacements | rules/packages/* | `export PKG_CONFIG_SYSROOT_DIR="/opt/sgug-staging"` — already in `%configure` macro but NOT in custom Makefile builds |
| No TrueType fonts on IRIX | Xft/fontconfig apps fail "can't open font" | bundle fonts | mogrix/bundle.py | `_include_fonts()` copies TTFs from `fonts/`, adds relative `<dir>` to fonts.conf, creates monospace alias conf.d, sets FONTCONFIG_FILE in wrapper |
| setenv/unsetenv missing | link error: undefined symbol setenv/unsetenv | inject_compat_functions | rules/packages/* | IRIX libc has putenv but not setenv/unsetenv. Compat wraps putenv(). |
| CJK font dependencies | k14, taipei16 fonts not on IRIX | configure_disable or spec_replacements | rules/packages/* | --disable-kanji --disable-big5 --disable-greek |
| Unpackaged doc files | make install + %doc both install docs | install_cleanup | rules/packages/* | Remove from %{buildroot}%{_pkgdocdir} |
| AUTOPOINT=true | autoreconf fails without autopoint | spec_replacements | rules/packages/* | `AUTOPOINT=true autoreconf -vfi` |
| autoreconf overwrites prep | prep_commands modify configure, autoreconf regenerates it | spec_replacements | rules/packages/* | Inject sed AFTER autoreconf line, not in prep_commands |
| sockaddr_storage hidden | dicl-clang-compat sets _XOPEN_SOURCE=500, hides sockaddr_storage | header overlay | cross/include/dicl-clang-compat/sys/socket.h | Define struct + pad macros in overlay |
| C++ cmath errors | GCC 9 c++config.h enables C99 math TR1 | staging c++config.h patch | /opt/sgug-staging/.../bits/c++config.h | Comment out _GLIBCXX_USE_C99_MATH_TR1 |
| wchar_t C++ keyword | IRIX stdlib.h typedef clashes with C++ keyword | irix-cxx wrapper | cross/bin/irix-cxx | `-D_WCHAR_T` prevents typedef |
| Bashisms in specs | pushd/popd, brace expansion, substring | spec_replacements | rules/packages/* | pushd→cd, {a,b}→expand, ${c:0:7}→hardcode |
| update-alternatives | Doesn't exist on IRIX, blocks rpm install | spec_replacements | rules/packages/* | Drop Requires(post/preun/postun) |
| Compat header conflicts | Unconditional decls clash with static versions | ac_cv_overrides + inject_compat | rules/packages/* | Override ac_cv_func_X="yes" |
| R_MIPS_REL32 crashes rld | Function pointers in static data | add_patch | patches/packages/* | Dispatch functions (switch/strcmp) |
| R_MIPS_REL32 partial failure | IRIX rld silently skips some R_MIPS_REL32 relocs in large executables; superclass/method pointers in widget ClassRec structs left as NULL | add_source + prep_commands | patches/packages/nedit/fix_class_recs.c | `__attribute__((constructor))` function patches broken fields at startup. Declare extern class records as parent type (e.g. XmManagerClassRec) to avoid needing private headers. Check fields `== NULL` then assign GOT-resolved values. Small test binaries resolve same symbols fine — issue is binary-size-dependent. See nedit.yaml |
| Long double crash | IRIX MIPS n32 has no 128-bit long double | ac_cv_overrides | rules/packages/* | `ac_cv_type_long_double_wider: "no"` |
| IRIX libgen.so | dirname/basename in libgen.so, not libc | spec_replacements | rules/packages/* | `LIBS="$LIBS -lgen"` before %configure |
| Cross-build doc generation | Build tries to run MIPS binary for docs | spec_replacements | rules/packages/* | Override make vars to empty, remove doc from `all:` target |
| bcond flipping | Spec has inline %if/%else/%endif inside %configure continuation | spec_replacements | rules/packages/* | `%bcond_without X` → `%bcond_with X`; RPM resolves conditionals to %else branches. configure_flags:remove can't handle these. See gnutls.yaml |
| PRIdMAX/SCNd64 macros | Compile error: implicit/undeclared PRIdMAX, SCNd64, etc. | compat header | compat/include/mogrix-compat/generic/inttypes.h | 18 macros: PRId/PRIu/PRIx/PRIX/PRIoMAX, PRId/PRIu/PRIx/PRIXPTR, SCNd/SCNu/SCNx64, SCNd/SCNu/SCNxMAX |
| cmake %cmake/%cmake3 macro | FC40 spec uses %cmake/%cmake_build/%cmake_install (or cmake3 variants) | spec_replacements | rules/packages/* | Replace with raw `cmake -B _build -S .` + `make -C _build`. Use `CMAKE_SYSTEM_NAME=Linux` (NOT IRIX — cmake has no Platform/IRIX.cmake; Linux gives proper .so versioning + UNIX=1). Set cross tools, staging paths, `-DBUILD_SHARED_LIBS=ON` if needed. Use `make -C _build install` (NOT `cd _build && make install` — changes cwd for subsequent commands). See json-c.yaml, brotli.yaml, libwebp.yaml |
| select() duplicate symbol / struct timeval incomplete | dicl-clang-compat sys/select.h: forward-declared `struct timeval` caused incomplete-type errors | header include | cross/include/dicl-clang-compat/sys/select.h | Now includes `<sys/time.h>` directly when not yet included, providing full struct timeval + select() declaration. Fixes xclip and any code using `struct timeval tv;` after sys/select.h. |
| rld symbol resolution debug | Binary crashes silently or ldd SIGSEGV | `_RLD_ARGS="-log /tmp/rld.log"` | methods/irix-testing.md | Shows unresolvable symbols, soname mismatches. Check NEEDED sonames match installed .so files |
| ncurses ext-colors terminfo | SIGBUS on MIPS, garbage cols/lines values | spec_replacements | rules/packages/ncurses.yaml | `--disable-ext-colors`; ext-colors reads 16-bit terminfo fields as 32-bit |
| Explicit Provides required | rpm -Uvh fails with unresolved deps | spec_replacements | rules/packages/* | rpmmacros.irix sets AutoProv:no; add `Provides: libfoo.so.N` for each .so |
| Plugin dlopen symbol export | rld Fatal Error: unresolvable symbol in plugin .so | --dynamic-list via spec_replacements | cross/bitlbee-plugin-symbols.list | --export-dynamic exports ALL symbols, crashes rld (>468 entries). --dynamic-list exports only listed symbols. Create .list file with plugin's UND symbols: `readelf -sW plugin.so \| grep UND` |
| GNU symbol versioning crash | dlopen SIGSEGV in rld processing .gnu.version* sections | irix-ld filters --version-script | cross/bin/irix-ld | Qt5, GLib, and other libs pass `--version-script` via `-Wl,`; irix-ld strips it. Symbols still exported, just without version tags |
| .init_array ignored by rld | Static constructors silently don't run in shared libs | custom linker script + CRT objects | cross/irix-shared.lds, cross/crt/ | `.ctors` preserved by linker script; crtbeginS.o provides `_init` that walks .ctors array. All .so linked via GNU ld now uses this |
| rld re-encounter GOT crash | SIGBUS when DSO with large GOT is loaded as NEEDED after dlopen | `beqz` guard in crtbeginS.o | cross/crt/crtbeginS.S | rld re-runs DT_INIT on re-encounter, zeros global GOT entries above LOCAL_GOTNO ~128. Guard checks `__CTOR_END__` for NULL before deref. Threshold: LOCAL_GOTNO ≤103 OK, ≥140 crashes. |
| rld global GOT entry limit | SIGSEGV/SIGBUS in rld (PC=0x0FB6AA44) loading .so with large symbol table | `-Bsymbolic-functions` in irix-ld | cross/bin/irix-ld | rld has ~4370 global GOT entry limit per shared library. Global GOT = SYMTABNO - GOTSYM. Qt5Gui (5364) and Qt5Widgets (8567) exceed it. `-Bsymbolic-functions` moves defined FUNC symbols to local GOT, reducing count by 50-65%. Added unconditionally to irix-ld for all shared libraries. |
| dlmalloc shared lib crash | ABORT/SIGSEGV cross-heap corruption when .so uses malloc/free across library boundaries | dlmalloc in exe only, not .so | mogrix/engine.py, cross/bin/irix-ld | `-Bsymbolic-functions` binds each .so's dlmalloc locally → private heaps → cross-heap free() abort. Fix: dlmalloc.o linked into executables only; .so leaves malloc undefined → rld resolves to exe's single heap. See [detailed section below](#dlmalloc-shared-library-crash-cross-heap-corruption) |
| dlmalloc + IRIX native libs | SIGSEGV when using IRIX native .so files (Motif libXm.so, etc.) with dlmalloc exe | `export_vars: {MOGRIX_NO_DLMALLOC: "1"}` | rules/packages/nedit.yaml | IRIX native .so files have their own malloc from libc.so.1. Our exe's dlmalloc can't override it (IRIX rld doesn't do symbol interposition for pre-linked native libs). Use `MOGRIX_NO_DLMALLOC=1` in export_vars to skip dlmalloc.o. |
| Bundle wrapper missing /usr/lib32 | rld fails to find IRIX system .so files (libXm.so.2, libXpm.so.2) — "unresolvable symbol" or SIGSEGV | Fixed in bundle.py wrapper templates | mogrix/bundle.py | Bundle wrappers set `LD_LIBRARYN32_PATH=$dir/_lib32` which OVERRIDES default rld search path. IRIX native libs in `/usr/lib32` become invisible. Fix: append `:/usr/lib32` to the path. Applied to both WRAPPER_TEMPLATE and SBIN_WRAPPER_TEMPLATE. |
| CRT symbol visibility | .so picks up wrong `__CTOR_END__` / `_init` from other .so → SIGSEGV at garbage PC (e.g. 0x24020000) when many DSOs loaded | `.hidden` in crtbeginS.S/crtendS.S + crt-hide.ver | cross/crt/crtbeginS.S, cross/crt/crtendS.S | `__CTOR_END__`, `__DTOR_END__`, `__do_global_ctors_aux`, `_init` now `.hidden` to prevent cross-library symbol interposition. **CONFIRMED**: this was the sole cause of the GTK3 crash (libgmodule + libharfbuzz pre-fix → crash; post-fix → works). ~65 libraries in staging still have exposed `_init` from pre-fix builds; they'll be fixed on rebuild. |
| gnutls CA trust store | "peer's certificate issuer is unknown" / TLS handshake fails | configure_flags: add | rules/packages/gnutls.yaml | `--with-default-trust-store-file=%{_sysconfdir}/pki/tls/certs/ca-bundle.crt`. Without it, gnutls has no compiled-in CA path. See [detailed section below](#gnutls-ca-trust-store) |
| Circular NEEDED crashes rld | SIGSEGV (PC=0x0) during dependency resolution with circular NEEDED entries | Break the cycle | rules/packages/freetype.yaml | freetype↔harfbuzz circular dep (freetype NEEDED harfbuzz, harfbuzz NEEDED freetype) crashes rld during single-pass resolution combined with deep dep trees. Fix: `--without-harfbuzz` for freetype. Harfbuzz still works standalone. |
| rld deep dependency tree crash | dlopen of library with 10+ transitive deps crashes rld | Sequential preloading | Bundle wrappers | Pre-load heavy deps (Qt5Core, harfbuzz) individually via sequential dlopen before loading the leaf library (Qt5Gui). Bundle wrappers implement the preload chain. |
| IRIX rld strict UND resolution | "unresolvable symbol" even when symbol exists in already-loaded DSO | Add explicit NEEDED | rules/packages/libxcb.yaml | IRIX rld does NOT search already-loaded DSOs for UND symbols unless there's a direct NEEDED chain. libxcb had UND `_XLockMutex_fn` from libX11 but no NEEDED libX11. Fix: `-lX11` in LDFLAGS. |
| IRIX lacks endian.h | `#include <endian.h>` or `<sys/endian.h>` fails; no byte-order macros | **GLOBAL: dicl-clang-compat/endian.h** | cross/include/dicl-clang-compat/endian.h | Provided globally — no per-package fix needed. Defines BYTE_ORDER, htobe16/32/64, htole16/32, be16toh/32toh/64toh, le16toh/32toh and bswap helpers. MIPS big-endian: htobe/betoh are no-ops. |
| IRIX struct dirent has no d_type | `d_type`, `DT_REG`, `DT_DIR` undeclared; compile error | prep_commands or add_patch | rules/packages/telescope.yaml | BSD/Linux extension not in IRIX `struct dirent`. Replace `d_type` checks with `stat()` calls and `S_ISREG()`/`S_ISDIR()` macros. |
| IRIX clock_gettime crashes | SIGSEGV or undefined CLOCK_MONOTONIC/CLOCK_REALTIME | prep_commands or add_patch | rules/packages/telescope.yaml | IRIX has `clock_gettime` in libc but no POSIX `CLOCK_*` constants defined. Calling with any value crashes. Replace with `gettimeofday()`. |
| IRIX lacks timersub macro | `timersub()` undeclared | **GLOBAL: dicl-clang-compat/sys/time.h** | cross/include/dicl-clang-compat/sys/time.h | Provided globally — no per-package fix needed. Also provides timeradd, timercmp, timerclear, timerisset. |
| IRIX scandir selector signature | `const struct dirent *` vs `dirent_t *` mismatch | prep_commands or add_patch | rules/packages/* | Linux: `int (*)(const struct dirent *)`. IRIX: `int (*)(dirent_t *)` (no const). Remove const from selector function signatures. |
| IRIX _SGIAPI is a macro expression | `#ifndef _SGIAPI` always FALSE (macro is defined as an expression) | Use `#if !_SGIAPI` instead | rules/packages/alpine.yaml | `_SGIAPI` is `#define _SGIAPI (expr)` in standards.h — always defined, evaluates to 0 or 1. Use `#if !_SGIAPI` not `#ifndef _SGIAPI`. Same applies to `_XOPEN4`, `_POSIX90`, etc. |
| NEVER #define _SGIAPI to a literal value | `blkcnt64_t` unknown type, `struct stat64` visible when it shouldn't be, `_LFAPI` unexpectedly TRUE | Use `push_macro`/`pop_macro` to save/restore `_SGIAPI` | `cross/include/dicl-clang-compat/sys/socket.h` | `_SGIAPI` is a complex expression-macro with `defined()` operators from `standards.h`. If you `#undef _SGIAPI` then `#define _SGIAPI 1`, you destroy the expression and make it always TRUE, which enables `_LFAPI`, exposing `struct stat64`/`blkcnt64_t`. MUST use `#pragma push_macro("_SGIAPI")` / `#pragma pop_macro("_SGIAPI")` instead. Same principle applies to `_LFAPI`, `_POSIX93`, `_XOPEN5`, and all IRIX standards.h expression-macros. |
| IRIX scandir hidden by _XOPEN_SOURCE | scandir/alphasort undeclared when `_XOPEN_SOURCE` is defined | Add guarded prototypes with `#if !_SGIAPI` | rules/packages/alpine.yaml | `_XOPEN_SOURCE=1` makes `_NO_XOPEN4=FALSE` which makes `_SGIAPI=FALSE`, hiding scandir in dirent.h. Add explicit prototypes guarded by `#if !_SGIAPI` to avoid conflict when _SGIAPI is true. |
| IRIX lacks open_memstream | `open_memstream()` undeclared / link error | **inject_compat_functions: open_memstream** | compat/stdio/open_memstream.c | Compat function available. Uses funopen() internally — must also inject funopen. Header declaration in mogrix-compat/generic/stdio.h (auto-available). |
| IRIX lacks dprintf | `dprintf(fd, fmt, ...)` undeclared / link error | **inject_compat_functions: dprintf** | compat/stdio/dprintf.c | Compat function available. Also provides vdprintf. Uses dup()+fdopen()+vfprintf(). Header declaration in mogrix-compat/generic/stdio.h (auto-available). |
| IRIX lacks IOV_MAX | `IOV_MAX` undeclared | **GLOBAL: dicl-clang-compat/limits.h** | cross/include/dicl-clang-compat/limits.h | Provided globally — no per-package fix needed. Defined as 1024 (POSIX typical). |
| MAP_ANON not on IRIX | `MAP_ANON` / `MAP_ANONYMOUS` undeclared | prep_commands or compat header | rules/packages/* | IRIX has no `MAP_ANON`/`MAP_ANONYMOUS`. Anonymous mmap must use `/dev/zero` fd with `MAP_PRIVATE`. Define `MAP_ANON` as 0 and open `/dev/zero` for the fd argument. |
| IRIX getentropy via /dev/urandom | `getentropy()` / `getrandom()` link error | prep_commands or add_patch | rules/packages/* | IRIX has no `getentropy`/`getrandom` syscall. `/dev/urandom` works and is available. Replace calls with `open("/dev/urandom") + read()`. |
| libretls exports compat functions | configure falsely detects `reallocarray`, `strlcpy`, `strlcat` as available | ac_cv_overrides | rules/packages/* | libretls.so exports these functions; configure links against it and thinks system libc has them. Fix: `ac_cv_func_reallocarray: "no"`, `ac_cv_func_strlcpy: "no"`, etc. to force internal implementations. |
| Cross-compile HOSTCC pattern | Build fails running MIPS binary on build host | spec_replacements or make_env | rules/packages/* | Build-time code generators (pagebundler, etc.) need host compiler. Pass `HOSTCC=gcc` to configure. Build system must use `BUILD_CC`/`HOSTCC` for generators and `CC` for target objects. See telescope.yaml. |
| Bundled library cross-compilation | Bundled library compiles for host instead of target | spec_replacements or prep_commands | rules/packages/* | Libraries bundled in source (e.g. libgrapheme in telescope) default to host CC. Must fix their config.mk/Makefile to use cross-compiler for target objects and host compiler for generators. |
| AC_CHECK_FILES cross-compile | `cannot check for file existence when cross compiling` | ac_cv_overrides | rules/packages/* | AC_CHECK_FILES checks host filesystem; provide `ac_cv_file_<path>: "no"` for each file (underscores for path separators). See cmatrix.yaml |
| C++ va_list type mismatch | `conflicting types for 'vfprintf'`, ambiguous overload in C++ | **GLOBAL: dicl-clang-compat/stdarg.h** | cross/include/dicl-clang-compat/stdarg.h | IRIX declares va_list as `char*`; clang `__builtin_va_list` is `void*`. C++ doesn't allow implicit void*→char*. Fix: stdarg.h defines `va_list = char*` in C++ mode with type-punning va_* macros |
| Static select() duplicate symbol | `duplicate symbol: select` in static archives | **GLOBAL: dicl-clang-compat/sys/time.h** | cross/include/dicl-clang-compat/sys/time.h | IRIX sys/time.h provides `static int select()` wrapper when XOPEN flags active. Each .o gets a copy → lld duplicate symbol. Fix: force `_NO_XOPEN4=1 _NO_XOPEN5=1` before including real header |
| Staging/chroot library mismatch | SIGABRT with NO error message after library loading succeeds; all steps work until first complex operation (e.g. GTK3 show_all). Par trace shows abort() immediately after fontconfig/pango init with zero diagnostic output | Redeploy all staging libs to chroot | methods/irix-testing.md | After rebuilding libraries via `mogrix build --cross` + `mogrix stage`, the IRIX chroot still has OLD versions. Must copy updated .so files from `/opt/sgug-staging/usr/sgug/lib32/` to the chroot. Compare file sizes to detect mismatches. **Root cause of session 58 GTK3 crash**: pango/cairo Feb 10 in chroot vs Feb 16 in staging. Symptom was indistinguishable from heap corruption. |
| FONTCONFIG_FILE for non-chroot apps | fontconfig reads SGUG-RSE's `/usr/sgug/etc/fonts/fonts.conf` (20+ conf.d includes) → SIGABRT. Or reads wrong font dirs | `FONTCONFIG_FILE` env var | Bundle wrappers | When running cross-compiled apps outside the chroot (direct IRIX console with LD_LIBRARYN32_PATH), fontconfig finds SGUG-RSE's config at its compiled-in path. Our fontconfig 2.15.0 is incompatible with SGUG-RSE's config format. Fix: set `FONTCONFIG_FILE` pointing to a standalone config with just `<dir>/usr/lib/X11/fonts/Type1</dir>` and `<cachedir>/tmp/fc-cache</cachedir>`. Bundle wrappers should set this automatically. |
| gnulib SIG_ATOMIC_MAX missing | `SIG_ATOMIC_MAX undeclared` in gnulib-generated code | **GLOBAL: compat stdint.h** | compat/include/mogrix-compat/generic/stdint.h | gnulib's include_next chain blocks IRIX stdint.h via `__STDINT_H__` guard. Fix: define SIG_ATOMIC_MIN/MAX in compat header |
| `%{__global_ldflags}` undefined | Literal `%{__global_ldflags}` string passed to compiler/linker | spec_replacements | rules/packages/* | Not defined in rpmmacros.irix. Remove from any Makefile build commands. See figlet.yaml, sl.yaml |
| GnuPG --with-lib*-prefix | `gpg-error-config: not found` during configure | configure_flags: add | rules/packages/* | GnuPG ecosystem uses `*-config` scripts not in build PATH. Add `--with-libgpg-error-prefix=/opt/sgug-staging/usr/sgug` etc. Do NOT use `--with-npth-prefix` (FC40 npth lacks npth-config; use NPTH_CONFIG export_var) |
| drop_subpackages orphaned scriptlets | `%post <subpkg>` / `%postun <subpkg>` left behind after subpackage dropped | spec_replacements | rules/packages/* | drop_subpackages removes %package/%description/%files but NOT scriptlets. Manually remove %post/%postun for dropped subpackages. See cmatrix.yaml |
| Clang 18 NULL int-conversion | `incompatible integer to pointer conversion returning 'long'` | CFLAGS `-Wno-int-conversion` | rules/packages/* | IRIX `NULL` is `0L` (long), not `((void *)0)`. Clang 18 hard-errors on implicit long-to-pointer conversion even without `-Werror`. Affects code using comma operator with NULL: `return (func(), NULL)`. Add `-Wno-int-conversion` to CFLAGS. See mksh.yaml |
| Custom build script cross-compile | Build uses custom Build.sh (not autotools/cmake) | spec_replacements + export CC/CFLAGS | rules/packages/* | Set `CC="%{__cc}"`, `CFLAGS="%{optflags}"`, `LDFLAGS`, `TARGET_OS` via export in spec_replacement. Build.sh scripts typically use compile+link tests (no execution), so cross-compilation works if CC is set. Skip any step that runs the just-built binary on the host. See mksh.yaml |
| pselect() undeclared | `use of undeclared identifier 'pselect'` | compat header + compat lib | compat/include/mogrix-compat/generic/sys/select.h, compat/stdlib/pselect.c | IRIX has select() but not pselect(). Compat header wraps sys/select.h with pselect declaration. Implementation wraps select()+sigprocmask(). pselect.o added to staging libmogrix_compat.a |
| posix_spawn link errors | `undefined symbol: posix_spawn`, posix_spawnattr_*, posix_spawn_file_actions_* | compat lib | compat/runtime/spawn.c, compat/include/mogrix-compat/generic/spawn.h | IRIX lacks posix_spawn. spawn.o in staging libmogrix_compat.a. Auto-detected by compat injector for standard builds. For custom build systems, link `-lmogrix_compat` |
| Link order: -l before objects | Linker errors for symbols from static archives | move -l flags to after $libs | rules/packages/* | Custom build systems (like ninja configure.py) put LDFLAGS before objects in link command. Static archive `-l` flags in LDFLAGS get discarded because no references exist yet. Fix: put `-L` paths in LDFLAGS, sed the link rule to append `-l` flags after `$libs`. See ninja-build.yaml |
| drop_subpackages unexpanded macros | drop_subpackages doesn't match names with unexpanded RPM macros | glob pattern | rules/packages/* | `python%{python3_pkgversion}-%{name}` won't match `python3-brotli`. Use glob: `"python*"`. See brotli.yaml |
| ninja-build bootstrap cross-compile | configure.py --bootstrap tries to run MIPS binary | prep_commands sed | rules/packages/ninja-build.yaml | configure.py runs `subprocess.check_call(rebuild_args)` after bootstrap. Sed to `pass # skip: cross-compiled binary`. Also needs `--force-pselect` (no ppoll), getloadavg stub. See ninja-build.yaml |

---

## Invariants (Settled Facts)

Only facts NOT already covered in the Problem Reference table above. For package-specific patterns, check the table first.

### IRIX Platform

| Fact | Implication | Reference |
|------|-------------|-----------|
| IRIX chroot doesn't isolate | Binaries see base system paths | methods/irix-testing.md |
| brk() heap limited to 176MB | libpthread at 0x0C080000 blocks growth; dlmalloc uses mmap (1.2GB) | methods/irix-address-space.md |
| Volatile fptr initializers crash | `static volatile fptr = memset;` relocation fails on rld | compat/string/explicit_bzero.c |
| IRIX native tar drops long paths | Silently drops >100 char paths; no `-z` flag. Use `gtar` from chroot | mogrix/bundle.py |
| Shell scripts must use full paths | DIDBS/SGUG-RSE in user PATH shadow IRIX system commands. Bare `tar`/`mkdir` resolve to wrong binary. Self-extracting .run scripts use `/sbin/tar`, `/sbin/mkdir`, `/usr/sbin/gzcat`, `/bin/tail`, `/bin/pwd` — never bare names | mogrix/bundle.py SELF_EXTRACTING_TEMPLATE |
| IRIX `cp -r` breaks on symlinks | Dereferences relative symlinks. Use `tar cf - \| tar xf -` pipe | IRIX host deployment |
| X11 .pc files for native libs | Handcrafted x11.pc, xext.pc, etc. in staging for pkg-config | /opt/sgug-staging/.../pkgconfig/ |
| Old /opt/cross/bin/irix-ld broken | Produces MIPS_OPTIONS tags → rld crash. Always use staging irix-ld | /opt/sgug-staging/usr/sgug/bin/irix-ld |
| Staging ≠ chroot until deployed | `mogrix stage` only updates `/opt/sgug-staging/`. Chroot at `/opt/chroot/` is a SEPARATE copy. After rebuilding ANY library, must scp updated .so to chroot. Mismatch causes silent SIGABRT (no error message). Compare file sizes: `wc -c` staging vs `wc -c` chroot | scp or `irix_copy_to` |
| SGUG-RSE paths leak at runtime | Non-chroot binaries with `LD_LIBRARYN32_PATH` still read hardcoded SGUG-RSE paths for GIO modules (`/usr/sgug/lib32/gio/modules/`), gdk-pixbuf cache, fontconfig. Set `FONTCONFIG_FILE`, `GIO_MODULE_DIR`, `GDK_PIXBUF_MODULE_FILE` as needed | Bundle wrappers |
| `-z separate-code` → 3 LOAD segments | IRIX rld only handles 2-segment (RE+RW) shared libs. 3 segments (R+RE+RW) cause SIGSEGV during rld init. irix-ld now uses irix-shared.lds to prevent this. Pre-fix libs must be rebuilt. Verify: `llvm-readelf-18 -l foo.so \| grep "^  LOAD"` must show exactly 2 | cross/bin/irix-ld, cross/lib32/irix-shared.lds |
| IRIX libc/libstdc++ word-aligned overread | SIGSEGV in memcmp, strcmp, _Hash_bytes at runtime (fault addr above BSS/brk) | safe_mem.o in irix-ld | cross/lib/safe_mem.c, cross/bin/irix-ld | IRIX optimized mem functions read in 4/8-byte chunks, overread past buffer end into unmapped pages. Byte-by-byte replacements auto-linked into all executables. Discovered on cmake 3.28.2 (279 static constructors). Shared libs get the override via dynamic symbol resolution. |
| `__rld_obj_head` must be in .dynsym | rld "unresolvable symbol" crash when loading libC.so.2 or libGLcore.so | cross/bin/irix-ld `--export-dynamic-symbol=__rld_obj_head` | crt1.o defines it as COMMON but LLD doesn't export COMMON to .dynsym by default. Any binary loading libC.so.2 or libGLcore.so (GL apps, C++ apps) needs this. Fixed in irix-ld for all executables. Use `par -a 300` to capture full rld error messages. |
| IRIX GL architecture | libGL.so exports GLX + `__glx_dispatch` table; libGLcore.so (hw-specific) exports all gl* functions as 12-byte dispatch stubs | `/usr/lib32/libGL.so`, `/usr/lib32/libGLcore.so` |
| Clang 16 `.cpsetup` N32 bug | Fixed in LLVM 18; use `-fno-integrated-as` if stuck on 16 | rules/packages/libffi.yaml |

### Engine Bugs & Gotchas

| Bug | Workaround | Reference |
|-----|------------|-----------|
| `extra_cflags` / `skip_manpages` / `make_env` dead code | Use prep_commands or export_vars instead | mogrix/rules/validator.py |
| `install_cleanup` misplacement | Misplaces if `%triggerpostun` precedes `%files`. Use spec_replacements | mogrix/rules/engine.py |
| `remove_conditionals` can't nest | Matches first `%endif`. Use bcond flipping | mogrix/rules/engine.py |
| `drop_subpackages` must be inside `rules:` | Only `add_patch`/`add_source` work at top-level | rules/packages/*.yaml |
| `drop_subpackages` ignores scriptlets | Doesn't remove %post/%postun. Use spec_replacements | mogrix/emitter/spec.py |
| `inject_compat_functions` affects HOST link | Bootstrap packages (flex) build HOST tools; -lmogrix-compat breaks them | rules/packages/flex.yaml |
| Perl `drop_subpackages` glob trap | `"*"` matches `-f` in `%files -f`. Use `"[A-Za-z]*"` | rules/packages/perl.yaml |
| cmake finds staging binaries | Set `-DCMAKE_MAKE_PROGRAM=/usr/bin/make` | rules/packages/weechat.yaml |
| batch.py rule passthrough | Must pass export_vars, skip_find_lang, skip_check, install_cleanup, spec_replacements | mogrix/batch.py |

### Bundle & Wrapper

| Fact | Implication | Reference |
|------|-------------|-----------|
| Shell wrapper recursion | Bundled dirname/pwd/basename cause fork bomb via $PATH. Use absolute `/bin/` paths | mogrix/bundle.py |
| LD_LIBRARYN32_PATH contamination | Wrappers must set fresh, not prepend to existing | mogrix/bundle.py |
| SSL_CERT_FILE is OpenSSL-only | gnutls ignores it; use app-specific CA config | mogrix/bundle.py |
| Bundle must include libz.so | IRIX ships zlib 1.1.4; modern libpng (1.6+) needs zlib 1.2+. "libpng error: bad parameters to zlib" → abort. Bundle libz.so from staging | xscreensaver-gl-hacks bundle |

## Anti-Patterns

| Anti-Pattern | Why It's Wrong | Do This Instead |
|-------------|----------------|-----------------|
| Inline C code in prep_commands (heredocs, printf chains generating .c/.h files) | YAML escaping corrupts C silently; can't review, test, or diff standalone | Put C files in `patches/packages/<name>/`, reference via `add_source`, copy with `cp %{_sourcedir}/file.c dest.c` in prep_commands |
| Fixes applied outside mogrix rules (manual sed, staging edits) | Knowledge lost — next rebuild fails the same way | Store in package YAML or generic.yaml |
| Rebuilding staging libs without redeploying to chroot | Silent SIGABRT at runtime, looks like heap corruption or library bug. Hours of debugging for a simple sync issue | After `mogrix stage`, always compare staging vs chroot sizes and redeploy changed .so files |
| Duplicating generic.yaml rules in package YAML | Double-application or silent conflict | Check generic.yaml first (see top of this file) |

## File Locations

| Category | Path | Contents |
|----------|------|----------|
| Generic rules | rules/generic.yaml | Applied to ALL packages |
| Class rules | rules/classes/*.yaml | Shared rules for package groups |
| Package rules | rules/packages/*.yaml | Per-package overrides (129 packages) |
| Source checks | rules/source_checks.yaml | IRIX source pattern definitions |
| Compat functions | compat/catalog.yaml | Function registry + source patterns |
| Compat sources | compat/string/, compat/stdio/, compat/stdlib/, compat/dicl/, compat/malloc/ | Implementation files |
| Compat headers | compat/include/mogrix-compat/generic/ | Header wrappers (pty.h for openpty) |
| Spec templates | specs/templates/ | Build-system spec templates (autoconf, cmake, meson, makefile) |
| Hand-written specs | specs/packages/ | Package-specific spec overrides |
| Bundle fonts | fonts/ | TTF fonts for X11 bundles (Iosevka Nerd Font) |
| X11 .pc files | /opt/sgug-staging/.../pkgconfig/ | x11.pc, xext.pc, xproto.pc, renderproto.pc |
| Patches | patches/packages/*/ | Source patches by package |
| rpmlint config | rpmlint.toml | IRIX-specific rpmlint filters |
| Source analyzer | mogrix/analyzers/source.py | Ripgrep-based source scanner |
| Rule auditor | mogrix/analyzers/rules.py | Duplication detection across packages |
| Spec validator | mogrix/validators/spec.py | Specfile structural validator |
| Test results | test-results/*.json | Bundle test results from mogrix-test MCP server |
| Test server | tools/mogrix-test-server.py | MCP server for IRIX testing (test_bundle, test_binary, check_deps, par_trace, screenshot) |
| Methods | rules/methods/*.md | Process documentation (see CLAUDE.md for index) |

## Rule Hierarchy

Rules are applied in order: **generic → class → package**. Each layer adds to or overrides the previous.

- **generic.yaml** — Universal IRIX fixes (all packages get these)
- **classes/*.yaml** — Shared patterns for groups of packages (opt-in via `classes:` in package yaml)
- **packages/*.yaml** — Package-specific overrides

**Adding a class:** Create `rules/classes/<name>.yaml` with a `class:` key and `rules:` section. Packages opt in by adding `classes: [<name>]` to their top-level yaml.

**Available classes:**

| Class | Purpose | Packages |
|-------|---------|----------|
| `nls-disabled` | Disables NLS, skips %find_lang, removes lang file refs | bison, coreutils, diffutils, findutils, flex, gawk, grep, gzip, make, nano, sed, tar |

**Auditing for elevation:** Run `mogrix audit-rules` to detect duplicated rules across packages and flag candidates for promotion to class or generic level. Top candidates (2026-02-11): strdup(69), strndup(48), getline(24), getopt_long(21), rm *.la(19), asprintf(14). Requires testing before bulk promotion.

## Smoke Tests

Package YAMLs can include a `smoke_test` field documenting how to verify the package works on IRIX. This is metadata only — the engine ignores it. Humans and agents can read it to know what to test.

```yaml
smoke_test:
  - command: "/usr/sgug/bin/bash -c 'echo $BASH_VERSION'"
    expect: "5.2.26"
  - command: "/usr/sgug/bin/bash -c 'for i in 1 2 3; do echo $i; done'"
```

| Field | Required | Description |
|-------|----------|-------------|
| `command` | Yes | Command to run via MCP `irix_exec` or `sgug-exec` |
| `expect` | No | Substring expected in stdout. If omitted, just check exit code 0. |

The `smoke_test` field is top-level (sibling of `package:` and `rules:`), not nested under `rules:`.

---

## Skipped Packages (Won't Work on IRIX)

| Package | Version | Reason |
|---------|---------|--------|
| gdb | 14.2 | All IRIX native debug support (irix5-nat.c, solib-irix.c) removed. Can't debug processes. SGUG-RSE used 7.6.2 (last version with IRIX support) with 31KB of patches. |
| htop | * | Needs Linux /proc backend |
| openjpeg2 | * | No SRPM available |

Before porting a package, check `rules/methods/before-you-start.md` section 0 — does the package actually need OS-specific support that IRIX lacks?

---

## Source Analysis

`mogrix analyze` and `mogrix convert` scan source tarballs for IRIX-incompatible patterns.

**Adding a new check:** Edit `rules/source_checks.yaml` or add `source_patterns` to functions in `compat/catalog.yaml`. No code changes needed.

**Two pattern sources:**
1. `rules/source_checks.yaml` — Issue-level patterns (need patches/overrides): `%zu`, `__thread`, volatile fptrs, epoll, inotify, getrandom, etc.
2. `compat/catalog.yaml` `source_patterns` — Function-level patterns (need compat injection): strdup, fopencookie, getline, asprintf, etc.

**Behavior differs by command:**
- `mogrix analyze` — Shows ALL findings (full triage for new packages)
- `mogrix convert` — Cross-references with rules, shows only UNHANDLED findings

---

## X.org Extension Libraries (Cross-Compilation)

**Problem: `xorg_cv_malloc0_returns_null` configure test fails during cross-compilation**
X.org libraries use their own `XORG_CHECK_MALLOC_ZERO` macro (not standard autoconf `ac_cv_func_malloc_0_nonnull`). It tries to run a test program, which fails when cross-compiling.
**Fix:** Add `ac_cv_overrides: { xorg_cv_malloc0_returns_null: "no" }` to the package YAML. Already applied to: libXrender, libXft, libXi, libXinerama, libXrandr.

**Problem: autoconf 2.71 `cannot make irix-cc report undeclared builtins`**
Host autoconf 2.71's new configure check fails with our cross-compiler. Affects all packages using autoreconf.
**Fix:** Don't use autoreconf for X11 libs — use the shipped configure script. For headers-only packages (xorgproto), skip configure entirely.

**Problem: `_XEatDataWords` undeclared (IRIX sysroot Xlibint.h)**
IRIX's native libX11 is pre-1.6, missing `_XEatDataWords`. It's just `_XEatData(dpy, n * 4)`.
**Fix:** Add patch with `#ifndef _XEatDataWords` / `static inline void _XEatDataWords(...)` shim. Needed by: libXfixes, libXi, libXinerama, libXrandr.

**Problem: Missing `Xge.h` (Generic Event Extension) for libXi**
IRIX's X11 predates X11R7.5. libXi's XI2 code needs `XGenericEvent`/`XGenericEventCookie` types and functions (`XESetWireToEventCookie`, `XESetCopyEventCookie`, `_XUnknownNativeEvent`).
**Fix:** Create stub `Xge.h` header + `xge_compat.c` with safe no-op implementations (IRIX X server doesn't generate Generic Events). See `patches/packages/libXi/`.

## Meson Cross-Builds (GTK3 and similar)

**Problem: irix-cc doesn't expand response files (@file) from meson**
Meson uses response files for large link commands. Our `irix-cc` shell wrapper doesn't expand `@file` syntax, so `-Wl,` flags and `-shared` flag are invisible to the wrapper's filtering logic.
**Fix:** Added response file expansion to `irix-cc` — detects `@file` args, reads and expands their contents (stripping single quotes), then re-sets positional params. See `cross/bin/irix-cc`.

**Problem: IRIX XEvent union lacks `xcookie` member (pre-X11R7.5)**
GTK3's X11 backend accesses `xevent->xcookie` extensively (gdkdevicemanager-xi2.c, gdkdisplay-x11.c, gdkeventsource.c, gdkwindow-x11.c, gtkdnd.c). IRIX's XEvent union predates GenericEvent.
**Fix:** prep_commands: (1) add `#include <X11/extensions/Xge.h>` to each file, (2) replace `&var->xcookie` → `(XGenericEventCookie*)var`, (3) replace `var->xcookie.member` → `((XGenericEventCookie*)var)->member`, (4) replace standalone `var->xcookie` → `(*((XGenericEventCookie*)var))`. Xge.h also provides inline stubs for `XGetEventData`/`XFreeEventData`/`XSetIMValues`.

**Problem: No atk-bridge-2.0 (AT-SPI2 accessibility) on IRIX**
GTK3 unconditionally includes `atk-bridge.h` under `#ifdef GDK_WINDOWING_X11`.
**Fix:** (1) Make dep `required: false` in meson.build, (2) Guard pkgconfig reference, (3) sed out the include and `atk_bridge_adaptor_init` call.

**Problem: GTK3 requires native glib tools (glib-compile-resources, gdbus-codegen, xmllint)**
These are build-machine tools not available on the cross-compile host.
**Fix:** Extract from Ubuntu debs using `apt-get download` + `dpkg-deb -x` to `/tmp/`. Symlink/copy to `cross/native-tools/`. gdbus-codegen needs its Python module path patched to point to the extracted location.

---

## Git / Makefile-based packages

**Problem: IRIX unistd.h includes getopt.h → struct option conflict**
IRIX `<unistd.h>` (line 392) includes `<getopt.h>`, which pulls in our compat getopt.h defining `struct option` (POSIX: name, has_arg, flag, val). Apps like git that define their own `struct option` with different fields get redefinition errors.
**Fix:** Use `_MOGRIX_NO_GETOPT_STRUCT_OPTION` guard. Add `-D_MOGRIX_NO_GETOPT_STRUCT_OPTION` to CFLAGS. Our compat getopt.h wraps struct option/getopt_long in `#ifndef _MOGRIX_NO_GETOPT_STRUCT_OPTION`.

**Problem: Makefile-based builds don't link compat library**
Mogrix injects compat via `export LIBS="-L$COMPAT_DIR -lmogrix-compat"`. Works for autoconf. Makefile-only builds (like git) ignore `LIBS`.
**Fix:** Use the package's own Makefile variables. Git uses `EXTLIBS += -L./mogrix-compat -lmogrix-compat`. Also check if the package has built-in compat (git has NO_MKDTEMP, NO_SETENV, NO_UNSETENV, NO_MEMMEM, NO_STRCASESTR, HAVE_GETDELIM, etc.).

**Problem: Linux auto-detection in cross builds (sysinfo, CLOCK_MONOTONIC, sync_file_range, PROCFS)**
Git's `config.mak.uname` detects Linux and adds `-DHAVE_SYSINFO` directly to BASIC_CFLAGS (not through ifdef). Setting `HAVE_SYSINFO=` in config.mak doesn't help.
**Fix:** prep_command: `sed -i '/HAVE_SYSINFO\|PROCFS_EXECUTABLE_PATH\|HAVE_PLATFORM_PROCINFO/d' config.mak.uname`. For `ifdef`-based vars, override with empty: `HAVE_CLOCK_GETTIME =`, `HAVE_CLOCK_MONOTONIC =`, `HAVE_SYNC_FILE_RANGE =`.

**Problem: IRIX atfork_parent/atfork_prepare name collision**
IRIX `<unistd.h>` declares `extern int atfork_parent(void (*)(int,int))`. Git's run-command.c defines `static void atfork_parent(struct atfork_state *)`. Link-time or compile-time conflict.
**Fix:** prep_command: `sed -i 's/atfork_parent/git_atfork_parent/g; s/atfork_prepare/git_atfork_prepare/g' run-command.c`

**Problem: IRIX fileno macro breaks with void* argument**
IRIX fileno is a macro. In http.c, `fileno(result)` where `result` is `void*` causes macro expansion to dereference void pointer.
**Fix:** prep_command: `sed -i 's/fileno(result)/(((FILE *)result)->_file)/g' http.c`

**Problem: dirname/basename live in libgen.so on IRIX, not libc**
Missing `dirname` and `basename` at link time.
**Fix:** Add `-lgen` to EXTLIBS or LDFLAGS. IRIX has `<libgen.h>` and both functions in `/usr/lib32/libgen.so`.

**Problem: pushd/popd in spec files (bash-isms)**
Fedora specs use `pushd`/`popd` which don't exist in `/bin/sh`.
**Fix:** spec_replacements: replace `pushd dir > /dev/null ... popd > /dev/null` with `(cd dir && ...)` subshells, or skip entirely if the operation isn't needed.

**Problem: Brace expansion in spec files**
Fedora specs use `{foo,bar}` brace expansion (bash-ism) in find commands, rm, etc.
**Fix:** spec_replacements: expand manually. E.g., `find %{buildroot}{%{_bindir},%{_libexecdir}}` → `find %{buildroot}%{_bindir} %{buildroot}%{_libexecdir}`.

**Problem: irix-cc wrapper misroutes -MM (dependency generation)**
Makefile-based builds that use `$(CC) -MM` for generating dependency files fail because the irix-cc wrapper doesn't recognize `-MM` as a preprocessing operation and falls through to compile+link mode.
**Fix:** Pre-create empty deps.mk and remove the regeneration rule from the Makefile: `touch src/deps.mk && sed -i '/^deps.mk:/,/>/d' src/Makefile`. See feh.yaml.

**Problem: _SC_HOST_NAME_MAX / HOST_NAME_MAX missing on IRIX**
Code using `sysconf(_SC_HOST_NAME_MAX)` or `HOST_NAME_MAX` fails. IRIX has `MAXHOSTNAMELEN` (256) but not the POSIX constant.
**Fix:** `sed -i '1i#ifndef HOST_NAME_MAX\n#define HOST_NAME_MAX 255\n#endif' src/file.c`. See feh.yaml.

**Problem: Make CFLAGS= on command line kills += additions**
When a Makefile uses `CFLAGS += -DPREFIX=... -DPACKAGE=...`, passing `CFLAGS=value` on the make command line overrides ALL `+=` additions, stripping crucial defines.
**Fix:** Use `export CFLAGS=value` (environment variable) instead of `make CFLAGS=value`. The env var allows `?=` to skip but `+=` to still append. See feh.yaml.

**Problem: IRIX scandir function pointer type mismatch**
IRIX `scandir()` uses `int (*)(dirent_t *)` instead of POSIX `int (*)(const struct dirent *)`. Causes incompatible-function-pointer-types errors.
**Fix:** Add `-Wno-incompatible-function-pointer-types` to CFLAGS. See feh.yaml.
