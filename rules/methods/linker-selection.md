# Linker Selection for IRIX Cross-Compilation

## Current Strategy (DO NOT CHANGE WITHOUT IRIX TESTING)

The `cross/bin/irix-ld` wrapper selects linkers based on output type:

| Output Type | Linker | Why |
|-------------|--------|-----|
| Shared libraries (`-shared`) | GNU ld + `-z separate-code` | Forces 3 LOAD segments (R/RE/RW) for IRIX rld |
| Executables | LLD 18 with patches | Correct relocations, correct segment layout |

### Shared Library Segment Layout (2026-02-05)

Without `-z separate-code`, GNU ld produces a **single RWE LOAD segment** for small
shared libraries (those with no `.data` section, only `.got`). This crashes IRIX rld
immediately after `elfmap()` with SIGSEGV. Affects all Perl XS modules (Dumper.so, etc.).

With `-z separate-code`, GNU ld produces 3 LOAD segments:
- `R` — headers, dynamic, hash, symtab, strtab
- `RE` — .text, .MIPS.stubs
- `RW` — .rodata, .got

IRIX rld handles this correctly (tested with `DynaLoader::dl_load_file`).

SGUG-RSE avoided this issue because their patched binutils + GCC produced compatible
single-segment layout. Our unpatched GNU ld + clang needs the explicit flag.

## KNOWN BAD FIXES - DO NOT ATTEMPT

### ❌ GNU ld for executables

**Symptom you're trying to fix**: R_MIPS_REL32 relocations without symbol names, causing NULL pointers in static data arrays (e.g., rpm's optionsTable pointing to poptHelpOptions).

**Why it fails**: GNU ld produces an executable segment layout that crashes IRIX rld with SIGSEGV during initialization (at ~7ms, before any application code runs).

**How the crash looks**:
```
7mS: syssgi(SGI_USE_FP_BCOPY, ...) = 0
7mS: received signal SIGSEGV
```

**Attempted and failed**: 2026-02-02 (multiple times across sessions)

### ❌ LLD 14 (vvuk's fork) for executables

**Binary**: `/opt/cross/bin/ld.lld-irix` (the OLD one)

**Why it fails**: Generates anonymous R_MIPS_REL32 relocations for external data symbols embedded in static arrays. The relocation has symbol index 0 (STN_UNDEF), so the dynamic linker can't resolve it.

**How it looks**:
```
00035f9c  00000003 R_MIPS_REL32     (no symbol)   <- BROKEN
```

**Symptom**: `rpm --help` returns "unknown option" because popt option tables have NULL pointers at runtime.

## Correct Solution

Use **LLD 18 with IRIX patches** from `lld-fixes/`:

**Binary**: `/home/edodd/projects/github/unxmaal/mogrix/tools/bin/ld.lld-irix-18`

The patches fix the relocation bug. Correct output looks like:
```
00035ffc  00007b03 R_MIPS_REL32      00000000   rpmInstallPoptTable
00036050  00007d03 R_MIPS_REL32      00000000   poptAliasOptions
0003606c  00007e03 R_MIPS_REL32      00000000   poptHelpOptions
```

## Debugging Linker Issues

If you suspect a linker problem:

1. **Check relocations**: `readelf -r binary | grep -E "(popt|rpm)"`
2. **Check segment layout**: `readelf -l binary` (shared libs need 3 LOAD segments: R/RE/RW)
3. **Check which linker is used**: Add `echo "Using: $GNULD" >&2` or `echo "Using: $LLD" >&2` to irix-ld

## See Also

- `lld-fixes/README.md` - How to build LLD 18 with patches
- `cross/bin/irix-ld` - The linker wrapper script
- `HANDOFF.md` - Session history documenting these failures
