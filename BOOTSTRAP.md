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
- [x] RPM architecture naming fixed (--target mips-sgi-irix)
- [x] Auto-devel package staging
- [x] Multiarch header handling
- [x] **TARGET PACKAGE (tdnf) BUILT SUCCESSFULLY!**

#### Full Validated Build Chain (2026-01-24)

| # | Package | Version | Status | Notes |
|---|---------|---------|--------|-------|
| 1 | zlib | 1.2.13 | ✓ Built | Compression |
| 2 | bzip2 | 1.0.8 | ✓ Built | Compression |
| 3 | popt | 1.19 | ✓ Built | Option parsing |
| 4 | openssl | 3.1.1 | ✓ Built | TLS/SSL |
| 5 | libxml2 | 2.10.4 | ✓ Built | XML parsing |
| 6 | curl | 8.2.1 | ✓ Built | HTTP/HTTPS |
| 7 | xz | 5.4.6 | ✓ Built | Compression |
| 8 | lua | 5.4.6 | ✓ Built | Scripting (rpm) |
| 9 | file | 5.45 | ✓ Built | libmagic |
| 10 | rpm | 4.19.1.1 | ✓ Built | Package manager core |
| 11 | libsolv | 0.7.28 | ✓ Built | Dependency solver |
| 12 | **tdnf** | **3.5.14** | **✓ Built** | **TARGET ACHIEVED** |

All packages produce valid **ELF 32-bit MSB, MIPS N32** binaries for IRIX 6.5.

#### tdnf Package Contents
- `/usr/sgug/bin/tdnf` - Main binary
- `/usr/sgug/bin/yum`, `/usr/sgug/bin/tyum` - Compatibility symlinks
- `/usr/sgug/lib32/libtdnf.so` - Library
- Configuration and completions

## Resolved Issues

### Dynamic Linking (SOLVED)
- **Shared libraries**: Use GNU ld (`mips-sgi-irix6.5-ld.bfd`) for correct 2-LOAD segment layout
- **Executables**: Use LLD with `--dynamic-linker=/lib32/rld`
- The `irix-ld` wrapper handles linker selection automatically

### Staging Automation (SOLVED)
- **-devel packages**: Auto-included by `mogrix stage`
- **Multiarch headers**: Auto-created (luaconf-mips64.h, configuration-mips64.h)
- **RPM arch naming**: `--target mips-sgi-irix` produces .mips.rpm files

### Common Compat Functions (Implemented)
All stored in `compat/` and `compat/catalog.yaml`:
- POSIX.1-2008 "at" functions (openat, fstatat, etc.)
- getprogname/setprogname, getline, strdup/strndup
- fopencookie, timegm, mkdtemp, qsort_r
- asprintf/vasprintf, getopt_long, strcasestr, strsep

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
