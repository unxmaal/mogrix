# Mogrix Cross-Compilation Handoff

**Last Updated**: 2026-01-27
**Status**: LLD 18 built and working - rpm 4.19.1.1 successfully cross-compiled with correct relocations

---

## Goal

Get **rpm** and **tdnf** working on IRIX so packages can be installed via the package manager.

---

## Current Progress

### What's Done
- All 12 packages cross-compile successfully (zlib-ng, bzip2, popt, openssl, libxml2, curl, xz, lua, file, rpm, libsolv, tdnf)
- LLD 18.1.3 built from source with IRIX patches - fixes MIPS relocation bug
- rpm 4.19.1.1 binary now has correct relocations (symbols reference actual names, not `*ABS*`)
- Bootstrap tarball infrastructure ready

### What's Next
1. Create new bootstrap tarball with LLD 18-built binaries
2. Test on IRIX to verify rpm works (`/opt/chroot/usr/sgug/bin/rpm --version`)
3. Build tdnf and test package installation

---

## What Worked

### LLD 18 from Source
Built LLD 18.1.3 from LLVM source with three essential patches. This fixed the MIPS relocation bug where external symbols in static data were incorrectly referenced as `*ABS*`.

### Patches Applied to LLVM 18.1.3

**1. IRIX ELF compatibility (Writer.cpp)**
Skip features that crash IRIX's ELF parser:
- PT_MIPS_ABIFLAGS program header
- PT_GNU_STACK program header
- PT_PHDR in shared libraries

**2. MIPS local symbols in DSO (InputFiles.cpp)**
IRIX shared libraries have section symbols (.text, .data, etc.) in the global part of the symbol table. LLD 18 rejects this by default.
```cpp
// Skip error for section symbols (starting with .) on MIPS
if (emachine != EM_MIPS || !name.starts_with(".")) {
  errorOrWarn(toString(this) + ": invalid local symbol...");
}
```

**3. sh_entsize=0 for .dynamic section (ELF.h)**
GNU ld (MIPS) produces .dynamic sections with sh_entsize=0. LLD 18 expects it to be 8.
```cpp
// Allow sh_entsize=0 - some linkers produce .dynamic with entsize=0
if (Sec.sh_entsize != sizeof(T) && sizeof(T) != 1 && Sec.sh_entsize != 0)
  return createError(...);
```

### RPM Fixes
- `-DRPM_CONFIGDIR=/usr/sgug/lib32/rpm` - Fix config directory path
- `sed -i 's/static __thread/static/'` - Remove TLS (IRIX rld doesn't support `__tls_get_addr`)

### libsolv Fix
- `xmlErrorPtr` → `const xmlError *` for newer libxml2 compatibility

---

## What Failed

### Using vvuk's LLD 14 fork
The existing LLD 14 (vvuk's fork at `/opt/cross/bin/ld.lld-irix`) generated incorrect relocations:
- External symbols in static data referenced `*ABS*` instead of actual symbol names
- IRIX rld couldn't resolve these, leaving pointers NULL
- Result: rpm failed with "unknown option" for ALL options

### Using GNU ld for executables
Tried modifying `irix-ld` to use GNU ld (`mips-sgi-irix6.5-ld.bfd`) for executables:
- **Failed**: cmake's linker flags don't include `-lcompat`
- The compat library provides `getprogname`, `setprogname`, etc.
- Would require modifying cmake toolchain files

### Setting RPM_CONFIGDIR environment variable
- Doesn't help because the bug was in binary's data section relocations, not config file loading

### Checking input file osabi for IRIX
First attempt to fix LLD 18 local symbol errors checked `this->osabi`:
- **Failed**: IRIX system libraries have osabi=0 (System V), not ELFOSABI_IRIX (8)
- Fixed by checking for MIPS + section symbol names starting with `.`

---

## Relocation Fix - VERIFIED

The original problem was that LLD 14 generated incorrect relocations for external symbols in static data:

**Before (LLD 14 - broken)**:
```
R_MIPS_REL32      00000000   *ABS*
```

**After (LLD 18 - fixed)**:
```
R_MIPS_REL32      00000000   rpmcliAllPoptTable
```

The optionsTable[] array now has proper symbol references that IRIX's rld can resolve at runtime.

---

## Key Files

| File | Purpose |
|------|---------|
| `tools/bin/ld.lld-irix-18` | New LLD 18 binary with IRIX patches |
| `scripts/build-lld-irix.sh` | Script to rebuild LLD from source |
| `tmp/lld-irix-build/` | LLD build directory with patched source |
| `rules/packages/rpm.yaml` | RPM build rules with TLS fix, config path fix |
| `rules/packages/libsolv.yaml` | libsolv build rules with xmlErrorPtr fix |
| `/opt/sgug-staging/usr/sgug/bin/irix-ld` | Linker wrapper (updated to use new LLD) |

---

## irix-ld Configuration

The linker wrapper is configured to use the new LLD 18:
```bash
LLD="${IRIX_LLD:-/home/edodd/projects/github/unxmaal/mogrix/tools/bin/ld.lld-irix-18}"
```

---

## Verification Commands

### Check relocations are correct
```bash
# Extract rpm binary from package
cd /tmp && rm -rf rpm-check && mkdir rpm-check && cd rpm-check
rpm2cpio ~/rpmbuild/RPMS/mips/rpm-4.19.1.1-1.mips.rpm | cpio -idmv

# Should show symbol names, not *ABS*
/opt/cross/bin/mips-sgi-irix6.5-readelf -r usr/sgug/bin/rpm

# Should show non-zero values in optionsTable
/opt/cross/bin/mips-sgi-irix6.5-objdump -s -j .data usr/sgug/bin/rpm | grep -A 5 34d5
```

### Test on IRIX
```bash
# Copy new bootstrap tarball to IRIX
scp tmp/irix-bootstrap.tar.gz edodd@192.168.0.81:/tmp/

# On IRIX:
cd /opt/chroot
tar xzf /tmp/irix-bootstrap.tar.gz
export LD_LIBRARYN32_PATH=/opt/chroot/usr/sgug/lib32
/opt/chroot/usr/sgug/bin/rpm --version
```

---

## Build Order (FC40) - Status

| # | Package | Version | Status |
|---|---------|---------|--------|
| 1 | zlib-ng | 2.1.6 | COMPLETE |
| 2 | bzip2 | 1.0.8 | COMPLETE |
| 3 | popt | 1.19 | COMPLETE |
| 4 | openssl | 3.2.1 | COMPLETE |
| 5 | libxml2 | 2.12.5 | COMPLETE |
| 6 | curl | 8.6.0 | COMPLETE |
| 7 | xz | 5.4.6 | COMPLETE |
| 8 | lua | 5.4.6 | COMPLETE |
| 9 | file | 5.45 | COMPLETE |
| 10 | rpm | 4.19.1.1 | **COMPLETE** - LLD 18 fix |
| 11 | libsolv | 0.7.28 | COMPLETE |
| 12 | tdnf | 3.5.14 | Ready to test |

---

## Environment

| Purpose | Path |
|---------|------|
| Mogrix project | `/home/edodd/projects/github/unxmaal/mogrix/` |
| Cross toolchain | `/opt/cross/bin/` |
| LLD 18 binary | `tools/bin/ld.lld-irix-18` |
| Staging area | `/opt/sgug-staging/usr/sgug/` |
| IRIX sysroot | `/opt/irix-sysroot/` |
| Python venv | `.venv/bin/activate` |
| IRIX host | `ssh edodd@192.168.0.81` |
| IRIX chroot | `/opt/chroot` |

---

## Rebuilding LLD 18

If you need to rebuild LLD with additional patches:

```bash
cd /home/edodd/projects/github/unxmaal/mogrix
# Patches are in tmp/lld-irix-build/llvm-project-18.1.3.src/
# - lld/ELF/Writer.cpp (IRIX ELF compat)
# - lld/ELF/InputFiles.cpp (local symbol fix)
# - llvm/include/llvm/Object/ELF.h (sh_entsize fix)

cd tmp/lld-irix-build/llvm-project-18.1.3.src/build
ninja lld
cp bin/lld ../../tools/bin/ld.lld-irix-18
```

---

## Long-Term Goal

After tdnf works: port **WebKitGTK 2.38.x** for a modern browser on IRIX.
- Use `-DENABLE_JIT=OFF` for interpreter mode
- Most GTK3 dependencies already in SGUG-RSE
