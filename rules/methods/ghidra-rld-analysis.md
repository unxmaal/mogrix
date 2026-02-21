# Ghidra Headless Analysis & rld Decompilation

## Overview

IRIX 6.5's runtime linker (`rld`) is a closed-source binary at `/usr/lib32/rld` (n32 ABI) and `/lib32/rld` (stripped). The debug version at `/usr/lib32/rld.debug` contains symbol names. We decompiled it using Ghidra headless to understand relocation processing, GOT management, and symbol resolution — knowledge that directly led to `fix-anon-relocs` and the two-symbol approach.

## Obtaining rld Binary

```sh
# From IRIX host (via MCP):
irix_host_exec "cp /usr/lib32/rld.debug /tmp/rld.debug && cp /lib32/rld /tmp/rld"

# Copy to Linux build host:
scp edodd@192.168.0.81:/opt/chroot/tmp/rld.debug /tmp/rld.debug

# Verify:
file /tmp/rld.debug
# ELF 32-bit MSB executable, MIPS, N32 MIPS64 rel2 version 1 (SYSV)
```

rld is mapped at base address `0x0fb60000` on IRIX.

## Ghidra Headless Invocation

**Critical**: Use `MIPS:BE:32:default` for rld (it's a 32-bit binary). Do NOT use `-cspec n32` (unsupported). Ghidra auto-detects the compiler spec from ELF headers.

**For N32 shared libraries**: Use `MIPS:BE:64:default` because N32 uses 64-bit registers (`sd`/`ld` instructions). `MIPS:BE:32` flags these as "bad instruction".

**Ghidra NPE on multi-GOT**: Ghidra 12.0.3 crashes with NullPointerException in `MIPS_ElfExtension.fixupGot()` on libraries with large multi-GOTs (e.g., libjavascriptcoregtk with 125K GOT entries). Works fine on normal-GOT libraries (libgtk-3, rld itself).

### Step 1: Decompile Specific Functions

```sh
# Write a Java script (Ghidra uses Java, not Python, for headless scripts)
cat > /tmp/DecompileRld.java << 'EOF'
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionIterator;
import java.io.FileWriter;
import java.io.PrintWriter;

public class DecompileRld extends GhidraScript {
    @Override
    public void run() throws Exception {
        String[] funcNames = {
            "resolve_relocations", "find_reloc", "resolve_symbol",
            "obj_dynsym_got", "is_symbol_in_got", "lazy_text_resolve"
        };
        DecompInterface decomp = new DecompInterface();
        decomp.openProgram(currentProgram);
        PrintWriter out = new PrintWriter(new FileWriter("/tmp/rld_key_functions.c"));

        FunctionIterator iter = currentProgram.getFunctionManager().getFunctions(true);
        while (iter.hasNext()) {
            Function f = iter.next();
            for (String name : funcNames) {
                if (f.getName().equals(name)) {
                    DecompileResults res = decomp.decompileFunction(f, 120, monitor);
                    if (res.decompileCompleted()) {
                        out.println("// === " + f.getName() + " @ " + f.getEntryPoint() +
                            " (" + f.getBody().getNumAddresses() + " bytes) ===");
                        out.println(res.getDecompiledFunction().getC());
                        out.println();
                    }
                }
            }
        }
        out.close();
        println("Key functions written to /tmp/rld_key_functions.c");
    }
}
EOF

# Run it:
/opt/ghidra_12.0.3_PUBLIC/support/analyzeHeadless /tmp/ghidra_project rld_analysis \
    -import /tmp/rld.debug \
    -postScript /tmp/DecompileRld.java -scriptPath /tmp \
    -deleteProject -processor "MIPS:BE:32:default" 2>&1 | tail -20
```

### Step 2: Find Callers of a Function

```sh
cat > /tmp/DecompileRld2.java << 'EOF'
// Finds all callers of resolve_relocations and decompiles them
import ghidra.app.decompiler.*;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.*;
import java.io.*;
import java.util.*;

public class DecompileRld2 extends GhidraScript {
    @Override
    public void run() throws Exception {
        DecompInterface decomp = new DecompInterface();
        decomp.openProgram(currentProgram);
        PrintWriter out = new PrintWriter(new FileWriter("/tmp/rld_reloc_callers.c"));

        // Find resolve_relocations
        Function target = null;
        FunctionIterator iter = currentProgram.getFunctionManager().getFunctions(true);
        while (iter.hasNext()) {
            Function f = iter.next();
            if (f.getName().equals("resolve_relocations")) { target = f; break; }
        }
        if (target == null) { println("Not found"); return; }

        // Find all callers via references
        Set<Function> callers = new HashSet<>();
        ReferenceIterator refs = currentProgram.getReferenceManager()
            .getReferencesTo(target.getEntryPoint());
        while (refs.hasNext()) {
            Reference ref = refs.next();
            Function caller = currentProgram.getFunctionManager()
                .getFunctionContaining(ref.getFromAddress());
            if (caller != null) callers.add(caller);
        }

        for (Function f : callers) {
            DecompileResults res = decomp.decompileFunction(f, 120, monitor);
            if (res.decompileCompleted()) {
                out.println("// === " + f.getName() + " @ " + f.getEntryPoint() + " ===");
                out.println(res.getDecompiledFunction().getC());
                out.println();
            }
        }
        out.close();
    }
}
EOF
```

### Step 3: Full Decompilation (all functions)

```sh
cat > /tmp/DecompileRldAll.java << 'EOF'
import ghidra.app.decompiler.*;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
import java.io.*;

public class DecompileRldAll extends GhidraScript {
    @Override
    public void run() throws Exception {
        DecompInterface decomp = new DecompInterface();
        decomp.openProgram(currentProgram);
        PrintWriter out = new PrintWriter(new FileWriter("/tmp/rld_full_decompile.c"));
        int count = 0, failed = 0;
        FunctionIterator iter = currentProgram.getFunctionManager().getFunctions(true);
        while (iter.hasNext()) {
            Function f = iter.next();
            DecompileResults res = decomp.decompileFunction(f, 120, monitor);
            if (res.decompileCompleted()) {
                out.println("// === " + f.getName() + " @ " + f.getEntryPoint() +
                    " (" + f.getBody().getNumAddresses() + " bytes) ===");
                out.println(res.getDecompiledFunction().getC());
                out.println();
                count++;
            } else { out.println("// FAILED: " + f.getName()); failed++; }
        }
        out.println("// Total: " + count + " decompiled, " + failed + " failed");
        out.close();
        println("Full decompilation: " + count + " functions → /tmp/rld_full_decompile.c");
    }
}
EOF

/opt/ghidra_12.0.3_PUBLIC/support/analyzeHeadless /tmp/ghidra_project rld_full \
    -import /tmp/rld.debug \
    -postScript /tmp/DecompileRldAll.java -scriptPath /tmp \
    -deleteProject 2>&1 | tail -10

# Copy result to project docs:
cp /tmp/rld_full_decompile.c docs/rld/rld_full_decompile.c
```

Output: `docs/rld/rld_full_decompile.c` (903KB, 32,674 lines, hundreds of functions).

### Analyzing Other Libraries (e.g., libgtk-3 for caller analysis)

```sh
# Ghidra works on normal-GOT libs
mkdir -p /tmp/ghidra_gtk
/opt/ghidra_12.0.3_PUBLIC/support/analyzeHeadless /tmp/ghidra_gtk GtkAnalysis \
    -import path/to/libgtk-3.so.0.2409.32 \
    -postScript /tmp/DecompileCrash.java -scriptPath /tmp/ghidra_scripts \
    -deleteProject -processor "MIPS:BE:64:default" 2>&1 | tail -20
```

## Key rld Findings (from decompilation)

### Where decompiled output lives

| File | Contents |
|------|----------|
| `docs/rld/rld_full_decompile.c` | All ~400 functions (903KB) |
| `docs/rld/rld_key_functions.c` | Curated: resolve_relocations, find_reloc, resolve_symbol, obj_dynsym_got, is_symbol_in_got, lazy_text_resolve |
| `docs/rld/rld_reloc_callers.c` | All callers of resolve_relocations (fix_all_defineds, search_for_conflicts, etc.) |
| `docs/rld/rld_init_functions.c` | map_object_into_mem_and_init_object_info, execute_all_init_sections, obj_init |

### Critical Discovery: Anonymous R_MIPS_REL32 Handling

**`fix_all_defineds()` at `0x0fb68ef0`**: Iterates `.dynsym` starting at index **1** (skips index 0). Only processes symbols where `st_other != 0 && st_other != 4`. This means:
- Anonymous relocs (sym_idx=0) are **never processed**
- Symbols with default visibility (st_other=0) or PROTECTED (st_other=4) are skipped unless they're STB_LOCAL

**`find_reloc()` at `0x0fb6b6c0`**: Binary search in `.rel.dyn`. The backward-walk has a boundary check that **prevents reaching entry [0]**:
```c
while (param_2 == *(uint *)(iVar7 + -4) >> 8) || (iVar6 == iVar7 + -8)
```
This stops when it reaches `base == ptr-8`, meaning position [0] is unreachable via backward walk (only reachable if binary search lands there directly).

**`resolve_relocations()` at `0x0fb67de0`**: R_MIPS_REL32 formula:
```
result = resolved_addr + (*target - sym_value)
```
LLD's multi-GOT mode writes `*target = addend` (usually 0), but rld expects `*target = st_value + addend`. This causes wrong resolved values.

### fix-anon-relocs: The Two-Symbol Approach

These discoveries led directly to `cross/bin/fix-anon-relocs`:

1. **STV_PROTECTED (value 3)** forces `fix_all_defineds()` to process a symbol (satisfies `st_other != 0 && st_other != 4`)
2. **Two symbols** used: `bulk_idx` (high) gets most relocs, `solo_idx` (low) gets exactly one. After sorting by sym_idx, solo's entry is at position [0] (reachable by direct binary search hit), bulk's entries are at [1..N-1] (reachable normally)
3. **Phase 1**: Fix named R_MIPS_REL32 addend mismatch (pre-add st_value to target)
4. **Phase 2**: Repoint anonymous R_MIPS_REL32 entries to bulk_idx, one to solo_idx
5. **Clear .MIPS.msym**: Zero the acceleration table so rld rebuilds via find_reloc()

### GOTSYM Threshold

rld only searches `.dynsym` entries at indices >= `DT_MIPS_GOTSYM` for external resolution. Symbols below this index are invisible even if GLOBAL DEFAULT. Fix: stub implementations via `_RLDN32_LIST` preload (see `patches/packages/webkitgtk/got_fixup.c`).

### _RLDN32_LIST + DT_INIT = SIGILL

rld doesn't correctly relocate `DT_INIT` addresses for `_RLDN32_LIST` libraries loaded at non-preferred addresses. If the library loads at a different address than `--image-base`, `DT_INIT` jumps to the wrong location → SIGILL. **Fix**: Only use _RLDN32_LIST for constructor-free libraries.

### rld Debug Flags

From decompilation of `_RLD_ARGS` processing:
```sh
export _RLD_ARGS="-log /tmp/rld.log"     # Log symbol resolution to file
export _RLD_ARGS="-v"                     # Verbose
export _RLD_ARGS="-trace"                 # Trace library loading
export _RLD_ARGS="-y <symname>"           # Track specific symbol
```

## Common Ghidra Pitfalls

1. **`-cspec n32` doesn't exist** — let Ghidra auto-detect from ELF headers
2. **MIPS:BE:32 can't decode `sd`/`ld`** — N32 uses 64-bit registers; use MIPS:BE:64
3. **NPE on multi-GOT libraries** — Ghidra 12.0.3's `MIPS_ElfExtension.fixupGot()` crashes on 125K+ GOT entries. Use llvm-objdump-18 instead for these
4. **`-deleteProject`** — Always use this for headless analysis; stale projects cause conflicts
5. **Script must match filename** — `DecompileRld.java` must contain `public class DecompileRld`
6. **Use `-scriptPath /tmp`** — Ghidra won't find scripts in arbitrary directories otherwise
