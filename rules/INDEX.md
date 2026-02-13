# Mogrix Rules Index

## Check generic.yaml First

**Before adding rules to a package YAML**, check what `rules/generic.yaml` already provides.
Generic rules are applied to EVERY package automatically. Do NOT duplicate them in package files.

**generic.yaml already handles:**

| What | Mechanism | Details |
|------|-----------|---------|
| Test suite | `skip_check: true` | Can't run MIPS binaries on build host |
| Linux-only build deps | `drop_buildrequires` | systemd, systemd-devel, libselinux-devel, kernel-headers, systemtap-sdt-devel, libcap-devel, libcap-ng-devel, libacl-devel, libattr-devel, audit-libs-devel, alsa-lib-devel, gnupg2, gcc-c++, libdicl, libdicl-devel |
| Linux-only runtime deps | `drop_requires` | libdicl, libdicl-devel |
| libdicl removal | `remove_lines` | CPPFLAGS/LIBS exports, %gpgverify, %ldconfig_scriptlets, systemd scriptlets |
| Common compat functions | `inject_compat_functions` | strdup, strndup, getline, getopt_long, asprintf, vasprintf (+ dlmalloc auto) |
| malloc(0) cross-detect | `ac_cv_overrides` | ac_cv_func_malloc_0_nonnull, ac_cv_func_realloc_0_nonnull, gl_cv_func_select_* |
| SGUG prefix paths | `rpm_macros` | _prefix=/usr/sgug, _libdir=/usr/sgug/lib32, _bindir, _includedir, etc. |
| Path translation | `rewrite_paths` | /usr/lib64 → /usr/sgug/lib32, /usr/lib → /usr/sgug/lib32, /usr/include → /usr/sgug/include |
| Linux-only features | `configure_disable` | selinux, systemd, udev, inotify, epoll, fanotify, timerfd, libcap, audit |
| Header stubs | `header_overlays: generic` | execinfo.h, malloc.h, error.h, etc. |
| Install cleanup | `install_cleanup` | Fix /usr/bin paths, rm *.la, rm infodir/dir |
| Subpackage bloat | `drop_subpackages` | debuginfo, debugsource, langpack-* |

**Only add to a package YAML** when the package needs something beyond the above.

---

## Per-Package Problem Reference

| Problem class | Symptoms / triggers | Rule mechanism | Rule location | Notes / constraints |
|---------------|---------------------|----------------|---------------|---------------------|
| Before you start | Stuck, confused, debugging | Read checklist | methods/before-you-start.md | Check SGUG-RSE, git history, clean build |
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
| TLS not supported | __thread / _Thread_local, rld Fatal Error `__tls_get_addr` | prep_commands | rules/packages/* | Sed `#define _Thread_local __thread` → `#define _Thread_local` in headers. `-D` from CFLAGS WON'T WORK (source `#define` overrides). See gnutls.yaml |
| Plugin dlopen path | App has plugins in non-standard path, commands missing | bundle wrapper env | mogrix/bundle.py | Set `WEECHAT_EXTRA_LIBDIR` etc. via `{extra_env_block}` in wrapper |
| gnutls CA certs | "certificate issuer unknown", TLS handshake fails | bundle `-r` flag | mogrix/bundle.py | Bundle auto-includes CA certs; weechat wrapper passes `-r "/set weechat.network.gnutls_ca_user ..."` |
| extra_cflags dead code | Rule validated but never applied | use prep_commands or export_vars | rules/packages/* | `extra_cflags` in validator.py but NOT in engine.py. Workaround: `prep_commands` sed or `export_vars: CFLAGS` |
| vsnprintf pre-C99 | garbled output, truncation | inject_compat_functions: vasprintf | rules/packages/* | IRIX vsnprintf(NULL,0) returns -1 |
| Linker selection | .so crashes rld, bad relocations | export_vars: LD | rules/packages/* | GNU ld for -shared, LLD 18 for exe |
| Path conventions | /etc vs /usr/sgug/etc | spec_replacements | rules/packages/* | SGUG uses /usr/sgug prefix |
| Scriptlet failures | ldconfig, systemd macros | spec_replacements | rules/packages/* | Make ldconfig no-op via macros.ldconfig |
| Man page compression | .1.gz vs .1 in %files | spec_replacements | rules/packages/* | Cross-build doesn't compress |
| NLS/gettext | %find_lang fails | skip_find_lang: true | rules/packages/* | Often disable NLS entirely |
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
| Long double crash | IRIX MIPS n32 has no 128-bit long double | ac_cv_overrides | rules/packages/* | `ac_cv_type_long_double_wider: "no"` |
| IRIX libgen.so | dirname/basename in libgen.so, not libc | spec_replacements | rules/packages/* | `LIBS="$LIBS -lgen"` before %configure |
| Cross-build doc generation | Build tries to run MIPS binary for docs | spec_replacements | rules/packages/* | Override make vars to empty, remove doc from `all:` target |
| bcond flipping | Spec has inline %if/%else/%endif inside %configure continuation | spec_replacements | rules/packages/* | `%bcond_without X` → `%bcond_with X`; RPM resolves conditionals to %else branches. configure_flags:remove can't handle these. See gnutls.yaml |
| PRIdMAX/SCNd64 macros | Compile error: implicit/undeclared PRIdMAX, SCNd64, etc. | compat header | compat/include/mogrix-compat/generic/inttypes.h | 18 macros: PRId/PRIu/PRIx/PRIX/PRIoMAX, PRId/PRIu/PRIx/PRIXPTR, SCNd/SCNu/SCNx64, SCNd/SCNu/SCNxMAX |
| cmake %cmake3 macro | FC40 spec uses %cmake3/%cmake3_build/%cmake3_install | spec_replacements | rules/packages/* | Replace with raw cmake + make. Set CMAKE_SYSTEM_NAME=IRIX, cross tools, staging paths. See weechat.yaml, tdnf.yaml |
| select() duplicate symbol | dicl-clang-compat `extern select()` vs IRIX `static select()` | header guard | cross/include/dicl-clang-compat/sys/select.h | Guard with `#ifndef _SYS_TIME_H` to prevent conflict when _XOPEN_SOURCE set |
| rld symbol resolution debug | Binary crashes silently or ldd SIGSEGV | `_RLD_ARGS="-log /tmp/rld.log"` | methods/irix-testing.md | Shows unresolvable symbols, soname mismatches. Check NEEDED sonames match installed .so files |
| ncurses ext-colors terminfo | SIGBUS on MIPS, garbage cols/lines values | spec_replacements | rules/packages/ncurses.yaml | `--disable-ext-colors`; ext-colors reads 16-bit terminfo fields as 32-bit |
| Explicit Provides required | rpm -Uvh fails with unresolved deps | spec_replacements | rules/packages/* | rpmmacros.irix sets AutoProv:no; add `Provides: libfoo.so.N` for each .so |
| Plugin dlopen symbol export | rld Fatal Error: unresolvable symbol in plugin .so | --dynamic-list via spec_replacements | cross/bitlbee-plugin-symbols.list | --export-dynamic exports ALL symbols, crashes rld (>468 entries). --dynamic-list exports only listed symbols. Create .list file with plugin's UND symbols: `readelf -sW plugin.so \| grep UND` |
| GNU symbol versioning crash | dlopen SIGSEGV in rld processing .gnu.version* sections | irix-ld filters --version-script | cross/bin/irix-ld | Qt5, GLib, and other libs pass `--version-script` via `-Wl,`; irix-ld strips it. Symbols still exported, just without version tags |
| .init_array ignored by rld | Static constructors silently don't run in shared libs | custom linker script + CRT objects | cross/irix-shared.lds, cross/crt/ | `.ctors` preserved by linker script; crtbeginS.o provides `_init` that walks .ctors array. All .so linked via GNU ld now uses this |
| rld re-encounter GOT crash | SIGBUS when DSO with large GOT is loaded as NEEDED after dlopen | `beqz` guard in crtbeginS.o | cross/crt/crtbeginS.S | rld re-runs DT_INIT on re-encounter, zeros global GOT entries above LOCAL_GOTNO ~128. Guard checks `__CTOR_END__` for NULL before deref. Threshold: LOCAL_GOTNO ≤103 OK, ≥140 crashes. |
| rld global GOT entry limit | SIGSEGV/SIGBUS in rld (PC=0x0FB6AA44) loading .so with large symbol table | `-Bsymbolic-functions` in irix-ld | cross/bin/irix-ld | rld has ~4370 global GOT entry limit per shared library. Global GOT = SYMTABNO - GOTSYM. Qt5Gui (5364) and Qt5Widgets (8567) exceed it. `-Bsymbolic-functions` moves defined FUNC symbols to local GOT, reducing count by 50-65%. Added unconditionally to irix-ld for all shared libraries. |
| Circular NEEDED crashes rld | SIGSEGV (PC=0x0) during dependency resolution with circular NEEDED entries | Break the cycle | rules/packages/freetype.yaml | freetype↔harfbuzz circular dep (freetype NEEDED harfbuzz, harfbuzz NEEDED freetype) crashes rld during single-pass resolution combined with deep dep trees. Fix: `--without-harfbuzz` for freetype. Harfbuzz still works standalone. |
| rld deep dependency tree crash | dlopen of library with 10+ transitive deps crashes rld | Sequential preloading | Bundle wrappers | Pre-load heavy deps (Qt5Core, harfbuzz) individually via sequential dlopen before loading the leaf library (Qt5Gui). Bundle wrappers implement the preload chain. |
| IRIX rld strict UND resolution | "unresolvable symbol" even when symbol exists in already-loaded DSO | Add explicit NEEDED | rules/packages/libxcb.yaml | IRIX rld does NOT search already-loaded DSOs for UND symbols unless there's a direct NEEDED chain. libxcb had UND `_XLockMutex_fn` from libX11 but no NEEDED libX11. Fix: `-lX11` in LDFLAGS. |
| IRIX lacks endian.h | `#include <endian.h>` or `<sys/endian.h>` fails; no byte-order macros | **GLOBAL: dicl-clang-compat/endian.h** | cross/include/dicl-clang-compat/endian.h | Provided globally — no per-package fix needed. Defines BYTE_ORDER, htobe16/32/64, htole16/32, be16toh/32toh/64toh, le16toh/32toh and bswap helpers. MIPS big-endian: htobe/betoh are no-ops. |
| IRIX struct dirent has no d_type | `d_type`, `DT_REG`, `DT_DIR` undeclared; compile error | prep_commands or add_patch | rules/packages/telescope.yaml | BSD/Linux extension not in IRIX `struct dirent`. Replace `d_type` checks with `stat()` calls and `S_ISREG()`/`S_ISDIR()` macros. |
| IRIX clock_gettime crashes | SIGSEGV or undefined CLOCK_MONOTONIC/CLOCK_REALTIME | prep_commands or add_patch | rules/packages/telescope.yaml | IRIX has `clock_gettime` in libc but no POSIX `CLOCK_*` constants defined. Calling with any value crashes. Replace with `gettimeofday()`. |
| IRIX lacks timersub macro | `timersub()` undeclared | **GLOBAL: dicl-clang-compat/sys/time.h** | cross/include/dicl-clang-compat/sys/time.h | Provided globally — no per-package fix needed. Also provides timeradd, timercmp, timerclear, timerisset. |
| IRIX scandir selector signature | `const struct dirent *` vs `dirent_t *` mismatch | prep_commands or add_patch | rules/packages/* | Linux: `int (*)(const struct dirent *)`. IRIX: `int (*)(dirent_t *)` (no const). Remove const from selector function signatures. |
| IRIX lacks open_memstream | `open_memstream()` undeclared / link error | **inject_compat_functions: open_memstream** | compat/stdio/open_memstream.c | Compat function available. Uses funopen() internally — must also inject funopen. Header declaration in mogrix-compat/generic/stdio.h (auto-available). |
| IRIX lacks dprintf | `dprintf(fd, fmt, ...)` undeclared / link error | **inject_compat_functions: dprintf** | compat/stdio/dprintf.c | Compat function available. Also provides vdprintf. Uses dup()+fdopen()+vfprintf(). Header declaration in mogrix-compat/generic/stdio.h (auto-available). |
| IRIX lacks IOV_MAX | `IOV_MAX` undeclared | **GLOBAL: dicl-clang-compat/limits.h** | cross/include/dicl-clang-compat/limits.h | Provided globally — no per-package fix needed. Defined as 1024 (POSIX typical). |
| MAP_ANON not on IRIX | `MAP_ANON` / `MAP_ANONYMOUS` undeclared | prep_commands or compat header | rules/packages/* | IRIX has no `MAP_ANON`/`MAP_ANONYMOUS`. Anonymous mmap must use `/dev/zero` fd with `MAP_PRIVATE`. Define `MAP_ANON` as 0 and open `/dev/zero` for the fd argument. |
| IRIX getentropy via /dev/urandom | `getentropy()` / `getrandom()` link error | prep_commands or add_patch | rules/packages/* | IRIX has no `getentropy`/`getrandom` syscall. `/dev/urandom` works and is available. Replace calls with `open("/dev/urandom") + read()`. |
| libretls exports compat functions | configure falsely detects `reallocarray`, `strlcpy`, `strlcat` as available | ac_cv_overrides | rules/packages/* | libretls.so exports these functions; configure links against it and thinks system libc has them. Fix: `ac_cv_func_reallocarray: "no"`, `ac_cv_func_strlcpy: "no"`, etc. to force internal implementations. |
| Cross-compile HOSTCC pattern | Build fails running MIPS binary on build host | spec_replacements or make_env | rules/packages/* | Build-time code generators (pagebundler, etc.) need host compiler. Pass `HOSTCC=gcc` to configure. Build system must use `BUILD_CC`/`HOSTCC` for generators and `CC` for target objects. See telescope.yaml. |
| Bundled library cross-compilation | Bundled library compiles for host instead of target | spec_replacements or prep_commands | rules/packages/* | Libraries bundled in source (e.g. libgrapheme in telescope) default to host CC. Must fix their config.mk/Makefile to use cross-compiler for target objects and host compiler for generators. |

## Invariants (Settled Facts)

| Fact | Implication | Reference |
|------|-------------|-----------|
| LLD 14 has MIPS relocation bugs | Always use LLD 18 with patches | lld-fixes/README.md |
| GNU ld exe layout crashes rld | LLD for executables, GNU ld for -shared only | methods/linker-selection.md |
| IRIX vsnprintf is pre-C99 | vsnprintf(NULL,0) returns -1, use iterative vasprintf | compat/stdio/asprintf.c |
| IRIX lacks __thread TLS | Remove from source via patch | per-package in patches/ |
| IRIX /dev/fd + utimes fails | futimens workaround in openat-compat.c | compat/dicl/openat-compat.c |
| IRIX chroot doesn't isolate | Binaries see base system paths | methods/irix-testing.md |
| fopencookie crashes | Use funopen instead | compat/stdio/funopen.c |
| IRIX libc has no `%zu` support | Use `%u` for size_t, `%d` for ssize_t; `%zu` corrupts varargs → SIGSEGV | methods/irix-quirks.md |
| dlmalloc replaces malloc but not strdup | Must inject compat strdup/strndup alongside dlmalloc | methods/irix-quirks.md |
| GNU ld linker scripts crash rld | Replace `INPUT(-lfoo)` .so files with symlinks | methods/irix-quirks.md |
| `-z separate-code` needed for .so | Without it, GNU ld produces single-segment .so that crashes rld | methods/linker-selection.md |
| brk() heap limited to 176MB | libpthread at 0x0C080000 blocks heap growth | methods/irix-address-space.md |
| mmap-based malloc bypasses limit | dlmalloc in compat uses mmap, 1.2GB available | compat/malloc/dlmalloc.c |
| Volatile fptr initializers crash | `static volatile fptr = memset;` relocation fails on rld | compat/string/explicit_bzero.c |
| Source analysis is rules-driven | Patterns in source_checks.yaml + catalog.yaml source_patterns | rules/source_checks.yaml |
| `-rdynamic` filtered in irix-ld | LLD doesn't support it; IRIX rld crashes on large dynamic symbol tables | cross/bin/irix-ld |
| GNU version scripts crash rld | `--version-script` creates VERNEED/VERSYM dynamic tags that IRIX rld doesn't understand → SIGSEGV during dlopen | cross/bin/irix-ld (filtered) |
| Shared libs need .ctors + DT_INIT | IRIX rld ignores `.init_array`; GNU ld merges .ctors→.init_array by default. Custom linker script + crtbeginS.o keeps .ctors and provides DT_INIT via .init function | cross/crt/crtbeginS.S, cross/irix-shared.lds |
| C++ static ctors in .so require CRT | Without crtbeginS.o/crtendS.o, static constructors silently don't run → NULL d_ptr in STL containers, crashes on first use | cross/crt/crtbeginS.S, cross/crt/crtendS.S |
| rld re-runs DT_INIT on re-encounter | Loading DSO A, then DSO B that NEEDS A, triggers A's .init again. Global GOT entries zeroed → SIGBUS. Fixed with beqz guard in crtbeginS.o | cross/crt/crtbeginS.S |
| rld global GOT limit ~4370 | Shared libs with >4370 global GOT entries crash rld (SIGSEGV at rld internal PC). Not total GOT, specifically global entries. `-Bsymbolic-functions` reduces by moving defined FUNC symbols from global→local GOT | cross/bin/irix-ld |
| X11 via IRIX native sysroot | Don't use `--x-includes`/`--x-libraries`; cross-compiler `--sysroot` finds X11 | rules/packages/aterm.yaml |
| IRIX X11R6.3 API limits | XICCallback→XIMCallback, XSetIMValues→no-op, Xutf8→Xmb, XUTF8→XCompound | rules/packages/st.yaml |
| Bundle wrapper shell recursion | Wrappers use `dirname`/`pwd`/`basename`; if those are also bundled commands, `$PATH` causes infinite loop fork bomb. Use `/bin/dirname`, `/bin/pwd`, `/bin/basename` (absolute paths) in all shell templates | mogrix/bundle.py |
| IRIX has no TrueType fonts | Xft apps need bundled fonts; fontconfig `<dir prefix="relative">` for bundle paths | mogrix/bundle.py `_include_fonts()` |
| `_XOPEN_SOURCE=600` hides IRIX defs | winsize/TIOCSWINSZ/TIOCGWINSZ gated by `_NO_XOPEN5`; remove, use `_SGI_SOURCE` | rules/packages/st.yaml |
| setenv/unsetenv NOT in IRIX libc | `putenv()` only; compat wraps it. Must add to `inject_compat_functions` | compat/stdlib/setenv.c |
| X11 .pc files for IRIX native libs | libXrender/libXft configure use `pkg-config --exists x11/xext`; handcrafted .pc files in staging | /opt/sgug-staging/.../pkgconfig/ |
| Man pages not compressed | rpmmacros.irix disables brp scripts; use `*.1*` not `*.1.gz` in %files | rpmmacros.irix |
| Circular NEEDED crashes rld | freetype↔harfbuzz circular dep + deep dep tree → rld SIGSEGV. Break cycles at build time | rules/packages/freetype.yaml |
| rld strict UND resolution | IRIX rld doesn't search already-loaded DSOs for UND symbols unless there's a NEEDED edge. Must add explicit `-l` linkage | rules/packages/libxcb.yaml |
| Qt5 needs preload chain | rld can't handle Qt5Gui's full dep tree in one dlopen. Must preload Qt5Core + harfbuzz first. Bundle wrappers implement this | rules/packages/qt5-qtbase.yaml |
| Old /opt/cross/bin/irix-ld broken | Simple GNU ld wrapper produces MIPS_OPTIONS tags → rld crash. Always use staging irix-ld (LLD 18 for executables) | /opt/sgug-staging/usr/sgug/bin/irix-ld |
| AUTOPOINT=true for NLS-disabled | Packages with autoreconf + nls-disabled class need `AUTOPOINT=true` | rules/packages/*.yaml |
| C++ uses GCC 9 libstdc++ | clang++ with SGUG-RSE libstdc++ headers + custom CRT (.ctors) | cross/bin/irix-cxx |
| IRIX libm lacks C99 math | Must disable _GLIBCXX_USE_C99_MATH_TR1 in c++config.h | staging c++config.h |
| autoreconf regenerates configure | prep_commands on configure get overwritten; use spec_replacements post-autoreconf | rules/packages/*.yaml |
| sockaddr_storage hidden by _XOPEN_SOURCE | dicl-clang-compat sys/socket.h needs explicit struct definition | cross/include/dicl-clang-compat/sys/socket.h |
| R_MIPS_REL32 crashes rld | Function pointers in static data arrays cause rld SIGSEGV | patches/packages/openssh/ |
| IRIX long double = double (n32) | 128-bit long double not supported; __extenddftf2 crashes | rules/packages/*.yaml |
| dirname/basename in libgen.so | Not in libc; must link -lgen explicitly | rules/packages/nano.yaml |
| bcond flipping is safe | Changing `%bcond_without` to `%bcond_with` only affects the macro default; all `%if %{with X}` blocks evaluate to FALSE, keeping `%else` branches | rules/packages/gnutls.yaml |
| AutoProv: no in rpmmacros.irix | Cannot be overridden from spec; every .so-producing package needs explicit `Provides:` | rpmmacros.irix |
| ncurses ext-colors breaks terminfo | ABI 6 auto-enables ext-colors → 32-bit number fields → garbage cols/lines → SIGBUS | rules/packages/ncurses.yaml |
| _RLD_ARGS="-log" for debugging | IRIX rld logging shows unresolvable symbols, wrong sonames, load order | methods/irix-testing.md |
| IRIX inttypes.h lacks MAX/PTR macros | PRIdMAX, SCNd64, etc. not defined; mogrix-compat inttypes.h provides them | compat/include/mogrix-compat/generic/inttypes.h |
| Source `#define` overrides `-D` | CFLAGS `-D_Thread_local=` won't work if source has `#define _Thread_local __thread` | rules/packages/gnutls.yaml |
| `extra_cflags` rule is dead code | Validated in validator.py but never applied by engine.py; use prep_commands or export_vars | mogrix/rules/validator.py |
| `skip_manpages` rule is dead code | Validated but never applied by engine. Use prep_commands to touch man pages | mogrix/rules/validator.py |
| `make_env` rule is dead code | Validated but never applied by engine. Use spec_replacements instead | mogrix/rules/validator.py |
| `drop_subpackages` must be inside `rules:` | Only `add_patch` and `add_source` are handled at both top-level and rules-level | rules/packages/*.yaml |
| `install_cleanup` misplacement | If spec has `%triggerpostun` before `%files`, cleanup lands in trigger scriptlet. Workaround: use spec_replacements | mogrix/rules/engine.py |
| `remove_conditionals` can't nest | Simple regex matches first `%endif`, not the correct one for nested `%if/%endif`. Use bcond flipping instead | mogrix/rules/engine.py |
| Clang 16 `.cpsetup` N32 bug | `.cpsetup` expands to `__gnu_local_gp` absolute refs instead of GP-relative. Fix: `-fno-integrated-as -B/opt/cross/bin/mips-sgi-irix6.5-`. Fixed in LLVM 18 (issue #52785) | rules/packages/libffi.yaml |
| cmake finds MIPS staging binaries | `CMAKE_FIND_ROOT_PATH` picks up gmake/pkg-config from staging. Must set `-DCMAKE_MAKE_PROGRAM=/usr/bin/make -DPKG_CONFIG_EXECUTABLE=/usr/bin/pkg-config` | rules/packages/weechat.yaml |
| `inject_compat_functions` affects HOST link | Bootstrap packages (flex, etc.) build HOST tools before cross-compiling. `-lmogrix-compat` in LIBS breaks HOST link. Fix: inject compat via prep_commands into cross-only source files | rules/packages/flex.yaml |
| Perl `drop_subpackages` glob trap | `"*"` glob matches `-f` in `%files -f`, breaking file manifests. Use `"[A-Za-z]*"` to match package names only | rules/packages/perl.yaml |
| SSL_CERT_FILE is OpenSSL-only | gnutls ignores it; use app-specific CA config (weechat: gnutls_ca_user) | mogrix/bundle.py |
| weechat 4.x renamed gnutls settings | `gnutls_ca_file` → `gnutls_ca_system` + `gnutls_ca_user` | rules/packages/weechat.yaml |
| IRIX has no endian.h | **SOLVED**: dicl-clang-compat/endian.h provides BYTE_ORDER, htobe/htole, bswap globally | cross/include/dicl-clang-compat/endian.h |
| IRIX struct dirent lacks d_type | `d_type`, `DT_REG`, `DT_DIR` are BSD/Linux extensions; use `stat()` instead (per-package) | rules/packages/telescope.yaml |
| IRIX clock_gettime has no CLOCK_* | Function exists in libc but CLOCK_MONOTONIC/CLOCK_REALTIME undefined; calling crashes (per-package) | rules/packages/telescope.yaml |
| IRIX lacks timersub macro | **SOLVED**: dicl-clang-compat/sys/time.h provides timersub/timeradd/timercmp globally | cross/include/dicl-clang-compat/sys/time.h |
| IRIX scandir selector lacks const | IRIX signature: `int (*)(dirent_t *)` not `int (*)(const struct dirent *)` (per-package) | rules/packages/*.yaml |
| IRIX lacks open_memstream | **SOLVED**: `inject_compat_functions: open_memstream` (also needs funopen) | compat/stdio/open_memstream.c |
| IRIX lacks dprintf | **SOLVED**: `inject_compat_functions: dprintf` (also provides vdprintf) | compat/stdio/dprintf.c |
| IRIX lacks IOV_MAX | **SOLVED**: dicl-clang-compat/limits.h provides IOV_MAX=1024 globally | cross/include/dicl-clang-compat/limits.h |
| MAP_ANON not on IRIX | No `MAP_ANON`/`MAP_ANONYMOUS`; anonymous mmap requires `/dev/zero` fd + `MAP_PRIVATE` | rules/packages/*.yaml |
| IRIX has no getentropy/getrandom | `/dev/urandom` available as fallback | rules/packages/*.yaml |
| libretls exports compat symbols | `reallocarray`, `strlcpy`, `strlcat` in libretls.so fool configure; override with ac_cv | rules/packages/*.yaml |
| HOSTCC needed for code generators | Build-time generators must use host compiler; target objects use cross-compiler | rules/packages/telescope.yaml |

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
| Methods | rules/methods/*.md | Process documentation (15 files) |

**Methods index:**

| File | Purpose |
|------|---------|
| before-you-start.md | Pre-debug checklist (read first when stuck) |
| mogrix-workflow.md | How to run mogrix commands |
| package-rules.md | Writing package YAML rules |
| autoconf-cross.md | Autotools cross-compilation patterns |
| cmake-cross.md | CMake cross-compilation patterns |
| compat-functions.md | Adding compat functions to catalog |
| patch-creation.md | Creating source patches |
| text-replacement.md | safepatch vs sed |
| linker-selection.md | GNU ld vs LLD 18 selection |
| irix-testing.md | IRIX shell rules, chroot, debugging |
| irix-quirks.md | IRIX libc/rld quirks reference |
| irix-address-space.md | IRIX memory layout and limits |
| bundles.md | Bundle architecture end-to-end |
| build-order.md | Package build ordering |
| task-tracking.md | ultralist task tracking |

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

**Auditing for elevation:** Run `mogrix audit-rules` to detect duplicated rules across packages and flag candidates for promotion to class or generic level.

**Elevation candidates** (audit 2026-02-11, rules used in 8+ packages):

| Rule | Type | Count | Candidate for |
|------|------|-------|---------------|
| `strdup` | inject_compat_functions | 69 | generic.yaml |
| `strndup` | inject_compat_functions | 48 | generic.yaml |
| `getline` | inject_compat_functions | 24 | generic.yaml |
| `getopt_long` | inject_compat_functions | 21 | generic.yaml |
| `rm *.la` | install_cleanup | 19 | generic.yaml |
| `asprintf` | inject_compat_functions | 14 | generic.yaml |
| `--disable-static` | configure_flags: add | 11 | generic.yaml |
| `gcc-c++` | drop_buildrequires | 11 | generic.yaml |
| `vasprintf` | inject_compat_functions | 10 | generic.yaml |
| `texinfo` | drop_buildrequires | 10 | class rule |
| `--disable-silent-rules` | configure_flags: add | 9 | generic.yaml |
| `--disable-doc` | configure_flags: add | 8 | generic.yaml |
| `rm infodir/dir` | install_cleanup | 7 | generic.yaml |
| `gl_cv_func_select_*` | ac_cv_overrides | 5-6 | generic.yaml |

Moving these to generic requires testing — compat function injection adds link-time overhead and some packages may not tolerate `--disable-static`. Evaluate before bulk promotion.

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

## Source Analysis

`mogrix analyze` and `mogrix convert` scan source tarballs for IRIX-incompatible patterns.

**Adding a new check:** Edit `rules/source_checks.yaml` or add `source_patterns` to functions in `compat/catalog.yaml`. No code changes needed.

**Two pattern sources:**
1. `rules/source_checks.yaml` — Issue-level patterns (need patches/overrides): `%zu`, `__thread`, volatile fptrs, epoll, inotify, getrandom, etc.
2. `compat/catalog.yaml` `source_patterns` — Function-level patterns (need compat injection): strdup, fopencookie, getline, asprintf, etc.

**Behavior differs by command:**
- `mogrix analyze` — Shows ALL findings (full triage for new packages)
- `mogrix convert` — Cross-references with rules, shows only UNHANDLED findings
