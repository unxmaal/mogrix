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
| TLS not supported | __thread keyword, rld link failure | add_patch | patches/packages/* | Remove __thread, accept single-thread |
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
| Spec conditionals | %if blocks for wrong platform | comment_conditionals | rules/packages/* | Comments out entire %if...%endif block |
| Extra compiler flags | Need -D or -I flags for build | extra_cflags | rules/packages/* | Added to CFLAGS in %build |
| Make environment | Need env vars for make | make_env | rules/packages/* | Exported before make commands |
| Skip man pages | Man pages cause install errors | skip_manpages | rules/packages/* | Removes man install from spec |
| Skip lang files | %files -f lang.txt fails | files_no_lang | rules/packages/* | Removes -f lang from %files |

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
| brk() heap limited to 176MB | libpthread at 0x0C080000 blocks heap growth | methods/irix-address-space.md |
| mmap-based malloc bypasses limit | dlmalloc in compat uses mmap, 1.2GB available | compat/malloc/dlmalloc.c |

## File Locations

| Category | Path | Contents |
|----------|------|----------|
| Generic rules | rules/generic.yaml | Applied to ALL packages |
| Package rules | rules/packages/*.yaml | Per-package overrides |
| Compat functions | compat/catalog.yaml | Function registry |
| Compat sources | compat/runtime/, compat/dicl/, compat/stdio/ | Implementation files |
| Compat headers | compat/include/mogrix-compat/generic/ | Header wrappers |
| Patches | patches/packages/*/ | Source patches by package |
| Methods | rules/methods/*.md | Process documentation |
