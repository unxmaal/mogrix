# SGUG-RSE Cross-Compilation Bootstrap

This document tracks the effort to bootstrap SGUG-RSE packages via cross-compilation from Linux.

**Golden Rule: NEVER build on IRIX.** The target hardware is 30+ years old and too slow. All compilation happens on Linux via cross-compilation.

## Workflow

```
┌─────────────────────────────────────────────────────────────────────┐
│                         LINUX HOST                                   │
│                                                                      │
│  FC40 SRPM ──► mogrix convert ──► IRIX SRPM ──► rpmbuild ──► RPM    │
│                                                    (cross)           │
└─────────────────────────────────────────────────────────────────────┘
                                                         │
                                                         ▼ scp/rsync
┌─────────────────────────────────────────────────────────────────────┐
│                         IRIX TARGET                                  │
│                                                                      │
│  rpm -ivh package.rpm  ──►  /usr/sgug/...  ──►  TEST ONLY           │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

## Current Status

### Phase A: Cross-Compilation Foundation

#### Toolchain (COMPLETE)
- [x] Clang 16 with `--target=mips-sgi-irix6.5`
- [x] vvuk's LLD with IRIX support at `/opt/cross/bin/ld.lld-irix`
- [x] Compiler wrapper `/opt/cross/bin/irix-cc-bootstrap`
- [x] Linker wrapper `/opt/sgug-staging/usr/sgug/bin/irix-ld`
- [x] Fixed CRT files at `/opt/irix-sysroot/usr/lib32/mips3/fixed/`
- [x] Pristine IRIX sysroot at `/opt/irix-sysroot/`
- [x] Staging area at `/opt/sgug-staging/usr/sgug/`

#### Foundation Libraries - Static (COMPLETE)
Built from upstream tarballs, validated on IRIX:

| Package | Version | Library | Size | Status |
|---------|---------|---------|------|--------|
| zlib | 1.3.1 | libz.a | 205KB | ✓ Working |
| bzip2 | 1.0.8 | libbz2.a | 206KB | ✓ Working |
| xz | 5.4.5 | liblzma.a | 1.2MB | ✓ Working |
| ncurses | 6.4 | libncursesw.a | 455KB | ✓ Working |
| readline | 8.2 | libreadline.a | 1.2MB | ✓ Working |

#### Core Utilities - Static (COMPLETE)
Built from upstream tarballs, validated on IRIX:

| Package | Version | Status |
|---------|---------|--------|
| bash | 5.2 | ✓ Working |
| coreutils | 8.30 | ✓ Working |
| tar | 1.30 | ✓ Working |
| gzip | 1.9 | ✓ Working |
| sed | 4.5 | ✓ Working |
| grep | 3.3 | ✓ Working |
| make | 4.3 | ✓ Working |
| patch | 2.7.6 | ✓ Working |

### Phase B: Mogrix SRPM Conversion

#### Mogrix Status
- [x] Spec file parsing
- [x] YAML rule loading
- [x] Rule application engine
- [x] Header overlay system (16 generic headers)
- [x] Compat source injection (strdup, getline, asprintf, etc.)
- [x] Spec file rewriting
- [x] SRPM extraction and repackaging
- [x] 32 package rules created

#### Package Rules Ready
See `mogrix/rules/packages/` for:
- popt, expat, libxml2
- libgpg-error, libgcrypt, libassuan, libksba, nettle
- libtasn1, p11-kit, gnutls, gpgme, gnupg2, openssl
- python3, glib2, gobject-introspection, pcre, pcre2
- elfutils, file, libarchive, rpm, libsolv, zchunk
- curl, libmetalink, ca-certificates
- tdnf

### Phase C: Cross-Build via rpmbuild (COMPLETE!)

#### rpmbuild Cross-Compilation
- [x] Cross-compilation macros at `/opt/sgug-staging/rpmmacros.irix`
- [x] Test package (test-hello) built and validated
- [x] FC40 zlib SRPM built and validated
- [x] Integration with mogrix-converted SRPMs
- [x] **TARGET PACKAGE (tdnf) BUILT SUCCESSFULLY!**

#### Successfully Built Package Chain

| Package | Version | Status | Notes |
|---------|---------|--------|-------|
| libxml2 | 2.12.5 | ✓ Built | Base XML library |
| libsolv | 0.7.29 | ✓ Built | Package dependency solver |
| curl | 8.6.0 | ✓ Built | URL transfer library |
| rpm | 4.19.1.1 | ✓ Built | Package manager base |
| **tdnf** | **3.5.14** | **✓ Built** | **Target: Tiny DNF package manager** |

All packages produce MIPS N32 ELF binaries compatible with IRIX 6.5.

#### tdnf Package Contents
- `/usr/sgug/bin/tdnf` - Main binary
- `/usr/sgug/bin/yum`, `/usr/sgug/bin/tyum` - Compatibility symlinks
- `/usr/sgug/lib32/libtdnf.so` - Library
- Configuration and completions

## Known Issues

### Static vs Shared Libraries
- **Static linking works** - All Phase A packages use static libraries
- **Shared library linking has issues** - Base address conflicts, needs more investigation
- **Current approach**: Static linking for bootstrap, revisit shared libs later

### Linker Settings (irix-ld wrapper)
Current `/opt/sgug-staging/usr/sgug/bin/irix-ld`:
- Executables: `--image-base=0x10000000`, interpreter `/usr/lib32/libc.so.1`
- Shared libs: `--image-base=0x5ffe0000` (untested)

### Common Source Fixes Required
These patterns appear across many packages:
- `getprogname.c` - IRIX procfs simplified (returns "?")
- `HAVE_EACCESS` - Disabled (IRIX lacks eaccess)
- `vfork/fork` - Macro conflicts resolved
- GNU glob extensions - Guarded with `#ifndef __sgi`
- `vasnprintf` - `USE_SNPRINTF` disabled (NULL buffer crash)
- `PATH_MAX` - Defined using IRIX `MAXPATHLEN`
- `libsoft_float_stubs.a` - For compiler-rt long double functions

## Next Steps

1. **Test tdnf on IRIX**
   - Copy tdnf and dependencies to IRIX
   - Install and validate functionality
   - Create/configure repositories

2. **Build additional packages**
   - python3 (required for many tools)
   - Additional CLI utilities

3. **Documentation improvements**
   - Complete build workflow guide
   - Troubleshooting common issues

## Key Paths

| Purpose | Path |
|---------|------|
| IRIX sysroot (pristine) | `/opt/irix-sysroot/` |
| Staging area | `/opt/sgug-staging/usr/sgug/` |
| Cross toolchain | `/opt/cross/bin/` |
| Mogrix project | `/home/edodd/projects/github/unxmaal/mogrix/` |
| rpmbuild macros | `/opt/sgug-staging/rpmmacros.irix` |
| Linker wrapper | `/opt/sgug-staging/usr/sgug/bin/irix-ld` |

## IRIX Test Host

- **SSH**: `ssh edodd@192.168.0.81`
- **Shell**: Run `/usr/sgug/bin/sgugshell` after connecting (or use csh setenv syntax)
- **System**: IRIX 6.5.30 Octane with full SGUG-RSE

## References

- `/src/plan.md` - Original SGUG-RSE revival plan
- `/src/overview.md` - Cross-compilation technical overview
- `mogrix/plan.md` - Mogrix design and package rules
- worldrebuilder.sh - Full 667-package build order (reference only)
