# Mogrix Build Order for tdnf

This document defines the complete build order for cross-compiling tdnf and all dependencies for IRIX 6.5.

**STATUS: VALIDATED** - All packages successfully built 2026-01-24. See HANDOFF.md for details.

## Prerequisites

Before starting, ensure:

1. **IRIX sysroot** is in place: `/opt/irix-sysroot/`
2. **Cross toolchain** is installed: `/opt/cross/bin/`
3. **Staging directory** exists: `/opt/sgug-staging/usr/sgug/`
4. **Fixed CRT files** exist: `/opt/irix-sysroot/usr/lib32/mips3/fixed/crt1.o` and `crtn.o`
5. **Toolchain wrappers** are deployed to staging:
   - `irix-cc` -> `/opt/sgug-staging/usr/sgug/bin/irix-cc`
   - `irix-ld` -> `/opt/sgug-staging/usr/sgug/bin/irix-ld`
6. **irix-compat.h** is in place: `/opt/sgug-staging/usr/sgug/include/irix-compat.h`

## Build Phases

### Phase 1: Foundation Libraries (No Dependencies)

| Package | Type | Notes |
|---------|------|-------|
| zlib | shared lib | Compression - used by almost everything |
| bzip2 | shared lib | Compression |
| xz (liblzma) | shared lib | Compression |
| expat | shared lib | XML parsing |
| popt | shared lib | Option parsing - needed by rpm |
| ncurses | static lib | Terminal handling |
| readline | static lib | Line editing (depends on ncurses) |

### Phase 2: Security/Crypto Libraries

| Package | Type | Dependencies | Notes |
|---------|------|--------------|-------|
| openssl | shared lib | zlib | TLS/SSL - use ./Configure irix-mips3-gcc |
| libgpg-error | shared lib | none | GnuPG base - needs lock-obj-pub.irix6.5.h |
| libgcrypt | shared lib | libgpg-error | Crypto primitives |
| libassuan | shared lib | libgpg-error | IPC library |
| nettle | shared lib | none | Low-level crypto |
| libtasn1 | shared lib | none | ASN.1 parsing |
| p11-kit | shared lib | libtasn1 | PKCS#11 |
| gnutls | shared lib | nettle, libtasn1, p11-kit | TLS (optional for tdnf) |
| gpgme | shared lib | libgpg-error, libassuan | GnuPG Made Easy |

### Phase 3: XML and Network

| Package | Type | Dependencies | Notes |
|---------|------|--------------|-------|
| libxml2 | shared lib | none (--without-zlib) | XML parsing - use --without-threads |
| curl | shared lib | openssl, zlib | HTTP/HTTPS - use --disable-threaded-resolver |
| sqlite | shared lib | none | Database (optional for rpm) |

### Phase 4: Archive and ELF Handling

| Package | Type | Dependencies | Notes |
|---------|------|--------------|-------|
| file (libmagic) | shared lib | zlib | File type detection |
| libarchive | shared lib | zlib, bzip2, xz, expat, openssl | Archive extraction |
| lua | static lib | readline | Scripting for rpm |

### Phase 5: RPM

| Package | Type | Dependencies | Notes |
|---------|------|--------------|-------|
| rpm | shared lib | popt, lua, libarchive, openssl, zlib, bzip2, xz, libmagic | Package manager core |

### Phase 6: Package Management (GOAL)

| Package | Type | Dependencies | Notes |
|---------|------|--------------|-------|
| libsolv | shared lib | zlib, bzip2, xz, expat, libxml2, rpm | Dependency solver - needs fopencookie |
| tdnf | executable | libsolv, rpm, curl, gpgme | Package manager - GOAL |

## Critical Build Notes

### Linker Selection
- **Shared libraries**: GNU ld (`mips-sgi-irix6.5-ld.bfd`) - produces correct 2-LOAD segments
- **Executables**: LLD with `--dynamic-linker=/lib32/rld` - critical for working executables

### Package-Specific Issues

| Package | Issue | Solution |
|---------|-------|----------|
| openssl | Non-autotools | Use `./Configure irix-mips3-gcc no-asm no-async no-sctp shared` |
| libgpg-error | Needs lock object | Generate `lock-obj-pub.irix6.5.h` on IRIX first |
| libxml2 | pthread crashes rld | Use `--without-threads --without-tls` |
| curl | pthread dlopen issue | Use `--disable-threaded-resolver` |
| sqlite | 128-bit float symbols | Use `-DLONGDOUBLE_TYPE=double` |
| libsolv | Needs fopencookie | Inject fopencookie compat source |

### Header Issues
- `scandir`/`alphasort` hidden behind `_SGIAPI` - declarations in irix-compat.h
- `isinf()` not available without `__c99` - macro in irix-compat.h
- `struct winsize` hidden by `_XOPEN_SOURCE` - definition in irix-compat.h

## Validation Commands

After each package, test on IRIX via MCP tools:

```bash
# For shared libraries (via MCP)
irix_copy_to /path/to/libfoo.so /tmp/libfoo.so
irix_exec "LD_LIBRARYN32_PATH=/tmp /tmp/test-dlopen /tmp/libfoo.so"

# For executables (via MCP)
irix_copy_to /path/to/executable /tmp/executable
irix_exec "LD_LIBRARYN32_PATH=/tmp /tmp/executable --version"
```

## Final Validation

When tdnf is built:

```bash
# On IRIX
LD_LIBRARYN32_PATH=/usr/sgug/lib32 /usr/sgug/bin/tdnf --version
# Should output: tdnf: 3.5.x

LD_LIBRARYN32_PATH=/usr/sgug/lib32 /usr/sgug/bin/tdnf repolist
# Should output: Error(1602) - expected, no config file
```

## Estimated Package Count

- Phase 1: 7 packages
- Phase 2: 9 packages
- Phase 3: 3 packages
- Phase 4: 3 packages
- Phase 5: 1 package
- Phase 6: 2 packages
- **Total: ~25 packages**
