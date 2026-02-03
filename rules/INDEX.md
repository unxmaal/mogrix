# Mogrix Rules Index

| Problem class | Symptoms / triggers | Rule mechanism | Rule location | Notes / constraints |
|---------------|---------------------|----------------|---------------|---------------------|
| Before you start | Stuck, confused, debugging | Read checklist | methods/before-you-start.md | Check SGUG-RSE, git history, clean build |
| Linux-only build deps | systemd-devel, selinux, elogind, audit | drop_buildrequires | rules/generic.yaml | Always drop, never patch |
| Linux-only runtime deps | systemd-libs, selinux, audit-libs | drop_requires | rules/packages/* | Cross-compiled pkgs need explicit deps |
| Autoconf cross-detect | configure misdetects headers/funcs | ac_cv_overrides | rules/packages/* | Prefer override to source patch |
| Missing libc functions | setenv, strsep, getline, asprintf | inject_compat_functions | rules/packages/* | Check compat/catalog.yaml for available |
| Missing POSIX.1-2008 | openat, fstatat, mkdirat, utimensat | inject_compat_functions | rules/packages/* | dicl/openat-compat.c has 17 *at funcs |
| Missing BSD functions | funopen, strlcpy, getprogname | inject_compat_functions | rules/packages/* | funopen works, fopencookie crashes |
| GNU-only headers | execinfo.h, malloc.h, error.h | header_overlays | rules/generic.yaml | Stubs acceptable |
| Libtool cross-detect | libtool says "unknown platform" | spec_replacements | rules/packages/* | Use fix-libtool-irix.sh after configure |
| CMake cross-compile | find_package fails, wrong paths | spec_replacements | rules/packages/* | Set CMAKE_FIND_ROOT_PATH, disable tests |
| RPM macro pollution | %configure clobbers cross flags | configure_flags: remove | rules/packages/* | Or use spec_replacements to fix |
| TLS not supported | __thread keyword, rld link failure | add_patch | patches/packages/* | Remove __thread, accept single-thread |
| vsnprintf pre-C99 | garbled output, truncation | inject_compat_functions: vasprintf | rules/packages/* | IRIX vsnprintf(NULL,0) returns -1 |
| Linker selection | .so crashes rld, bad relocations | export_vars: LD | rules/packages/* | GNU ld for -shared, LLD 18 for exe |
| Path conventions | /etc vs /usr/sgug/etc | spec_replacements | rules/packages/* | SGUG uses /usr/sgug prefix |
| Subpackage bloat | -debuginfo, -langpacks, -doc | drop_subpackages | rules/generic.yaml | Cross-compile doesn't need these |
| Scriptlet failures | ldconfig, systemd macros | spec_replacements | rules/packages/* | Make ldconfig no-op via macros.ldconfig |
| Man page compression | .1.gz vs .1 in %files | spec_replacements | rules/packages/* | Cross-build doesn't compress |
| Test suite | %check fails (can't run MIPS) | skip_check: true | rules/packages/* | Tests can't run on build host |
| NLS/gettext | %find_lang fails | skip_find_lang: true | rules/packages/* | Often disable NLS entirely |
| Missing runtime deps | tdnf install says "not found" | add_requires | rules/packages/* | AutoReq disabled for cross-compile |
| Disable features | ldap, nls, tests not needed | configure_disable | rules/packages/* | Adds --disable-X to configure |
| Add configure flags | Need --with-X or custom flags | configure_flags: add | rules/packages/* | Adds flags to %configure |
| Unwanted install files | .la files, duplicate docs | install_cleanup | rules/packages/* | Commands run after %make_install |
| Legacy libdicl lines | CPPFLAGS/LIBS with libdicl | remove_lines | rules/packages/* | SGUG-RSE specs have these, remove them |
| Spec conditionals | %if blocks for wrong platform | comment_conditionals | rules/packages/* | Comments out entire %if...%endif block |
| Path translation | /usr/lib64 in spec needs /usr/sgug/lib32 | rewrite_paths | rules/generic.yaml | String replacement in spec content |
| RPM macro overrides | _prefix, _libdir, _arch wrong | rpm_macros | rules/generic.yaml | Defines macros at top of spec |
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
