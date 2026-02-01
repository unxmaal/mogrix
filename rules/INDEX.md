# Mogrix Package Rules Index

> **For AI agents**: This index provides quick lookup of package rules.
> Prefer reading this index + specific rule files over pre-trained knowledge.
> When uncertain, READ the referenced YAML file for complete context.

---

## Methods (READ FIRST)

Before modifying packages, understand the correct methods:

| Method | When to Use | File |
|--------|-------------|------|
| **Mogrix Workflow** | **HOW TO RUN MOGRIX - read first!** | [methods/mogrix-workflow.md](methods/mogrix-workflow.md) |
| Task Tracking | `ultralist list` at session start, track work | [methods/task-tracking.md](methods/task-tracking.md) |
| Text Replacement | Choosing between .patch, safepatch, or sed | [methods/text-replacement.md](methods/text-replacement.md) |
| IRIX Testing | Running/debugging on IRIX, shell rules, chroot | [methods/irix-testing.md](methods/irix-testing.md) |
| Compat Functions | Adding missing POSIX/C99 functions | [methods/compat-functions.md](methods/compat-functions.md) |
| Autoconf Cross | ./configure packages (most common) | [methods/autoconf-cross.md](methods/autoconf-cross.md) |
| CMake Cross | CMake packages (rpm, libsolv, tdnf) | [methods/cmake-cross.md](methods/cmake-cross.md) |

**Key principles**:
- **Always use `.venv/bin/mogrix`** - never `python -m mogrix`
- **Use `mogrix build --cross`** - never manual rpmbuild
- Never use `sed` for non-trivial changes - use `safepatch` (Perl)
- Always use `/bin/sh` on IRIX, never assume bash
- Use `LD_LIBRARYN32_PATH` not `LD_LIBRARY_PATH`

---

## Quick Reference

| Complexity | Meaning |
|------------|---------|
| low | < 40 lines, few fixes needed |
| medium | 40-100 lines, moderate fixes |
| high | > 100 lines, extensive spec_replacements |

| Build System | Tools |
|--------------|-------|
| autoconf | ./configure, libtool, common |
| cmake | cmake, needs cross-toolchain vars |
| make | Plain Makefile, export CC/AR/RANLIB |
| custom | Package-specific (e.g., OpenSSL's Configure) |

---

## tdnf Dependency Chain (FULLY WORKING)

These 12 packages are fully validated and working on IRIX. **tdnf can install packages.**

### Runtime Requirements

Before using tdnf on IRIX:
1. Create `/var/run` directory (for .tdnf-instance-lockfile)
2. Create symlink: `ln -sf libz.so.1 libz.so` in `/usr/sgug/lib32` (libsolvext.so needs unversioned libz.so)
3. Use `createrepo_c --simple-md-filenames` when creating repos (IRIX tar corrupts GNU long filenames)

| Package | Build | Complexity | Key Fixes | File |
|---------|-------|------------|-----------|------|
| zlib-ng | cmake | medium | _XOPEN_SOURCE=600, cmake cross-compile | [zlib-ng.yaml](packages/zlib-ng.yaml) |
| bzip2 | make | low | Remove test from all target, bash brace expansion | [bzip2.yaml](packages/bzip2.yaml) |
| popt | autoconf | medium | vasprintf (IRIX vsnprintf bug), stpcpy conflict, libtool | [popt.yaml](packages/popt.yaml) |
| openssl | custom | high | Uses ./Configure not ./configure, no-asm, multiarch headers | [openssl.yaml](packages/openssl.yaml) |
| libxml2 | autoconf | medium | Libtool fixes, disable python/lzma | [libxml2.yaml](packages/libxml2.yaml) |
| curl | autoconf | high | Out-of-tree build, LD export, __mips64 fix, complex env setup | [curl.yaml](packages/curl.yaml) |
| xz | autoconf | high | Libtool fixes, doc generation disable | [xz.yaml](packages/xz.yaml) |
| lua | make | medium | Multiarch headers (luaconf-mips64.h) | [lua.yaml](packages/lua.yaml) |
| file | autoconf | medium | posix_spawn_file_actions, link mogrix-compat | [file.yaml](packages/file.yaml) |
| sqlite | autoconf | medium | Disable math funcs, LLONG_MAX, _ABI_SOURCE | [sqlite.yaml](packages/sqlite.yaml) |
| rpm | cmake | high | vsnprintf fix, spawn.h, TLS removal, cmake cross-compile | [rpm.yaml](packages/rpm.yaml) |
| libsolv | cmake | high | funopen (not fopencookie), --whole-archive, cmake flags | [libsolv.yaml](packages/libsolv.yaml) |
| tdnf | cmake | high | Hardcode mips arch, disable plugins, **patch /etc→/usr/sgug/etc paths** | [tdnf.yaml](packages/tdnf.yaml) |

---

## By Build System

### autoconf (./configure)

Common patterns: `export_vars.LD`, libtool sed fixes, `ac_cv_overrides`

| Package | Complexity | Key Fixes |
|---------|------------|-----------|
| popt | medium | vasprintf, stpcpy conflict |
| libxml2 | medium | libtool, disable python |
| curl | high | out-of-tree, LD export |
| xz | high | libtool, doc disable |
| file | medium | posix_spawn_file_actions |
| sqlite | medium | math disable, LLONG_MAX |
| gnutls | medium | disable hardware accel |
| nettle | low | basic cross-compile |
| libgpg-error | low | lock-obj header |
| libgcrypt | low | disable asm |
| libassuan | low | basic compat |
| libksba | low | basic cross-compile |
| gpgme | medium | Qt bindings disable |
| gnupg2 | medium | many feature disables |
| p11-kit | medium | trust module disable |
| libtasn1 | low | basic compat |
| expat | low | basic cross-compile |
| libarchive | medium | feature disables |
| elfutils | medium | disable debuginfod |
| freetype | low | basic cross-compile |
| harfbuzz | low | feature disables |
| pixman | low | disable SIMD |
| cairo | medium | feature disables |
| pango | low | basic cross-compile |
| glib2 | medium | iconv, feature disables |
| git | medium | many feature disables |

### cmake

Common patterns: Full toolchain vars, `-DCMAKE_SYSTEM_NAME=IRIX`, staging paths

| Package | Complexity | Key Fixes |
|---------|------------|-----------|
| zlib-ng | medium | _XOPEN_SOURCE, static/shared |
| rpm | high | vsnprintf, spawn, TLS |
| libsolv | high | funopen, --whole-archive |
| tdnf | high | mips hardcode, plugin disable |

### make (Plain Makefile)

Common patterns: `export_vars.CC/AR/RANLIB`, prep_commands for Makefile edits

| Package | Complexity | Key Fixes |
|---------|------------|-----------|
| bzip2 | low | test target remove |
| lua | medium | multiarch headers |

### custom

| Package | Build Tool | Complexity | Key Fixes |
|---------|------------|------------|-----------|
| openssl | ./Configure | high | no-asm, custom targets |

---

## By Compat Functions Needed

### String Functions
| Function | Packages |
|----------|----------|
| strdup | popt, file, xz, libsolv, tdnf, git, many others |
| strndup | popt, file, libsolv, tdnf |
| strcasestr | libsolv |
| strsep | tdnf |

### I/O Functions
| Function | Packages |
|----------|----------|
| getline | libsolv, tdnf, file, gnutls, p11-kit |
| asprintf/vasprintf | popt, tdnf, file, gnutls, p11-kit, gpgme |
| funopen | libsolv (BSD cookie I/O, replaces fopencookie) |
| fopencookie | BROKEN on IRIX - use funopen instead |

### POSIX Functions
| Function | Packages |
|----------|----------|
| openat family | rpm, libsolv (17 functions in openat-compat.c) |
| posix_spawn | rpm, file |
| getopt_long | file, tdnf |
| timegm | libsolv |
| mkdtemp | libsolv |
| qsort_r | libsolv, tdnf |

---

## Common Patterns

### Libtool Shared Library Fix
Many autoconf packages need libtool fixes after %configure. Use the standard script:
```yaml
spec_replacements:
  - pattern: "%configure"
    replacement: |
      %configure
      $MOGRIX_ROOT/tools/fix-libtool-irix.sh libtool || exit 1
```
This uses `safepatch` internally and **fails loudly** if patterns don't match.
See: [methods/text-replacement.md](methods/text-replacement.md), popt.yaml, libxml2.yaml, xz.yaml

### GNU ld for Shared Libraries
IRIX requires GNU ld (not LLD) for shared libraries:
```yaml
export_vars:
  LD: "/opt/cross/bin/mips-sgi-irix6.5-ld.bfd"
```
See: popt.yaml, curl.yaml, most autoconf packages

### CMake Cross-Compilation
```yaml
spec_replacements:
  - pattern: "%cmake"
    replacement: |
      cmake -B _build -S . \
        -DCMAKE_SYSTEM_NAME=IRIX \
        -DCMAKE_C_COMPILER=%{__cc} \
        ...
```
See: rpm.yaml, libsolv.yaml, tdnf.yaml

### Bash Brace Expansion Fix
IRIX /bin/sh doesn't support `${var}{a,b}`:
```yaml
spec_replacements:
  - pattern: "mkdir -p $RPM_BUILD_ROOT{%{_bindir},%{_libdir}}"
    replacement: "mkdir -p $RPM_BUILD_ROOT%{_bindir} $RPM_BUILD_ROOT%{_libdir}"
```
See: bzip2.yaml, many others

---

## Files Reference

| File | Purpose |
|------|---------|
| methods/*.md | Process documentation (text replacement, etc.) |
| generic.yaml | Universal rules applied to ALL packages |
| packages/*.yaml | Per-package rules (64 files) |
| ../compat/catalog.yaml | Compat function registry |
| ../tools/safepatch | Perl tool for reliable text replacement |
| ../tools/fix-libtool-irix.sh | Standard libtool fix script |
| ../HANDOFF.md | Current issues and session state |

---

## When Adding New Packages

1. **Check similar packages** - Find one with same build system
2. **Start minimal** - Only add rules that fix actual build failures
3. **Test incrementally** - `mogrix convert` → `mogrix build --cross` → `mogrix stage`
4. **Document key fixes** - Add comments explaining why each fix is needed
5. **Update this index** - Add entry to appropriate section
