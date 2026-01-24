# Mogrix Cross-Compilation Handoff

**Last Updated**: 2026-01-24
**Status**: tdnf 3.5.14 BUILD COMPLETE - TARGET ACHIEVED

---

## CRITICAL: Knowledge Must Be Stored in Mogrix

**READ THIS FIRST.** The mission of mogrix is storing knowledge. If you make a fix and don't write it into mogrix rules, that knowledge is LOST.

### Fixes Made Outside Mogrix (MUST BE FIXED)

| Fix | Location | Status |
|-----|----------|--------|
| `sys/stat.h` wrapper | `/opt/sgug-staging/.../dicl-clang-compat/sys/stat.h` | **NOT IN MOGRIX** |

**This WILL break a clean build.** The file must be copied to `mogrix/cross/include/dicl-clang-compat/sys/stat.h`.

### The Rule

Before ending ANY session:
1. List all fixes made outside mogrix source
2. Add each fix to appropriate mogrix location
3. Verify clean build would work

If you can't rebuild from a fresh clone of mogrix + fresh staging area, **you have not finished**.

---

## Current Session: tdnf Build Complete

Successfully built tdnf 3.5.14 (the TARGET PACKAGE) for IRIX 6.5 MIPS N32.

### Package Validation Status

| Package | Mogrix Workflow | Notes |
|---------|-----------------|-------|
| zlib | DONE | No issues |
| bzip2 | DONE | No issues |
| openssl | DONE | 3.2.1 - RPMs built |
| curl | DONE | 8.6.0 - RPMs built |
| lua | DONE | 5.4.6 - Required by rpm |
| file | DONE | 5.45 - libmagic for rpm |
| rpm | DONE | 4.19.1.1 - RPMs built (MIPS N32) |
| libxml2 | DONE | 2.12.5 - RPMs built |
| libsolv | DONE | 0.7.29 - RPMs built |
| **tdnf** | **DONE** | **3.5.14 - TARGET ACHIEVED** |

### tdnf 3.5.14 Build Complete (TARGET ACHIEVED)

**Built RPMs (MIPS N32):**
- tdnf-3.5.14-1.mips.rpm - Main package manager
- tdnf-cli-libs-3.5.14-1.mips.rpm - CLI library
- tdnf-devel-3.5.14-1.mips.rpm - Development headers

**Package contents:**
- `/usr/sgug/bin/tdnf` - Main binary
- `/usr/sgug/bin/yum`, `/usr/sgug/bin/tyum` - Compatibility symlinks
- `/usr/sgug/lib32/libtdnf.so` - Library
- Configuration files and bash completion

**Verified:**
```
ELF 32-bit MSB executable, MIPS, N32 MIPS-III version 1 (SYSV),
dynamically linked, interpreter /lib32/rld, with debug_info, not stripped
```

### Key Fixes for tdnf (stored in rules/packages/tdnf.yaml)

| Fix | Description |
|-----|-------------|
| sqlite3_stub | Stub implementation (no sqlite3 on IRIX) |
| qsort_r | GNU extension compat |
| strsep | BSD extension compat |
| strings.h | For strcasecmp/strncasecmp |
| sys/ttold.h | For struct winsize |
| sys/statfs.h | IRIX statfs differences |
| GPG stubs | rpm API incompatible |
| cmake cross-compile | Full toolchain setup |
| Disabled: Python, metalink, repogpgcheck, systemd | N/A on IRIX |

### Key Fixes for rpm 4.19

**POSIX.1-2008 "at" functions (compat/dicl/openat-compat.c):**
- openat, fstatat, faccessat, mkdirat, unlinkat, renameat
- readlinkat, symlinkat, linkat, fchmodat, fchownat
- mkfifoat, mknodat, utimensat, futimens
- stpcpy, stpncpy

**getprogname/setprogname (compat/unistd/getprogname.c):**
- BSD extension for program name access

**cmake cross-compilation settings:**
- CMAKE_SYSTEM_NAME=IRIX
- CMAKE_EXE_LINKER_FLAGS and CMAKE_SHARED_LINKER_FLAGS with compat library
- All HAVE_* cache variables for compat functions
- Disabled: NLS, Python, plugins, SELinux, audit, dbus, systemd

**Extensive spec_replacements in rpm.yaml:**
- bcond changes to disable optional features
- Skip systemd unit file installation
- Comment out all plugin %files entries
- Fix library versioning for IRIX
- Post-install merge of lib/ to lib32/

## Files Modified This Session

| File | Change |
|------|--------|
| `rules/packages/tdnf.yaml` | Extensive cross-compilation rules (233 lines) |
| `compat/stdlib/qsort_r.c` | qsort_r GNU extension compat |
| `compat/string/strsep.c` | strsep BSD extension compat |
| `compat/sqlite/sqlite3_stub.c` | Stub sqlite3 for disabled history |
| `compat/include/mogrix-compat/generic/sqlite3.h` | sqlite3 stub header |
| `compat/include/mogrix-compat/generic/string.h` | Added strsep declaration |
| `compat/include/mogrix-compat/generic/stdlib.h` | Added qsort_r declaration |
| `compat/catalog.yaml` | Added sqlite3_stub, qsort_r, strsep |
| `/opt/sgug-staging/.../dicl-clang-compat/sys/stat.h` | **NOT IN MOGRIX - MUST FIX** |
| `BOOTSTRAP.md` | Updated with tdnf success |
| `plan.md` | Added knowledge gap phase |
| `Claude.md` | Added knowledge storage emphasis |

## Rpmbuild Command

```bash
# For packages needing --nodeps (cross-compilation skips BuildRequires check)
rpmbuild -ba --nodeps /path/to/spec.spec
```

## Solution Summary

Cross-compiled dynamically-linked executables and shared libraries work on IRIX:

1. **Shared libraries**: Use GNU ld (`mips-sgi-irix6.5-ld.bfd`) for correct 2-LOAD segment layout
2. **Executables**: Use LLD with `--dynamic-linker=/lib32/rld` interpreter

### Key Technical Details

| Tool | Use Case | Notes |
|------|----------|-------|
| clang | Compilation | Via irix-cc wrapper |
| ld.lld-irix | Executables | LLD with `--dynamic-linker=/lib32/rld` |
| mips-sgi-irix6.5-ld.bfd | Shared libraries | GNU ld for correct segment layout |
| llvm-ar | Static libraries | Standard archiver |

## Environment

- **IRIX Host**: `ssh edodd@192.168.0.81`
- **SGUG shell**: `/usr/sgug/bin/sgugshell`
- **Cross toolchain**: `/opt/cross/bin/`
- **Staging**: `/opt/sgug-staging/usr/sgug/`

## Next Steps

### IMMEDIATE: Store Missing Knowledge

1. **Copy sys/stat.h to mogrix source:**
   ```bash
   cp /opt/sgug-staging/usr/sgug/include/dicl-clang-compat/sys/stat.h \
      mogrix/cross/include/dicl-clang-compat/sys/stat.h
   ```

2. **Move generic patterns to generic.yaml:**
   - `%systemd_post`, `%systemd_preun`, etc.
   - Other systemd scriptlet macros

3. **Create cmake class** for cross-compilation boilerplate

4. **Verify clean build** - Can we rebuild from fresh staging?

### After Knowledge Gaps Fixed

1. **Test tdnf on IRIX**
   - Copy RPMs to IRIX
   - Create tdnf.conf configuration
   - Test `tdnf repolist`, `tdnf search`, etc.

2. **Create bootstrap repository**
   - Package all dependencies
   - Host repository for IRIX machines

## Known Issues

### Knowledge Gaps (FIXED THIS SESSION)

| Fix | Status |
|-----|--------|
| sys/stat.h wrapper | FIXED - copied to `cross/include/dicl-clang-compat/sys/stat.h` |
| `%systemd_*` scriptlets | FIXED - added to `generic.yaml` remove_lines |

### Remaining Improvements (Lower Priority)

| Pattern | Currently In | Should Be In |
|---------|--------------|--------------|
| cmake cross-compile settings | tdnf.yaml | cmake class (future) |
| Python bindings disable | tdnf.yaml | python class (future) |

### Compat Functions Implemented

| Function | Package | Status |
|----------|---------|--------|
| POSIX.1-2008 "at" funcs | rpm | DONE - openat-compat.c |
| getprogname/setprogname | rpm | DONE - getprogname.c |
| fopencookie | libsolv | DONE - fopencookie.c |
| qsort_r | tdnf | DONE - qsort_r.c |
| strsep | tdnf | DONE - strsep.c |
| sqlite3_stub | tdnf | DONE - sqlite3_stub.c |

### rpmbuild Warnings (Expected)

- "Binaries arch (1) not matching the package arch (2)" - Expected for cross-compilation
- "Missing build-id" - IRIX binaries don't have Linux build-ids
