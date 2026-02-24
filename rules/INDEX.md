# Mogrix Rules Index

> **Do NOT read this entire file.** Grep for specific problem keywords. See `rules/GENERIC_SUMMARY.md` for what generic.yaml already handles.

---

## Per-Package Problem Reference

| Problem class | Symptoms / triggers | Rule mechanism | Rule location | Notes |
|---------------|---------------------|----------------|---------------|-------|
| Before you start | Stuck, confused, debugging | Read checklist | methods/before-you-start.md | |
| Linux-only syscalls/APIs | Silent failures, missing networking | Grep for OS(LINUX), syscall names | methods/linuxism-detection.md | EARLY troubleshooting step |
| Batch builds / agents | Building multiple packages, parallel agents | Wave orchestration | methods/task-tracking.md | Max 2-3 agents |
| Upstream (non-Fedora) package | Package only in git/tarball, not FC40 | upstream: block + create-srpm | methods/upstream-packages.md | |
| Suite bundle | Combine multiple apps in one bundle | mogrix bundle pkg1 pkg2 --name X | methods/upstream-packages.md | |
| Package-specific build deps | dbus-devel, gpgme-devel, etc. | drop_buildrequires | rules/packages/* | Only deps NOT in generic.yaml |
| Package-specific runtime deps | systemd-libs, rpm-libs, etc. | drop_requires | rules/packages/* | |
| Autoconf cross-detect | configure misdetects headers/funcs | ac_cv_overrides | rules/packages/* | malloc/realloc in generic |
| Missing libc functions | setenv, strsep, getline, asprintf | inject_compat_functions | rules/packages/* | Check compat/catalog.yaml |
| Missing POSIX.1-2008 | openat, fstatat, mkdirat, utimensat | inject_compat_functions | rules/packages/* | dicl/openat-compat.c has 17 *at funcs |
| Missing BSD functions | funopen, strlcpy, getprogname | inject_compat_functions | rules/packages/* | funopen works, fopencookie crashes |
| Libtool cross-detect | libtool says "unknown platform" | spec_replacements | rules/packages/* | fix-libtool-irix.sh after configure |
| CMake cross-compile | find_package fails, wrong paths | spec_replacements | rules/packages/* | CMAKE_FIND_ROOT_PATH |
| RPM macro pollution | %configure clobbers cross flags | configure_flags: remove | rules/packages/* | Or use spec_replacements |
| TLS not supported | __thread, _Thread_local, __tls_get_addr Fatal Error | prep_commands, -Dtls=disabled | gnutls.yaml, gdbm.yaml, pixman.yaml | sed #define, not CFLAGS -D. Runtime crash not link error |
| Missing forkpty / PTY | forkpty undefined, tmux "server exited" | prep_commands | tmux.yaml | IRIX _getpty(), reopen slave after setsid() without O_NOCTTY |
| libevent devpoll crash | Server crashes, devpoll backend | bundle wrapper env | mogrix/bundle.py | EVENT_NODEVPOLL=1 forces poll() backend |
| Plugin dlopen path | App plugins in non-standard path | bundle wrapper env | mogrix/bundle.py | WEECHAT_EXTRA_LIBDIR etc. |
| gnutls CA certs | "certificate issuer unknown", TLS handshake fails | configure_flags: add | gnutls.yaml | --with-default-trust-store-file |
| extra_cflags dead code | Rule validated but never applied | use prep_commands or export_vars | rules/packages/* | Bug: extra_cflags in validator but NOT engine |
| vsnprintf pre-C99 | garbled output, truncation | inject_compat_functions: vasprintf | rules/packages/* | IRIX vsnprintf(NULL,0) returns -1 |
| Linker selection | .so crashes rld, bad relocations | export_vars: LD | rules/packages/* | LLD 18 default. BFD fallback: MOGRIX_USE_BFDLD_SHARED=1 |
| BFD multi-GOT corruption | 30% GOT entries point to data in large .so | switch to LLD | cross/bin/irix-ld | BFD -Bsymbolic + multi-GOT mixes code/data |
| LLD multi-GOT / secondary GOT | SIGSEGV in constructors for large .so (>16K GOT) | -mxgot + --mips-got-size=1048576 | cross/bin/irix-ld, irix-cc | rld only processes primary GOT. Both flags in irix-ld/irix-cc |
| rld unresolvable symbol | SIGSEGV or exit(1) with no output | check /var/adm/SYSLOG | `irix_host_exec "tail /var/adm/SYSLOG"` | rld logs to syslog, NOT stderr |
| dlopen(libpthread/libstdc++) | SIGSEGV dlopen'ing lib needing libpthread/libstdc++ | pre-load as NEEDED | link exe with -lpthread -lstdc++ | IRIX system libs crash under dlopen |
| LLD shared lib flags | rld "Cannot map" or missing PIC | --no-rosegment -z norelro --image-base | cross/bin/irix-ld | 2-segment RE+RW required |
| crtbeginS/crtendS PIC flags | rld "Cannot map" shared lib | .abicalls directive in .S | cross/crt/crtbeginS.S, crtendS.S | CRT .o must have EF_MIPS_PIC+CPIC |
| Path conventions | /etc vs /usr/sgug/etc | spec_replacements | rules/packages/* | |
| Scriptlet failures | ldconfig, systemd macros | spec_replacements | rules/packages/* | |
| Man page compression | .1.gz vs .1 in %files | spec_replacements | rules/packages/* | |
| NLS/gettext | %find_lang fails | skip_find_lang: true | **generic.yaml** | |
| Missing runtime deps | tdnf install says "not found" | add_requires | rules/packages/* | AutoReq disabled for cross |
| Disable features | ldap, nls, tests not needed | configure_disable | rules/packages/* | |
| Add configure flags | Need --with-X or custom flags | configure_flags: add | rules/packages/* | |
| Unwanted install files | .la files, duplicate docs | install_cleanup | rules/packages/* | |
| Legacy libdicl lines | CPPFLAGS/LIBS with libdicl | remove_lines | rules/generic.yaml | |
| GNU ld linker scripts | `INPUT(-lfoo)` text files as .so | spec_replacements | rules/packages/* | rld can't load linker scripts |
| `%zu` format specifier | SIGSEGV in printf/snprintf | add_patch or prep_commands | rules/packages/* | IRIX libc pre-C99. Use %u. See methods/irix-quirks.md |
| dlmalloc + libc strdup | SIGSEGV in free/realloc | inject_compat_functions: strdup | rules/packages/* | |
| Spec conditionals | %if blocks for wrong platform | comment_conditionals | rules/packages/* | |
| X11 path detection | AC_PATH_XTRA fails cross | spec_replacements | rules/packages/* | Remove --x-includes/--x-libraries |
| IRIX X11R6.3 missing APIs | XICCallback, XSetIMValues, Xutf8 undeclared | prep_commands | st.yaml | XICCallback→XIMCallback, Xutf8→Xmb |
| _XOPEN_SOURCE hides IRIX defs | winsize, TIOCSWINSZ missing | prep_commands | rules/packages/* | Remove -D_XOPEN_SOURCE; _SGI_SOURCE provides all |
| _XOPEN_SOURCE hides wprintf | Bare `#define _XOPEN_SOURCE` (no value) = 0 | prep_commands | lolcat.yaml | Change to `#define _XOPEN_SOURCE 500` |
| Host build-tool cross-compile | `./fbc: cannot execute binary file` | spec_replacements | bc.yaml | Build host tool with gcc first |
| XA_UTF8_STRING missing (X11R6.3) | XA_UTF8_STRING undeclared | prep_commands | xclip.yaml | XInternAtom(dpy,"UTF8_STRING",False) |
| Makefile compat ordering | compat undefined symbols in Makefile builds | spec_replacements | st.yaml, figlet.yaml | Build compat BEFORE make, pass via LDFLAGS |
| PKG_CONFIG_SYSROOT_DIR | pkg-config returns IRIX paths | spec_replacements | rules/packages/* | export PKG_CONFIG_SYSROOT_DIR |
| No TrueType fonts on IRIX | Xft/fontconfig "can't open font" | bundle fonts | mogrix/bundle.py | _include_fonts() copies TTFs |
| setenv/unsetenv missing | link error: undefined setenv/unsetenv | inject_compat_functions | rules/packages/* | IRIX has putenv not setenv |
| Unpackaged doc files | make install + %doc both install docs | install_cleanup | rules/packages/* | |
| AUTOPOINT=true | autoreconf fails without autopoint | spec_replacements | rules/packages/* | |
| autoreconf overwrites prep | prep_commands modify configure, autoreconf regenerates | spec_replacements | rules/packages/* | Inject sed AFTER autoreconf line |
| sockaddr_storage hidden | _XOPEN_SOURCE=500 hides it | header overlay | cross/include/dicl-clang-compat/sys/socket.h | |
| C++ cmath errors | GCC 9 c++config.h enables C99 math TR1 | staging c++config.h | /opt/sgug-staging/.../bits/c++config.h | Comment out _GLIBCXX_USE_C99_MATH_TR1 |
| wchar_t C++ keyword | IRIX stdlib.h typedef clashes | irix-cxx wrapper | cross/bin/irix-cxx | -D_WCHAR_T prevents typedef |
| Bashisms in specs | pushd/popd, brace expansion | spec_replacements | rules/packages/* | pushd→cd, {a,b}→expand |
| update-alternatives | Doesn't exist on IRIX | spec_replacements | rules/packages/* | Drop Requires(post/preun/postun) |
| Compat header conflicts | Unconditional decls clash with static versions | ac_cv_overrides + inject | rules/packages/* | Override ac_cv_func_X="yes" |
| R_MIPS_REL32 crashes rld | Function pointers in static data | add_patch | patches/packages/* | Dispatch via switch/strcmp. See cross/bin/fix-anon-relocs |
| R_MIPS_REL32 anonymous relocs | LLD sym_idx=0 relocs; rld skips index 0 | post-link fix-anon-relocs | cross/bin/fix-anon-relocs | Two-symbol approach, auto-applied by irix-ld |
| SOCK_SEQPACKET AF_UNIX | errno 120, WebKit IPC abort | compat socketpair | compat/sys/socketpair.c | SOCK_SEQPACKET→SOCK_STREAM. Both sockets bind()ed for getsockname() |
| IRIX lacks MSG_NOSIGNAL | sendmsg() with MSG_NOSIGNAL fails to compile | compat sys/socket.h overlay (`#define MSG_NOSIGNAL 0`) | compat/include/mogrix-compat/generic/sys/socket.h | Callers should also SIG_IGN SIGPIPE. Affects WebKit, Qt5, GnuTLS, libsoup |
| IRIX lacks SOCK_CLOEXEC / SOCK_NONBLOCK | socket() with SOCK_CLOEXEC fails to compile | compat sys/socket.h overlay (defined as 0) | compat/include/mogrix-compat/generic/sys/socket.h | Sockets work but cloexec/nonblock NOT set — must use fcntl() manually. Affects Qt5, dbus |
| IRIX lacks eventfd | eventfd() not available — used for async IPC notification | Package-specific pipe replacement | patches/packages/webkitgtk/ | General: any multi-process app using eventfd needs pipe-based alternative. dbus may hit this |
| MIPS alignment SIGBUS | char* cast to uint32_t* and deref | prep_commands (sed) | rules/packages/* | Use memcpy. Also check endianness: MIPS is big-endian |
| Long double crash | IRIX MIPS n32 no 128-bit long double | ac_cv_overrides | rules/packages/* | ac_cv_type_long_double_wider: "no" |
| IRIX libgen.so | dirname/basename not in libc | spec_replacements | rules/packages/* | LIBS="$LIBS -lgen" |
| Cross-build doc generation | Build tries to run MIPS binary for docs | spec_replacements | rules/packages/* | Override make vars to empty |
| bcond flipping | inline %if/%else inside %configure | spec_replacements | rules/packages/* | %bcond_without→%bcond_with. See gnutls.yaml |
| PRIdMAX/SCNd64 macros | undeclared PRIdMAX, SCNd64 | compat header | compat/include/mogrix-compat/generic/inttypes.h | 18 macros |
| cmake %cmake macro | FC40 uses %cmake/%cmake_build | spec_replacements | json-c.yaml, brotli.yaml | Raw cmake -B _build. CMAKE_SYSTEM_NAME=Linux |
| select() duplicate symbol | dicl-clang-compat sys/select.h | header include fix | cross/include/dicl-clang-compat/sys/select.h | Includes sys/time.h for full struct timeval |
| rld symbol resolution debug | Binary crashes or ldd SIGSEGV | _RLD_ARGS="-log /tmp/rld.log" | methods/irix-testing.md | |
| ncurses ext-colors terminfo | SIGBUS, garbage cols/lines | spec_replacements | ncurses.yaml | --disable-ext-colors |
| Explicit Provides required | rpm -Uvh fails unresolved deps | spec_replacements | rules/packages/* | rpmmacros.irix AutoProv:no |
| Plugin dlopen symbol export | rld Fatal: unresolvable symbol in plugin | --dynamic-list | cross/bitlbee-plugin-symbols.list | --export-dynamic exports ALL, crashes rld |
| GNU symbol versioning crash | dlopen SIGSEGV in .gnu.version* | irix-ld strips --version-script | cross/bin/irix-ld | Qt5, GLib affected |
| .init_array ignored by rld | Static constructors don't run in .so | custom linker script + CRT | cross/irix-shared.lds, cross/crt/ | .ctors preserved; crtbeginS walks array |
| rld re-encounter GOT crash | SIGBUS DSO large GOT loaded after dlopen | beqz guard in crtbeginS.o | cross/crt/crtbeginS.S | LOCAL_GOTNO ≤103 OK, ≥140 crashes |
| rld global GOT entry limit | SIGSEGV PC=0x0FB6AA44 loading large .so | -Bsymbolic in irix-ld | cross/bin/irix-ld | ~4370 global GOT limit. -Bsymbolic binds all defined locally |
| dlmalloc shared lib crash | ABORT cross-heap corruption .so malloc/free | dlmalloc in exe only, not .so | cross/bin/irix-ld | -Bsymbolic-functions causes private heaps |
| dlmalloc + IRIX native libs | SIGSEGV with Motif libXm.so + dlmalloc | MOGRIX_NO_DLMALLOC=1 | nedit.yaml | Only for apps linking IRIX native .so |
| dlmalloc mmap returns 0 | SIGSEGV first mmap in dlmalloc | dlmalloc_irix_mmap() rejects 0 | compat/malloc/dlmalloc-src.inc | IRIX mmap(0,...) can return 0 with 77+ libs |
| _RLDN32_LIST + DT_INIT = SIGILL | SIGILL preloading .so with constructor | No constructors in _RLDN32_LIST libs | cross/crt/, compat/ | rld doesn't relocate DT_INIT for preloaded libs |
| rld MIPS_GOTSYM threshold | "unresolvable symbol" below GOTSYM index | Provide stubs via _RLDN32_LIST | patches/packages/webkitgtk/ | rld only searches .dynsym >= DT_MIPS_GOTSYM |
| Bundle wrapper missing /usr/lib32 | rld can't find IRIX system .so | Fixed in bundle.py | mogrix/bundle.py | Append :/usr/lib32 to LD_LIBRARYN32_PATH |
| CRT symbol visibility | .so picks wrong __CTOR_END__/_init SIGSEGV | .hidden in crtbeginS/crtendS | cross/crt/crtbeginS.S, crtendS.S | Sole cause of GTK3 crash |
| gnutls CA trust store | "peer's certificate issuer unknown" | configure_flags: add | gnutls.yaml | --with-default-trust-store-file |
| Circular NEEDED crashes rld | SIGSEGV PC=0x0 dependency resolution | Break the cycle | freetype.yaml | freetype↔harfbuzz. --without-harfbuzz |
| rld deep dependency tree crash | dlopen 10+ transitive deps crashes rld | Sequential preloading | Bundle wrappers | Pre-load heavy deps individually |
| IRIX rld strict UND resolution | "unresolvable symbol" even when loaded | Add explicit NEEDED | libxcb.yaml | rld needs direct NEEDED chain |
| IRIX lacks endian.h | endian.h / sys/endian.h fails | **GLOBAL** | cross/include/dicl-clang-compat/endian.h | |
| rld SYMTABNO limit for executables | SIGSEGV rld init, first 7 syscalls | Remove export_dynamic:true | gtkterm.yaml | ~350-400 limit. dlmalloc adds ~20 exports |
| IRIX rld NEEDED ordering sensitivity | SIGSEGV before main() wrong lib order | Ensure VTE linked BEFORE GTK | gtkterm.yaml | Unknown root cause. -lvte-2.91 before -lgtk-3 |
| IRIX X11R6 lacks XGE extension | rld Fatal: XESetWireToEventCookie | libx11_xge_stubs.so | compat/x11_xge_stubs.c | --no-as-needed -lx11_xge_stubs |
| GTK3/cairo rendering crash pixman TLS | SIGSEGV stroke/arc/text; __tls_get_addr | -Dtls=disabled pixman + no MIT-SHM cairo | pixman.yaml, cairo.yaml | Runtime crash, not link error. Lazy resolution |
| IRIX IPC_RMID destroys SHM immediately | MIT-SHM between client and X server fails | Disable MIT-SHM at build | cairo.yaml | ipc_rmid_deferred_release doesn't control usage |
| IRIX STREAMS PTY ptem/ldterm | No echo, no \r\n, no ^C in terminal | Push ptem+ldterm on slave PTY | vte291.yaml | Guard __sun must also include __sgi |
| O_CLOEXEC not available | O_CLOEXEC undeclared | Remove flag, use fcntl after | vte291.yaml | FD_CLOEXEC=1=O_WRONLY on IRIX — NEVER substitute |
| MOGRIX_NO_DLMALLOC misapplied | Unnecessary libc malloc | Only for IRIX native .so apps | nedit.yaml | gtkterm had this wrong |
| WebKit "WebProcess CRASHED" is IPC loss | Not a memory fault — IPC socket close | IPC debug logging | webkitgtk.yaml | SIGTERM cleanup not SIGSEGV. See methods/webkit-debug.md |
| WebKit/ir8 IRIX memory optimization | Google.com OOMs, heavy JS sites exhaust RAM | Memory pressure + cache + GC tuning | methods/webkit-memory.md | ir8/main.c, bundle.py |
| IRIX IPC primitives all work | socketpair, SCM_RIGHTS, poll all work | No fix needed | N/A | **NEGATIVE**: Not the cause of WebKit IPC failures |
| WebKit Content Filtering not compiled | ENABLE(CONTENT_FILTERING) is Apple/Cocoa only | No fix needed | N/A | **NEGATIVE**: Not a blocker on GTK builds |
| WebKit Content Extensions no-op | ENABLE(CONTENT_EXTENSIONS) ON for GTK but no rules loaded | No fix needed | N/A | **NEGATIVE**: Default config has no extension rules, code is a no-op |
| WebKit CORS preflight not triggered for simple GET | Simple navigations don't trigger OPTIONS | No fix needed for basic HTTP | N/A | **NEGATIVE**: Only triggers for cross-origin XHR with custom headers |
| WebKit GLib main loop CONFIRMED working | GLib async (soup_session_send_async) uses GMainLoop which is cross-compiled | No fix needed | N/A | **NEGATIVE**: Confirmed working — soup callbacks fire correctly. Not the cause of any HTTP issue |
| GNetworkMonitor NULL on IRIX (GENERAL) | g_network_monitor_get_default() returns NULL — no /proc/net, no netlink on IRIX. g_signal_connect() on NULL crashes | NULL-check before use | webkitgtk.yaml (example) | **GENERAL**: ANY GIO app checking network status will crash. Not just WebKit |
| GLib async callbacks may silently vanish (GENERAL) | GLib async operations (GTask, gio async I/O) may never fire completion callbacks if underlying IRIX infrastructure is missing | Force synchronous path or bypass | webkitgtk.yaml disk cache bypass | **GENERAL**: No crash, no error, no timeout. Request enters async op and never returns. Diagnosed in WebKit disk cache |
| Silent-fail-closed security models (GENERAL) | Apps with sandbox/permission checks that fail closed without logging. If sandbox doesn't exist (IRIX), allowlists are empty, checks always fail | Bypass security checks entirely | webkitgtk.yaml MESSAGE_CHECK bypass | **GENERAL**: dbus, polkit, any sandbox-aware app. Pattern: check fails → silent return or process termination, no stderr |
| Multi-step debugging: dark territory | Works in config A, fails in config B, don't know where | Step-mapping analysis | methods/step-mapping.md | Start from known-good, map shared path, light from both ends |
| IRIX shm_open creates files at / | Permission denied, SharedMemory fails | Compat shm_open | compat/sys/shm_open.c | Redirects to /tmp/.shm_name |
| IRIX par tracing limitations | Can't trace fork+exec children, adds 50x overhead | Use C signal_catcher or IPC logging | methods/irix-testing.md | |
| Shell wrapper killed by process group | MiniBrowser kills entire pgrp, wrapper dies | C fork wrapper with setpgid(0,0) | N/A | signal(SIGTERM,SIG_IGN) in parent |
| RPM spec # in prep commands | # becomes comment in generated spec | Use perl not sed for #include/#define | webkitgtk.yaml | perl regex preserves # |
| YAML percent in prep commands | %d/%s causes YAML parse error | Move format strings to .h macros | webkitgtk.yaml, ipc_debug_log.h | |
| WEBKIT_DEBUG / G_MESSAGES_DEBUG useless | No extra debug output at runtime | Compile-time flags needed | webkitgtk.yaml | **NEGATIVE**: Requires DEVELOPER_MODE |
| IRIX struct dirent no d_type | d_type, DT_REG undeclared | prep_commands | telescope.yaml | stat() + S_ISREG/S_ISDIR instead |
| IRIX clock_gettime crashes | SIGBUS, CLOCK_MONOTONIC undefined | prep_commands | telescope.yaml | Replace with gettimeofday() |
| IRIX lacks timersub macro | timersub() undeclared | **GLOBAL** | cross/include/dicl-clang-compat/sys/time.h | Also timeradd, timercmp |
| IRIX scandir selector signature | const mismatch | prep_commands | rules/packages/* | Remove const from selector |
| IRIX _SGIAPI is a macro expression | #ifndef _SGIAPI always FALSE | Use #if !_SGIAPI | alpine.yaml | |
| NEVER #define _SGIAPI literal | blkcnt64_t unknown, stat64 visible | push_macro/pop_macro | cross/include/dicl-clang-compat/sys/socket.h | Destroys expression-macro |
| IRIX scandir hidden by _XOPEN_SOURCE | scandir/alphasort undeclared | Add guarded prototypes | alpine.yaml | #if !_SGIAPI guard |
| IRIX lacks open_memstream | open_memstream() undeclared | inject_compat_functions | compat/stdio/open_memstream.c | Uses funopen internally |
| IRIX lacks dprintf | dprintf(fd,fmt,...) undeclared | inject_compat_functions | compat/stdio/dprintf.c | Also vdprintf |
| IRIX lacks IOV_MAX | IOV_MAX undeclared | **GLOBAL** | cross/include/dicl-clang-compat/limits.h | |
| MAP_ANON not on IRIX | MAP_ANON/MAP_ANONYMOUS undeclared | prep_commands | rules/packages/* | Define MAP_ANON=0, use /dev/zero fd |
| IRIX getentropy via /dev/urandom | getentropy()/getrandom() link error | prep_commands | rules/packages/* | open /dev/urandom + read |
| libretls exports compat functions | configure falsely detects reallocarray etc | ac_cv_overrides | rules/packages/* | Force ac_cv_func_X="no" |
| Cross-compile HOSTCC pattern | Build fails running MIPS binary | spec_replacements | telescope.yaml | HOSTCC=gcc for generators |
| Bundled library cross-compilation | Bundled lib compiles for host | spec_replacements | rules/packages/* | Fix config.mk to use cross CC |
| AC_CHECK_FILES cross-compile | "cannot check for file existence" | ac_cv_overrides | cmatrix.yaml | ac_cv_file_path: "no" |
| C++ va_list type mismatch | conflicting types vfprintf | **GLOBAL** | cross/include/dicl-clang-compat/stdarg.h | IRIX va_list=char*, clang=void* |
| Static select() duplicate symbol | duplicate symbol: select | **GLOBAL** | cross/include/dicl-clang-compat/sys/time.h | _NO_XOPEN4=1 _NO_XOPEN5=1 |
| Staging/chroot library mismatch | SIGABRT no error after lib loading | Redeploy staging libs to chroot | methods/irix-testing.md | Compare file sizes to detect |
| FONTCONFIG_FILE for non-chroot apps | SGUG-RSE config → SIGABRT or wrong fonts | FONTCONFIG_FILE env var | Bundle wrappers | |
| gnulib SIG_ATOMIC_MAX missing | SIG_ATOMIC_MAX undeclared | **GLOBAL** | compat/include/mogrix-compat/generic/stdint.h | |
| gnulib signal.h vs IRIX | redefinition sigaction, NSIG verify | ac_cv_overrides + safepatch | libpipeline.yaml | Force sigaction/sigprocmask="yes" |
| IRIX lacks /proc/self/fd — GLib fdwalk | GLib g_fdwalk_set_cloexec() and g_spawn_async() enumerate fds via /proc/self/fd. IRIX has no /proc. Brute-force fallback clobbers IPC sockets | Package-specific workaround | webkitgtk.yaml | **GENERAL**: Any GLib app spawning children may hit this. WebKit fix: skip SetCloexecOnClient. Other apps: may need to skip fdwalk entirely |
| gnulib clearenv/setenv detection | Build uses wrong fallback | ac_cv_overrides | libpipeline.yaml | clearenv:"no", setenv:"yes" |
| _XOPEN_SOURCE hides math.h | expm1, log1p, cbrt undeclared in C++ | **GLOBAL** | cross/include/dicl-clang-compat/math.h | |
| IRIX sys/param.h TICK macro | narrowing conversion, TICK collision | prep_commands (sed) | icu.yaml | #define TICK 10000000 |
| Fedora %{gsub} macro | rpmbuild Macro %gsub not found | spec_replacements | icu.yaml | Hardcode the result |
| ICU cross-compilation two-stage | no --with-cross-build | spec_replacements | icu.yaml | Build host ICU first, unset cross vars |
| genccode cross-endian assembly | only same-endianness ELF supported | spec_replacements | icu.yaml | sed GENCCODE_ASSEMBLY=-a gcc. BUT causes word-swap |
| ICU data endianness word-swap | U_INVALID_FORMAT_ERROR, ubrk_open NULL | spec_replacements (post-make) | icu.yaml | genccode -a gcc word-swaps. Fix: icupkg -tb + .incbin asm |
| %{__global_ldflags} undefined | Literal string passed to linker | spec_replacements | figlet.yaml, sl.yaml | |
| GnuPG --with-lib*-prefix | gpg-error-config not found | configure_flags: add | rules/packages/* | --with-libgpg-error-prefix |
| drop_subpackages orphaned scriptlets | %post/%postun left behind | spec_replacements | cmatrix.yaml | |
| Clang 18 NULL int-conversion | incompatible integer to pointer | -Wno-int-conversion | mksh.yaml | IRIX NULL is 0L not (void*)0 |
| Custom build script cross-compile | Build.sh not autotools/cmake | spec_replacements + export CC | mksh.yaml | |
| pselect() undeclared | use of undeclared identifier pselect | compat header + lib | compat/stdlib/pselect.c | |
| posix_spawn link errors | undefined symbol posix_spawn | compat lib | compat/runtime/spawn.c | |
| Link order: -l before objects | Linker errors from static archives | move -l after $libs | ninja-build.yaml | |
| drop_subpackages unexpanded macros | Doesn't match RPM macros | glob pattern | brotli.yaml | Use "python*" not literal |
| xnedit BadMatch X_CreateWindow | BadMatch launching GUI on IRIX | add_patch | xnedit.yaml | Patch FindBestVisual() to force 24-bit |
| IRIX bsearch nmemb=0 crash | SIGSEGV bsearch() empty array | inject + libmogrix_compat.so | compat/stdlib/bsearch.c, generic.yaml | inject alone NOT enough — rld no exe preemption |
| IRIX rld no exe symbol preemption | compat in exe doesn't override for .so | libmogrix_compat.so + _RLDN32_LIST | mogrix/bundle.py | See methods/compat-functions.md |
| Bundler missing unversioned soname symlinks | Falls back to ancient IRIX lib | bundle.py fixes | mogrix/bundle.py | _create_soname_symlinks() |
| Bundler IRIX sysroot lib priority wrong | Uses IRIX lib not mogrix-built | bundle.py dep resolution order | mogrix/bundle.py | Check _soname_to_rpm before _irix_sonames |
| C++ vtable R_MIPS_REL32 SIGSEGV | Many virtual methods, UNDEF relocs | Static link the C++ lib | dillo.yaml | -Wl,-Bstatic -lfltk -Wl,-Bdynamic |
| -fcommon for pre-GCC10 C | duplicate symbol from tentative defs | export_vars CFLAGS | dillo.yaml | GCC 10+ defaults -fno-common |
| Dillo no SNI → wrong cert | "self-signed cert", handshake failure | add_patch (SSL_set_tlsext_host_name) | patches/packages/dillo/ | |
| Dillo dpid can't find DPI plugins | "Can't find directory for dpis" | bundle wrapper | mogrix/bundle.py | Writes ~/.dillo/dpidrc + ~/.dillo/dpid |
| WebKit IPC Semaphore Linux-only (eventfd) | HTTP never loads, file:// works. NetworkProcess idle | Pipe-based semaphore replacement | patches/packages/webkitgtk/IPCSemaphoreUnix_irix.cpp, webkitgtk.yaml | |
| WebKit NP silent blockers (5 found, ALL BYPASSED) | HTTP blank page, no crash/log/error. NP receives request but never calls libsoup | prep_commands bypass | webkitgtk.yaml, docs/webkit-silent-blocker-audit.md | **RESOLVED**: See methods/webkit-silent-blockers.md |
| WebKit HTTP disk cache async callback | canUseCache() true → m_cache->retrieve() async callback never fires on IRIX | canUseCache() always returns false | webkitgtk.yaml | Request vanishes in cache lookup. No crash/error/timeout |
| WebKit allowsFirstPartyForCookies | Cookie domain check silently kills HTTP loads (TerminateWebProcess) | Bypass all 11 instances | webkitgtk.yaml | Sandbox allowlist empty on IRIX for IP URLs |
| WebKit SW import gate | isImportCompleted() never true → load deferred forever | Condition set to `false` | webkitgtk.yaml | ENABLE(SERVICE_WORKER) ON for GTK, SW DB never initializes |
| WebKit SW load routing | startWithServiceWorker() blocks before startRequest() | Changed to start() | webkitgtk.yaml | createFetchTask+content filtering silently block |
| WebKit MESSAGE_CHECK macro | NETWORK_PROCESS_MESSAGE_CHECK silently returns + sends TerminateWebProcess | Bypass security checks | webkitgtk.yaml | No crash, no stderr, no callback. Fail-closed sandbox design |
| WebKit RELEASE_LOG_FAULT invisible | RELEASE_LOG_FAULT goes to Apple unified logging, not stderr | Compile-time DIAG | webkitgtk.yaml | **NEGATIVE**: Cannot rely on RELEASE_LOG for IRIX debugging |
| WebKit sandbox model on IRIX | UIProcess populates allowlists; NP enforces with silent-fail-closed | Bypass NP security checks | webkitgtk.yaml | No sandbox on IRIX = allowlists empty/wrong |
| WebKit GNetworkMonitor NULL crash | g_signal_connect on NULL GNetworkMonitor → crash | NULL-check before signal connect | webkitgtk.yaml | GNetworkMonitor not available on IRIX |
| WebKit HTTP disk cache + GNetworkMonitor | Bypass #4 and #5 complete. All 5 HTTP bypasses applied | prep_commands bypass | webkitgtk.yaml | **RESOLVED**: HTTP rendering works on IRIX (bundle 0223261927) |
| WebKit HTTP verified WORKING on IRIX | Full response chain: soup_send → didSendRequest(200) → dispatchResp → ContinueDidReceiveResponse round-trip → readCB → didFinish | All 5 bypasses in webkitgtk.yaml | webkitgtk.yaml | Tested against HTTP server, user-confirmed. 33 DIAG tags verified chain |
| WebKit first-load transient failure | First HTTP load gets GIO error 4, retry succeeds | No fix needed | N/A | **NEGATIVE**: Transient — server not ready or DNS hiccup. WebKit auto-retries |
| WebKit WebProcess NULL+220 crash | SIGSEGV at NULL+220 in libwebkit2gtk, WP gets respawned | No fix needed | N/A | **NEGATIVE**: Pre-existing, cosmetic. Does not affect HTTP loading |
| WebKit ContinueDidReceiveResponse round-trip | For main resource loads, NP sends DidReceiveResponse IPC to WP and WAITS for WP reply before reading body | Not a bug — critical architecture knowledge | docs/webkit-ipc-flow-map.md | If body data never arrives, check this round-trip. DIAG tags: NP_loader_willWait, NP_sendDidRecvResp, NP_contDidRecvResp |
| SOUP_DEBUG env var useless in WebKit | SOUP_DEBUG=1 produces no output from NetworkProcess | Compile-time DIAG | webkitgtk.yaml | **NEGATIVE**: Like WEBKIT_DEBUG, requires DEVELOPER_MODE build flags |
| IRIX interposing _exit() crashes | SIGBUS BUS_ADRALN in exit() | Cannot safely interpose _exit() on IRIX | patches/shared/mogrix_crash_handler.c | rld doesn't set $t9/GP for interposed symbols |
| WebKit HTTPS verified WORKING on IRIX | Full TLS chain: TLS_HANDSHAKING → TLS_HANDSHAKED → status=200 → data delivery → didFinish | Three fixes: DEVELOPER_MODE bypass, WEBKIT_TLS_CAFILE_PEM wrapper export, bundle pruning fix | webkitgtk.yaml, mogrix/bundle.py | Bundle `0223262218`. Screenshot confirmed. 11 TLS DIAG tags |
| Bundle pruning dangling symlinks (GIO modules) | rld "Cannot Successfully map soname" for libs needed by dlopen'd GIO modules | Protect symlink targets + deps-of-deps in bundle.py pruning | mogrix/bundle.py | libgiognutls.so → libgnutls.so.30 → libtasn1, libnettle, libhogweed, libgmp. Sonames added to `needed` but symlink targets were pruned |
| WEBKIT_TLS_CAFILE_PEM behind DEVELOPER_MODE | HTTPS fails silently — GnuTLS can't find CA certs at compiled-in path | Remove `#if ENABLE(DEVELOPER_MODE)` guard via prep_commands | webkitgtk.yaml | Wrapper sets env var to bundle CA file. GnuTLS ignores SSL_CERT_FILE (OpenSSL-only) |
| soup_session_get_tls_database is soup3 only | Undeclared function error in soup2 build | Use `g_object_get(G_OBJECT(session), "tls-database", &db, NULL)` | webkitgtk.yaml | SOUP2 API doesn't expose this directly |

---

## Platform Invariants

| Fact | Implication | Reference |
|------|-------------|-----------|
| IRIX chroot doesn't isolate | Binaries see base system paths | methods/irix-testing.md |
| brk() heap limited to 176MB | dlmalloc uses mmap (1.2GB) instead | methods/irix-address-space.md |
| Volatile fptr initializers crash | `static volatile fptr = memset;` relocation fails | compat/string/explicit_bzero.c |
| IRIX native tar drops long paths | Use gtar from chroot | mogrix/bundle.py |
| Shell scripts must use full paths | DIDBS/SGUG-RSE shadow IRIX commands | mogrix/bundle.py |
| IRIX cp -r breaks on symlinks | Use tar pipe instead | IRIX host deployment |
| Staging ≠ chroot until deployed | mogrix stage only updates staging. Redeploy to chroot | methods/irix-testing.md |
| SGUG-RSE paths leak at runtime | Set FONTCONFIG_FILE, GIO_MODULE_DIR, GDK_PIXBUF_MODULE_FILE | Bundle wrappers |
| GIO module cache controls loading | Stale cache = modules ignored. Run gio-querymodules | Bundle wrappers |
| IRIX host shell is csh | VAR=value command fails. Use env VAR=value | sh scripts |
| IRIX /bin/sh lacks $() | Use backtick substitution, not $(...). Also no $(()) — use `expr` | ir8_test.sh |
| IRIX /bin/sh date lacks %s | No epoch seconds. Use `date +%H:%M:%S` + expr to compute elapsed | ir8_test.sh |
| IRIX grep lacks \\| alternation | Use `egrep 'a|b'` not `grep 'a\|b'`. Native grep treats \\| as literal | ir8_test.sh |
| Old /opt/cross/bin/irix-ld broken | Produces MIPS_OPTIONS → rld crash. Use staging irix-ld | /opt/sgug-staging/usr/sgug/bin/irix-ld |
| Clang 16/18 MIPS 56GB memory bug | cc1 uses 56GB compiling large TUs with inline asm (LLInt = 57K lines). x86=95MB, frontend=126MB, llc=42MB. Bug in cc1 integrated asm for MIPS. Workaround: two-step compile (clang -emit-llvm + llc -filetype=obj). Total 168MB | patches/packages/webkitgtk/compile-llint-twostep.sh |
| LLInt offline asm emits MIPS32 `mul` | `mul rd,rs,rt` is MIPS32, not MIPS III/IV. Fix: `mult rs,rt` + `mflo rd` in offlineasm/mips.rb. Without fix, llc aborts (SIGABRT) | rules/packages/webkitgtk.yaml |
| JSC big-endian JSValue32 tag/payload swap | `storePair32`/`loadPair32` arg order assumes little-endian. 10 functions in AssemblyHelpers.h need swapped args for big-endian. 7 have `static_assert` guards (compile error), 3 silently corrupt (`loadValue(void*)`, both `storeTrustedValue`). Fix: `#if __BYTE_ORDER__` conditional in patch | patches/packages/webkitgtk/jit-bigendian-jsvalue32.patch |
| LLInt `.cpload` O32 PIC on N32 | `.cpload` is O32 PIC directive — sets up `$gp` at each label. N32: `$gp` is callee-saved (set in prologue, stays valid). `.cpload` on N32 generates incorrect `_gp_disp` references. Fix: `#if defined(WTF_MIPS_PIC) && !defined(__sgi)` in LLIntOfflineAsmConfig.h | rules/packages/webkitgtk.yaml |
| LLInt push/pop 4-byte vs 8-byte alignment | Offline asm push/pop uses `addiu $sp, ±4` (4-byte). N32 ABI requires 8-byte minimum stack alignment at call boundaries. Fix: change to ±8 in mips.rb. Wastes 4 bytes per op but guarantees alignment | rules/packages/webkitgtk.yaml |
| LLInt two-step wrapper + ccache | cmake RULE_LAUNCH_COMPILE passes `ccache compiler flags`. Wrapper $1 is ccache, not the compiler. Must detect `*/ccache` and shift. Step 3 (asm→obj) must skip ccache — use compiler directly. Remove `\|\| true` to avoid silent failures; use file-existence checks instead | patches/packages/webkitgtk/compile-llint-twostep.sh |
| LLInt three-step compile: .size/.file/$tmp stripping | llc generates .size directives with non-absolute expressions (LLVM MIPS bug), GNU as places .file/FILE symbol after SECTION symbols corrupting symtab sh_info, and $tmp labels as LOCAL symbols interleaved with GLOBALs. Fix: sed strip all three from assembly before assembling | patches/packages/webkitgtk/compile-llint-twostep.sh |
| LLInt three-step compile: N32 vs mips64 triple | llc with -mtriple=mips64-sgi-irix6.5 generates 64-bit address relocations (R_MIPS_HIGHER/HIGHEST) even with p:32:32 data layout. Fix: use -target-abi n32 to get proper N32 code (32-bit addresses, 64-bit registers) | patches/packages/webkitgtk/compile-llint-twostep.sh |
| LLInt three-step compile: GOT overflow (xgot) | libjavascriptcoregtk GOT exceeds 64KB. llc generates %got() and %got_disp() with R_MIPS_GOT16/GOT_DISP (16-bit) that overflow. Clang natively uses GOT_PAGE/GOT_OFST pairs. Fix: perl post-process to convert %got()/%got_disp() to %got_hi/%got_lo pairs (R_MIPS_GOT_HI16/GOT_LO16) | patches/packages/webkitgtk/compile-llint-twostep.sh |
| LLInt three-step compile: PIC flags | GNU as needs -KPIC flag to set pic/cpic ELF flags matching clang-compiled objects | patches/packages/webkitgtk/compile-llint-twostep.sh |
| GLib 2.80 n32 G_DEFINE_TYPE crash | `G_STATIC_ASSERT` in `g_once_init_enter` fails (gsize!=gpointer) | Version cap + fixup header | methods/glib-n32-compat.md | Affects ALL GTK/GLib apps on n32 |
| Hand-written specs need CC export | Link fails with "skipping incompatible" — host gcc used instead of cross-compiler | spec_replacements: export CC | methods/makefile-builds.md | `%make_build` doesn't export CC unlike `%configure` |

### Engine Bugs & Gotchas

| Bug | Workaround | Reference |
|-----|------------|-----------|
| extra_cflags / skip_manpages / make_env dead code | Use prep_commands or export_vars | mogrix/rules/validator.py |
| install_cleanup misplacement | Use spec_replacements if %triggerpostun precedes %files | mogrix/rules/engine.py |
| remove_conditionals can't nest | Use bcond flipping | mogrix/rules/engine.py |
| drop_subpackages must be inside rules: | Only add_patch/add_source at top-level | rules/packages/*.yaml |
| drop_subpackages ignores scriptlets | Use spec_replacements for %post/%postun | mogrix/emitter/spec.py |
| inject_compat_functions affects HOST link | Bootstrap packages build HOST tools | flex.yaml |
| Perl drop_subpackages glob trap | "*" matches -f in %files. Use "[A-Za-z]*" | perl.yaml |
| cmake finds staging binaries | Set -DCMAKE_MAKE_PROGRAM=/usr/bin/make | weechat.yaml |

### Bundle & Wrapper

| Fact | Implication | Reference |
|------|-------------|-----------|
| Shell wrapper recursion | Bundled dirname/pwd shadow /bin/. Use absolute paths | mogrix/bundle.py |
| LD_LIBRARYN32_PATH contamination | Set fresh, not prepend | mogrix/bundle.py |
| SSL_CERT_FILE is OpenSSL-only | gnutls ignores it. Use WEBKIT_TLS_CAFILE_PEM for WebKit/GnuTLS | mogrix/bundle.py |
| WEBKIT_TLS_CAFILE_PEM for GnuTLS CA certs | Points soup session TLS database to bundle CA file | mogrix/bundle.py |
| Bundle pruning must protect dlopen'd module deps | GIO modules loaded via dlopen — their deps not in NEEDED chain | mogrix/bundle.py |
| Bundle must include libz.so | IRIX ships zlib 1.1.4; modern libpng needs 1.2+ | xscreensaver-gl-hacks |

## Debugging: Crash Handler

| Keyword | Description | Fix | Source |
|---------|-------------|-----|--------|
| silent crash / no stderr | GSubprocess swallows stderr | MOGRIX_CRASH_DEBUG=1 MOGRIX_CRASH_DIR=/usr/people/edodd | patches/shared/mogrix_crash_handler.c |
| SIGPIPE silent kill | MSG_NOSIGNAL=0 on IRIX. Do NOT catch SIGPIPE in crash handler | Removed from signal list in session 105 | patches/shared/mogrix_crash_handler.c |
| mogrix_init_<pid>.log missing | Crash handler didn't load | Check _RLDN32_LIST and libmogrix_compat.so | mogrix/bundle.py |
| mogrix_exit_<pid>.log only | Process called exit(), not crashed | Check parent IPC logs | patches/shared/mogrix_crash_handler.c |
| Recompiling libmogrix_compat.so | After editing compat sources | See methods/compat-functions.md for build command | compat/, patches/shared/ |

## Anti-Patterns

| Anti-Pattern | Do This Instead |
|-------------|-----------------|
| Inline C in prep_commands (heredocs) | Put C in patches/packages/, use add_source + cp |
| Fixes outside mogrix rules | Store in package YAML or generic.yaml |
| Staging libs without chroot redeploy | Compare sizes, redeploy changed .so |
| Duplicating generic.yaml rules | Check GENERIC_SUMMARY.md first |
| Ad-hoc IRIX wrapper scripts | Use mogrix-test MCP tools |
| Assuming /tmp writable on IRIX host | Use /usr/people/edodd/tmp/ |
| Large builds as background Bash | Use haiku sub-agents. See methods/task-tracking.md Rule 6-7 |
| Retrying failed builds in loop | Fix root cause first. Max 1 retry for large packages |
| mogrix convert <package-name> | convert takes a FILE PATH to SRPM |
| Heredocs in irix_exec | Write file locally, irix_copy_to + irix_host_exec cp |
| irix_copy_to = host filesystem | It copies to CHROOT. Chain with irix_host_exec cp |
| Rebuilding libmogrix_compat.so abbreviated | ALWAYS use full build command from methods/compat-functions.md |
| Assuming WebKit silent failures are crashes | Check for MESSAGE_CHECK macros, deferred callbacks, #if ENABLE() gates |
| Bypassing one instance of a pattern | Search for ALL instances (grep the whole file/directory). See webkit cookie bypass |
| Debugging WebKit with RELEASE_LOG output | Use compile-time MOGRIX_DIAG instead. RELEASE_LOG goes to Apple unified logging |
| Inserting DIAG into bare if/else body | Insert BEFORE the if statement, not into its body. Braceless `if (x) stmt; else stmt;` — inserting before stmt orphans the else. Use perl -0777 multiline match |
| chown -R on bundles with symlinks | Use `chown -Rh` (no-follow). `-R` fails on broken symlinks with ENOENT. Bundles always have symlinks |

---

## Detailed Methods (moved from INDEX)

| Topic | Location |
|-------|----------|
| X.org extension cross-compilation | methods/xorg-cross.md |
| WebKit IPC debugging | methods/webkit-debug.md |
| WebKit silent blockers (NP) | methods/webkit-silent-blockers.md |
| Git/Makefile builds, bundler details | methods/makefile-builds.md |
| IRIX testing & chroot | methods/irix-testing.md |
| Compat functions | methods/compat-functions.md |
| Text replacement (safepatch/sed) | methods/text-replacement.md |
| Patch creation | methods/patch-creation.md |
| Task tracking & agents | methods/task-tracking.md |
| Upstream packages | methods/upstream-packages.md |
| Step-mapping analysis | methods/step-mapping.md |

## Skipped Packages

| Package | Reason |
|---------|--------|
| gdb | All IRIX debug support removed. SGUG-RSE used 7.6.2 |
| htop | Needs Linux /proc backend |
| openjpeg2 | No SRPM available |

## Rule Hierarchy

Rules applied in order: **generic → class → package**. See `rules/generic.yaml`, `rules/classes/*.yaml`, `rules/packages/*.yaml`. Run `mogrix audit-rules` for duplication detection.
