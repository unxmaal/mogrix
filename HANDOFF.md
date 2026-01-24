# Mogrix Cross-Compilation Handoff

**Last Updated**: 2026-01-24
**Status**: rpm 4.19 build complete; ready for libsolv and tdnf

## Current Session: rpm Build Complete

Successfully built rpm 4.19.1.1 through the mogrix workflow with extensive spec_replacements.

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
| libsolv | PENDING | Next - needs fopencookie |
| tdnf | PENDING | Blocked by libsolv |

### rpm 4.19.1.1 Build Complete

**Built RPMs (MIPS N32):**
- rpm-4.19.1.1-1.x86_64.rpm (4.1MB) - Main package with IRIX binaries
- rpm-build-4.19.1.1-1.x86_64.rpm (177KB) - rpmbuild, rpmspec, rpmlua
- rpm-devel-4.19.1.1-1.x86_64.rpm (1.3MB) - Development headers
- rpm-sign-4.19.1.1-1.x86_64.rpm (57KB) - Package signing
- rpm-libs-4.19.1.1-1.x86_64.rpm, rpm-build-libs, rpm-sign-libs - Libraries
- rpm-apidocs-4.19.1.1-1.noarch.rpm (4.0MB) - API documentation

**Verified:**
```
ELF 32-bit MSB executable, MIPS, N32 MIPS-III version 1 (SYSV),
dynamically linked, interpreter /lib32/rld, with debug_info, not stripped
```

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
| `compat/dicl/openat-compat.c` | Added futimens implementation |
| `compat/catalog.yaml` | Added futimens to openat provides |
| `/opt/sgug-staging/usr/sgug/include/dicl-clang-compat/sys/stat.h` | futimens declaration, UTIME_* defines |
| `rules/packages/rpm.yaml` | Complete rewrite with all spec_replacements |
| `rpm-4.19.1.1-1.fc40.src-converted/rpm.spec` | Fixed spec file |

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

1. **Build libsolv** - Needs fopencookie compat
2. **Build tdnf** - Final goal
3. **Test RPMs on IRIX**

## Known Issues

### Compat Functions Required

| Function | Package | Status |
|----------|---------|--------|
| POSIX.1-2008 "at" funcs | rpm | DONE - openat-compat.c |
| getprogname/setprogname | rpm | DONE - getprogname.c |
| fopencookie | libsolv | READY - compat/stdio/fopencookie.c |

### rpmbuild Arch Warnings

When cross-compiling, rpmbuild shows "Binaries arch (1) not matching the package arch (2)".
This is expected - the host is x86_64 but binaries are MIPS.

### Missing build-id Warnings

Cross-compiled IRIX binaries don't have Linux build-ids. These warnings are expected.
