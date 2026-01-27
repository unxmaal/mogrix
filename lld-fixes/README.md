# LLD IRIX Patches for LLVM 18.1.3

This directory contains patches to make LLD 18.1.3 work with IRIX 6.5
cross-compilation. These patches fix several incompatibilities between
modern LLD and IRIX's ELF format expectations.

## Background

IRIX 6.5 uses an older MIPS ELF ABI that predates several modern features.
LLD 18 is stricter than earlier versions and rejects binaries/libraries
that don't conform to the current ELF spec. These patches add tolerance
for IRIX-specific quirks.

## Patches

### 00-original-irix-compat.patch
The original IRIX ELF compatibility patch (reference/documentation).
This was the starting point based on vvuk's LLD fork concepts.

### 01-writer-irix-elf-compat.patch
**File:** `lld/ELF/Writer.cpp`

Skips ELF features that crash IRIX's ELF parser/rld:
- PT_MIPS_ABIFLAGS program header
- PT_GNU_STACK program header
- PT_PHDR in shared libraries

These are conditionally disabled when `config->osabi == ELFOSABI_IRIX`.

### 02-inputfiles-mips-local-symbols.patch
**File:** `lld/ELF/InputFiles.cpp`

IRIX shared libraries have section symbols (.text, .data, .rodata) in the
global part of the symbol table. This violates the ELF spec but is how
IRIX linkers have always produced libraries.

LLD 18 error:
```
error: libc.so: invalid local symbol '.text' in global part of symbol table
```

This patch skips the error for section symbols (names starting with `.`)
on MIPS targets.

### 03-elf-header-sh-entsize-zero.patch
**File:** `llvm/include/llvm/Object/ELF.h`

GNU ld for MIPS produces .dynamic sections with sh_entsize=0. LLD 18
rejects these when linking against shared libraries:

```
error: librpm.so: section [index 3] has invalid sh_entsize: expected 8, but got 0
```

This patch allows sh_entsize=0 in the `getSectionContentsAsArray` template.

## Building LLD 18 with Patches

### Quick Rebuild (if already built)

```bash
cd /home/edodd/projects/github/unxmaal/mogrix
cd tmp/lld-irix-build/llvm-project-18.1.3.src/build
ninja lld
cp bin/lld ../../tools/bin/ld.lld-irix-18
```

### Full Build from Scratch

```bash
cd /home/edodd/projects/github/unxmaal/mogrix
./lld-fixes/build-lld-irix.sh
```

Or manually:

```bash
# Download LLVM 18.1.3
cd tmp
mkdir -p lld-irix-build && cd lld-irix-build
curl -LO https://github.com/llvm/llvm-project/releases/download/llvmorg-18.1.3/llvm-project-18.1.3.src.tar.xz
tar xf llvm-project-18.1.3.src.tar.xz
cd llvm-project-18.1.3.src

# Apply patches (or use sed as in build-lld-irix.sh)
patch -p1 < /path/to/01-writer-irix-elf-compat.patch
patch -p1 < /path/to/02-inputfiles-mips-local-symbols.patch
patch -p1 < /path/to/03-elf-header-sh-entsize-zero.patch

# Build
mkdir build && cd build
cmake -G Ninja \
    -DLLVM_ENABLE_PROJECTS="lld" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_COMPILER=clang-18 \
    -DCMAKE_CXX_COMPILER=clang++-18 \
    -DLLVM_TARGETS_TO_BUILD="Mips" \
    -DLLVM_DEFAULT_TARGET_TRIPLE="mips-sgi-irix6.5" \
    ../llvm
ninja lld

# Install
cp bin/lld /path/to/tools/bin/ld.lld-irix-18
```

## Using the Patched LLD

Update the irix-ld wrapper to use the new binary:

```bash
# In /opt/sgug-staging/usr/sgug/bin/irix-ld:
LLD="${IRIX_LLD:-/home/edodd/projects/github/unxmaal/mogrix/tools/bin/ld.lld-irix-18}"
```

## Problem Solved

These patches fix the MIPS relocation bug where LLD 14 (vvuk's fork)
generated incorrect relocations for external symbols in static data:

**Before (broken):**
```
R_MIPS_REL32      00000000   *ABS*
```

**After (fixed):**
```
R_MIPS_REL32      00000000   rpmcliAllPoptTable
```

This was causing rpm to fail with "unknown option" for all options because
the popt option tables had NULL pointers that IRIX's rld couldn't resolve.

## Files

| File | Description |
|------|-------------|
| `00-original-irix-compat.patch` | Original reference patch |
| `01-writer-irix-elf-compat.patch` | Skip IRIX-incompatible ELF features |
| `02-inputfiles-mips-local-symbols.patch` | Allow MIPS DSOs with local symbols |
| `03-elf-header-sh-entsize-zero.patch` | Accept sh_entsize=0 in .dynamic |
| `build-lld-irix.sh` | Build script (uses sed, not patch files) |
| `README.md` | This file |

## Binary Location

The built LLD binary is stored at:
```
tools/bin/ld.lld-irix-18
```

This is separate from the patches to avoid bloating the repository with a
52MB binary. The `tmp/lld-irix-build/` directory contains the full LLVM
source with patches applied if you need to rebuild.
