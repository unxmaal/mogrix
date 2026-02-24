# WebKit JSC JIT on IRIX MIPS N32

## Overview

Getting JavaScriptCore's JIT working on IRIX MIPS required solving 10+ issues across the LLVM compiler, the LLInt offline assembler, the JIT code emitter, and the N32 ABI. This took sessions 122-123 (~12 build iterations).

## The Big Problem: Clang 56GB Memory Bug

**Symptom:** Cross-compiling `LowLevelInterpreter.cpp` (57K lines after macro expansion) for MIPS causes clang's cc1 to consume 56GB+ RAM — vs 95MB for the same file targeting x86.

**Root cause:** Bug in clang 16/18's integrated assembler for MIPS inline assembly in large translation units. The frontend itself uses 126MB; the backend/codegen explodes.

**Fix:** Three-step compilation bypassing cc1's integrated assembler:
1. `clang -emit-llvm -c` → LLVM bitcode (126MB RAM, ~5s)
2. `llc -filetype=asm` → MIPS assembly text (42MB RAM, ~3s)
3. `mips-sgi-irix6.5-as` → object file (negligible)

Total: 168MB instead of 56GB. Implemented in `patches/packages/webkitgtk/compile-llint-twostep.sh`, injected via cmake `RULE_LAUNCH_COMPILE` on the `LowLevelInterpreterLib` target.

## Three-Step Compile: Post-Processing Required

The llc → GNU as path requires assembly post-processing (step 2.5) to fix several LLVM MIPS backend issues:

### .size directives (LLVM bug)
llc emits `.size` with non-absolute expressions. GNU as accepts them but produces entries that confuse lld. Safe to strip — IRIX rld uses `.MIPS.options`, not ELF `.size`.

### .file directives (GNU as bug)
GNU as places the FILE symbol after SECTION symbols in the symtab, corrupting `sh_info`. lld rejects with "invalid binding: 0". Strip all `.file` directives.

### $tmp labels (llc quirk)
llc generates `$tmpN:` labels as LOCAL symbols interleaved with GLOBALs, violating ELF symbol ordering requirements. They're only definitions (never referenced). Strip them.

### GOT overflow — xgot conversion
`libjavascriptcoregtk`'s GOT exceeds 64KB. llc generates `%got(SYM)($gp)` and `%got_disp(SYM)($gp)` with R_MIPS_GOT16/GOT_DISP relocations (16-bit offset, overflows). Clang natively uses GOT_PAGE/GOT_OFST pairs which don't have this limit.

**Fix:** Perl post-process converts `%got()` / `%got_disp()` to `%got_hi()` / `%got_lo()` pairs using `$at` (`$1`) as a temp register. These produce R_MIPS_GOT_HI16/GOT_LO16 (32-bit).

### llc flags
```
llc -mtriple=mips64-sgi-irix6.5 -target-abi n32 -mattr=+xgot -mcpu=mips4 -relocation-model=pic -O0
```
- `-target-abi n32`: Required. Without it, `mips64` triple generates 64-bit address relocations (R_MIPS_HIGHER/HIGHEST) even with 32-bit pointer data layout.
- `-mattr=+xgot`: Enables 32-bit GOT pairs for function calls (CALL_HI16/CALL_LO16).
- `-O0`: Matches the source file's compile flags (debug/unoptimized LLInt).

### GNU as flags
```
mips-sgi-irix6.5-as -mabi=n32 -march=mips4 -mfp64 -KPIC
```
- `-KPIC`: Sets PIC/CPIC ELF flags matching clang-compiled objects.
- `-mfp64`: FR=1 mode (64-bit FP registers), matching the rest of the build.

### ccache interaction
cmake invokes `<RULE_LAUNCH_COMPILE> [ccache] <compiler> <flags...>`. The wrapper must detect `*/ccache` as `$1` and shift past it. Step 3 (assembly) must skip ccache — use the compiler/assembler directly.

## LLInt Offline Assembler Fixes

The offline assembler (`offlineasm/mips.rb`) generates MIPS assembly from a DSL. Several outputs are wrong for N32:

### MIPS32 `mul` instruction
Offline asm emits `mul rd,rs,rt` — a MIPS32 instruction not available on MIPS III/IV (R10000). **Fix:** Replace with `mult rs,rt` + `mflo rd` (two instructions, same result).

### O32 register names ($t4-$t7)
Offline asm uses O32 register names `$t4`-`$t7`. In N32, registers 12-15 are `$t0`-`$t3`; the `$t4`-`$t7` names don't exist. **Fix:** Use numeric names `$12`, `$13`, `$15`.

### .cpload (O32 PIC directive)
Offline asm emits `.cpload` at each label — an O32 PIC directive that sets up `$gp`. In N32, `$gp` is callee-saved (set once in prologue, stays valid). `.cpload` generates incorrect `_gp_disp` relocations. **Fix:** Gate on `#if defined(WTF_MIPS_PIC) && !defined(__sgi)`.

### Push/pop stack alignment
Offline asm uses `addiu $sp, $sp, ±4` for push/pop (4-byte slots). N32 ABI requires 8-byte minimum stack alignment at call boundaries. **Fix:** Change to ±8. Wastes 4 bytes per operation but guarantees alignment.

## JIT Code Emitter Fixes

### Big-endian JSValue32 tag/payload swap
`storePair32` / `loadPair32` argument order in `AssemblyHelpers.h` assumes little-endian layout. On big-endian MIPS, tag and payload words are swapped in memory.

- 7 of 10 affected functions have `static_assert` guards → compile error (caught)
- 3 functions silently corrupt: `loadValue(void*)`, both `storeTrustedValue` variants

**Fix:** `#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__` conditional swapping arguments. Patch: `patches/packages/webkitgtk/jit-bigendian-jsvalue32.patch`.

### FP64 vmov without mfhc1/mthc1
MIPS III/IV with FR=1 (64-bit FP regs) lacks the `mfhc1`/`mthc1` instructions (MIPS32r2). WebKit's `MIPSAssembler.h` only has paths for `WTF_MIPS_ISA_REV(2) && WTF_MIPS_FP64` and `!WTF_MIPS_FP64`.

**Fix:** Add a `WTF_MIPS_FP64 && !ISA_REV(2)` path: spill the double to stack via `sdc1`, load as two `lw` words (and reverse for `vmov` to FP). Big-endian: high word at lower address. Patch: `patches/packages/webkitgtk/jit-mips-vmov-fp64.patch`.

## Other JIT Enablement

### OS gate
JIT is gated on `OS(LINUX)` in `PlatformEnable.h`. Add `|| defined(__sgi)`.

### Cache flushing
`ExecutableAllocator.h` includes `sys/cachectl.h` only on Linux. IRIX has the same header. Add `|| defined(__sgi)`.

### MUST_TAIL_CALL
MIPS backend can't guarantee tail call elimination. Disable `[[clang::musttail]]` by patching `Compiler.h` to return 0 for `__has_cpp_attribute(clang::musttail)`.

### Branch range overflow
`ColorConversion.cpp` and `ColorLuminance.cpp` expand to huge template functions. LLVM's MIPS assembler can't relax 16-bit branch offsets → "out of range PC16 fixup". **Fix:** Exclude from unified builds (`@no-unify`) and compile at `-O1`.

## Files

| File | Purpose |
|------|---------|
| `patches/packages/webkitgtk/compile-llint-twostep.sh` | Three-step compile wrapper |
| `patches/packages/webkitgtk/jit-bigendian-jsvalue32.patch` | Big-endian JSValue32 fix (10 functions) |
| `patches/packages/webkitgtk/jit-mips-vmov-fp64.patch` | FP64 vmov without mfhc1/mthc1 |
| `rules/packages/webkitgtk.yaml` | All prep_commands for JIT enablement |
