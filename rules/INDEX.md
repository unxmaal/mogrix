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
| GNU ld linker scripts | `INPUT(-lfoo)` text files as .so | spec_replacements | rules/packages/* | IRIX rld can't load linker scripts; replace with symlinks |
| `%zu` format specifier | SIGSEGV in printf/snprintf/fprintf | add_patch or prep_commands | rules/packages/* | IRIX libc is pre-C99; `%zu` corrupts varargs → crash. Use `%u` instead. See methods/irix-quirks.md |
| dlmalloc + libc strdup | SIGSEGV in free/realloc | inject_compat_functions: strdup | rules/packages/* | Packages with dlmalloc must also inject strdup/strndup compat |
| Spec conditionals | %if blocks for wrong platform | comment_conditionals | rules/packages/* | Comments out entire %if...%endif block |
| Extra compiler flags | Need -D or -I flags for build | extra_cflags | rules/packages/* | Added to CFLAGS in %build |
| Make environment | Need env vars for make | make_env | rules/packages/* | Exported before make commands |
| Skip man pages | Man pages cause install errors | skip_manpages | rules/packages/* | Removes man install from spec |
| Skip lang files | %files -f lang.txt fails | files_no_lang | rules/packages/* | Removes -f lang from %files |
| X11 path detection | configure AC_PATH_XTRA fails cross | spec_replacements | rules/packages/* | Remove --x-includes/--x-libraries, sysroot autodetects |
| CJK font dependencies | k14, taipei16 fonts not on IRIX | configure_disable or spec_replacements | rules/packages/* | --disable-kanji --disable-big5 --disable-greek |
| Unpackaged doc files | make install + %doc both install docs | install_cleanup | rules/packages/* | Remove from %{buildroot}%{_pkgdocdir} |
| AUTOPOINT=true | autoreconf fails without autopoint | spec_replacements | rules/packages/* | `AUTOPOINT=true autoreconf -vfi` |

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
| Man pages not compressed | rpmmacros.irix disables brp scripts; use `*.1*` not `*.1.gz` in %files | rpmmacros.irix |
| AUTOPOINT=true for NLS-disabled | Packages with autoreconf + nls-disabled class need `AUTOPOINT=true` | rules/packages/*.yaml |

## File Locations

| Category | Path | Contents |
|----------|------|----------|
| Generic rules | rules/generic.yaml | Applied to ALL packages |
| Class rules | rules/classes/*.yaml | Shared rules for package groups |
| Package rules | rules/packages/*.yaml | Per-package overrides (96 packages) |
| Source checks | rules/source_checks.yaml | IRIX source pattern definitions |
| Compat functions | compat/catalog.yaml | Function registry + source patterns |
| Compat sources | compat/string/, compat/stdio/, compat/stdlib/, compat/dicl/, compat/malloc/ | Implementation files |
| Compat headers | compat/include/mogrix-compat/generic/ | Header wrappers |
| Patches | patches/packages/*/ | Source patches by package |
| rpmlint config | rpmlint.toml | IRIX-specific rpmlint filters |
| Source analyzer | mogrix/analyzers/source.py | Ripgrep-based source scanner |
| Rule auditor | mogrix/analyzers/rules.py | Duplication detection across packages |
| Spec validator | mogrix/validators/spec.py | Specfile structural validator |
| Methods | rules/methods/*.md | Process documentation |

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
