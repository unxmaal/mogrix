# Linker Selection for IRIX Cross-Compilation

## Current Strategy (DO NOT CHANGE WITHOUT IRIX TESTING)

The `cross/bin/irix-ld` wrapper selects linkers based on output type:

| Output Type | Linker | Why |
|-------------|--------|-----|
| Shared libraries (`-shared`) | GNU ld + custom linker script + `-Bsymbolic-functions` | Standard 2-segment layout (RE+RW) for IRIX rld |
| Executables | LLD 18 with patches + `dlmalloc.o` | Correct relocations, mmap-based malloc |

### Shared Library Segment Layout (updated 2026-02-14)

IRIX rld expects shared libraries with exactly 2 LOAD segments: RE (text) + RW (data/got).

**DO NOT use `-z separate-code`**: Produces 3 LOAD segments (R+RE+RW) which corrupts rld
internal state on dlopen. Previously used (2026-02-05) but found to cause crashes.

The custom linker script (`cross/irix-shared.lds`) produces the correct 2-segment layout.
GNU ld uses it via `-T irix-shared.lds`.

### dlmalloc Architecture (2026-02-14)

dlmalloc is linked into **executables only** via `irix-ld` (as `dlmalloc.o` in the LLD
command). Shared libraries do NOT contain dlmalloc — they leave malloc/free/calloc/realloc
undefined so IRIX rld resolves them to the executable's single allocator.

**Why**: `-Bsymbolic-functions` (needed for rld's ~4370 global GOT entry limit) binds each
.so's function calls locally. If dlmalloc was in every .so, each library would get its own
private heap. When library A allocates and library B frees, dlmalloc detects a foreign
pointer and calls abort(). This was the root cause of the weechat TLS ABORT bug (session 37).

### C++ Static Constructors in Shared Libraries (2026-02-10)

IRIX rld does NOT process `.init_array` sections — it only processes `.init` functions
via `DT_INIT`. GNU ld's default linker script merges `.ctors` → `.init_array`, so even
with clang's `-fno-use-init-array` (which generates `.ctors` in .o files), the final .so
ends up with `.init_array` only. Static constructors silently don't run.

**Fix (3 parts):**
1. **Custom linker script** (`cross/irix-shared.lds`): Keeps `.ctors` in its own section
   instead of merging into `.init_array`. Also handles `.dtors`.
2. **CRT objects** (`cross/crt/crtbeginS.S`, `cross/crt/crtendS.S`): PIC versions of
   crtbeginT/crtendT. `crtbeginS.o` provides a `_init` function (in `.init` section)
   that walks the `.ctors` array calling each constructor. `crtendS.o` provides the
   NULL sentinel at `.ctors` end. The `_init` global symbol triggers GNU ld to create
   a `DT_INIT` entry pointing to it.
3. **irix-ld** links shared libs: `crtbeginS.o <user.o files> crtendS.o` with `-T irix-shared.lds`.

**Verification**: `readelf -d foo.so` must show `INIT` tag. `readelf -S foo.so` must show
`.ctors` section (not `.init_array`). `.init` section must also exist.

### rld Re-Encounter GOT Crash (2026-02-11)

When a shared library is loaded via `dlopen()`, and then another DSO references it as
`DT_NEEDED`, IRIX rld "re-encounters" the already-loaded library. During re-encounter,
rld re-processes the GOT and re-runs `DT_INIT`. For libraries with `MIPS_LOCAL_GOTNO > ~128`,
the global GOT entries above a certain index get zeroed during this re-processing.

The `__do_global_ctors_aux` function (from `crtbeginS.o`) reads `__CTOR_END__` from a global
GOT entry. On re-encounter, this entry is zeroed, causing `s0 = 0`, then `s0 = 0 - 4 = 0xFFFFFFFC`,
then `lw $25, 0($s0)` → SIGBUS at address 0xFFFFFFFC.

**Threshold**: Libraries with `LOCAL_GOTNO ≤ 103` are not affected. Those with `≥ 140` crash.
The exact threshold is between 103 and 140 (likely 128, a power-of-2 internal rld limit).

**Affected libraries**: harfbuzz (LOCAL_GOTNO=1250), Qt5Core (2714), Qt5Sql (140),
freetype (152, when it had .init). Pure C libraries without crtbeginS.o are unaffected.

**Fix**: Added `beqz $s0, .Ldone` guard in `crtbeginS.S` after the `lw %got(__CTOR_END__)($gp)`
instruction. If the GOT entry is zeroed (re-encounter), the function returns immediately
without walking the .ctors array. On first load, the GOT entry is valid and constructors
run normally.

**Why DT_INIT removal doesn't help**: Even when DT_INIT is stripped from the dynamic section,
rld still calls the `_init` symbol (found by name in dynsym) on re-encounter. The crash
happens in `__do_global_ctors_aux` which is called by `_init`.

**Test pattern**: `dlopen("libX.so"); dlopen("libY.so")` where Y has X as NEEDED.
Working: both loads succeed. Crashing: SIGBUS during second dlopen.

### GNU Symbol Versioning Crashes rld (2026-02-10)

`--version-script` creates `.gnu.version`, `.gnu.version_r`, and adds `VERNEED`/`VERSYM`
dynamic tags. IRIX rld predates GNU symbol versioning and crashes (SIGSEGV) when
processing these unknown tags during `dlopen()`.

**Fix**: `irix-ld` filters out `--version-script` (both `=FILE` and ` FILE` forms).
Symbols are still exported globally; they just don't have version tags. This is fine
for IRIX since rld doesn't support version-based symbol resolution anyway.

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
2. **Check segment layout**: `readelf -l binary` (shared libs need 2 LOAD segments: RE/RW; NOT 3)
3. **Check which linker is used**: Add `echo "Using: $GNULD" >&2` or `echo "Using: $LLD" >&2` to irix-ld

## See Also

- `lld-fixes/README.md` - How to build LLD 18 with patches
- `cross/bin/irix-ld` - The linker wrapper script
- `HANDOFF.md` - Session history documenting these failures
