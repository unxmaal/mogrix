# Mogrix Rules Index

## Check generic.yaml First

**Before adding rules to a package YAML**, check what `rules/generic.yaml` already provides.
Generic rules are applied to EVERY package automatically. Do NOT duplicate them in package files.

**generic.yaml already handles:**

| What | Mechanism | Details |
|------|-----------|---------|
| Test suite | `skip_check: true` | Can't run MIPS binaries on build host |
| Linux-only build deps | `drop_buildrequires` | systemd, systemd-devel, libselinux-devel, kernel-headers, systemtap-sdt-devel, libcap-devel, libcap-ng-devel, libacl-devel, libattr-devel, audit-libs-devel, alsa-lib-devel, gnupg2, libdicl, libdicl-devel |
| Linux-only runtime deps | `drop_requires` | libdicl, libdicl-devel |
| libdicl removal | `remove_lines` | CPPFLAGS/LIBS exports, %gpgverify, %ldconfig_scriptlets, systemd scriptlets |
| malloc(0) cross-detect | `ac_cv_overrides` | ac_cv_func_malloc_0_nonnull, ac_cv_func_realloc_0_nonnull |
| SGUG prefix paths | `rpm_macros` | _prefix=/usr/sgug, _libdir=/usr/sgug/lib32, _bindir, _includedir, etc. |
| Path translation | `rewrite_paths` | /usr/lib64 → /usr/sgug/lib32, /usr/lib → /usr/sgug/lib32, /usr/include → /usr/sgug/include |
| Linux-only features | `configure_disable` | selinux, systemd, udev, inotify, epoll, fanotify, timerfd, libcap, audit |
| Header stubs | `header_overlays: generic` | execinfo.h, malloc.h, error.h, etc. |
| Subpackage bloat | `drop_subpackages` | debuginfo, debugsource, langpack-* |

**Only add to a package YAML** when the package needs something beyond the above.

---

## Per-Package Problem Reference

| Problem class | Symptoms / triggers | Rule mechanism | Rule location | Notes / constraints |
|---------------|---------------------|----------------|---------------|---------------------|
| Before you start | Stuck, confused, debugging | Read checklist | methods/before-you-start.md | Check SGUG-RSE, git history, clean build |
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
| X11 via IRIX native sysroot | Don't use `--x-includes`/`--x-libraries`; cross-compiler `--sysroot` finds X11 | rules/packages/aterm.yaml |
| IRIX X11R6.3 API limits | XICCallback→XIMCallback, XSetIMValues→no-op, Xutf8→Xmb, XUTF8→XCompound | rules/packages/st.yaml |
| IRIX has no TrueType fonts | Xft apps need bundled fonts; fontconfig `<dir prefix="relative">` for bundle paths | mogrix/bundle.py `_include_fonts()` |
| `_XOPEN_SOURCE=600` hides IRIX defs | winsize/TIOCSWINSZ/TIOCGWINSZ gated by `_NO_XOPEN5`; remove, use `_SGI_SOURCE` | rules/packages/st.yaml |
| setenv/unsetenv NOT in IRIX libc | `putenv()` only; compat wraps it. Must add to `inject_compat_functions` | compat/stdlib/setenv.c |
| X11 .pc files for IRIX native libs | libXrender/libXft configure use `pkg-config --exists x11/xext`; handcrafted .pc files in staging | /opt/sgug-staging/.../pkgconfig/ |
| Man pages not compressed | rpmmacros.irix disables brp scripts; use `*.1*` not `*.1.gz` in %files | rpmmacros.irix |
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
| SSL_CERT_FILE is OpenSSL-only | gnutls ignores it; use app-specific CA config (weechat: gnutls_ca_user) | mogrix/bundle.py |
| weechat 4.x renamed gnutls settings | `gnutls_ca_file` → `gnutls_ca_system` + `gnutls_ca_user` | rules/packages/weechat.yaml |

## File Locations

| Category | Path | Contents |
|----------|------|----------|
| Generic rules | rules/generic.yaml | Applied to ALL packages |
| Class rules | rules/classes/*.yaml | Shared rules for package groups |
| Package rules | rules/packages/*.yaml | Per-package overrides (119 packages) |
| Source checks | rules/source_checks.yaml | IRIX source pattern definitions |
| Compat functions | compat/catalog.yaml | Function registry + source patterns |
| Compat sources | compat/string/, compat/stdio/, compat/stdlib/, compat/dicl/, compat/malloc/ | Implementation files |
| Compat headers | compat/include/mogrix-compat/generic/ | Header wrappers (pty.h for openpty) |
| Bundle fonts | fonts/ | TTF fonts for X11 bundles (Iosevka Nerd Font) |
| X11 .pc files | /opt/sgug-staging/.../pkgconfig/ | x11.pc, xext.pc, xproto.pc, renderproto.pc |
| Patches | patches/packages/*/ | Source patches by package |
| rpmlint config | rpmlint.toml | IRIX-specific rpmlint filters |
| Source analyzer | mogrix/analyzers/source.py | Ripgrep-based source scanner |
| Rule auditor | mogrix/analyzers/rules.py | Duplication detection across packages |
| Spec validator | mogrix/validators/spec.py | Specfile structural validator |
| Methods | rules/methods/*.md | Process documentation |
| Bundle architecture | rules/methods/bundles.md | How bundles work end-to-end |

## Rule Hierarchy

Rules are applied in order: **generic → class → package**. Each layer adds to or overrides the previous.

- **generic.yaml** — Universal IRIX fixes (all packages get these)
- **classes/*.yaml** — Shared patterns for groups of packages (opt-in via `classes:` in package yaml)
- **packages/*.yaml** — Package-specific overrides

**Adding a class:** Create `rules/classes/<name>.yaml` with a `class:` key and `rules:` section. Packages opt in by adding `classes: [<name>]` to their top-level yaml.

**Available classes:**

| Class | Purpose | Packages |
|-------|---------|----------|
| `nls-disabled` | Disables NLS, skips %find_lang, removes lang file refs | gawk, grep, sed, flex, libpng, make, findutils, tar, fontconfig |

**Auditing for elevation:** Run `mogrix audit-rules` to detect duplicated rules across packages and flag candidates for promotion to class or generic level.

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
