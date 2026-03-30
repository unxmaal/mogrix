# fix-anon-relocs: Anonymous Relocation Post-Processing

## The Problem

IRIX rld ignores `R_MIPS_REL32` relocations with `sym_idx=0` (anonymous). LLD emits these
for function pointers in `.data.rel.ro`, `.ctors`, and `.data` sections. When a shared library
is loaded at a different address than its preferred base (`MIPS_BASE_ADDRESS`), these data
pointers remain at their link-time values instead of being relocated.

All mogrix shared libraries use `--image-base=0x0f800000` (set by irix-ld). When multiple
libraries are loaded in a single process, rld can only place ONE at 0x0f800000 — the rest
are displaced. Displaced libraries with anonymous relocations have wrong function pointers.

**Symptoms**: SIGSEGV during process startup, often in .ctors (constructor calls wrong function)
or in function pointer dispatch tables. The crash address is typically in a DIFFERENT library
(the one occupying the displaced library's original base address).

## The Fix

`cross/bin/fix-anon-relocs` rewrites anonymous `R_MIPS_REL32` entries to reference a named
symbol with `STV_PROTECTED` visibility, allowing rld's `fix_all_defineds()` to process them.

### When It Should Run

After EVERY shared library link (irix-ld post-link step, lines 255-260).

### How To Verify

```bash
# Check a specific library
llvm-readelf --dyn-syms libfoo.so | grep PROTECTED
# Should show >= 1 PROTECTED symbol if the library has anonymous relocs

# Check if anonymous relocs exist
llvm-readobj --relocations libfoo.so | grep "R_MIPS_REL32 -" | wc -l
# If > 0 and no PROTECTED symbols: fix-anon-relocs didn't run!

# Bulk audit all staging libraries
for lib in staging/opt/mogrix/lib32/*.so; do
  anon=$(llvm-readobj --relocations "$lib" 2>/dev/null | grep -c "R_MIPS_REL32 -")
  prot=$(llvm-readelf --dyn-syms "$lib" 2>/dev/null | grep -c PROTECTED)
  [ "$anon" -gt 0 ] && [ "$prot" -eq 0 ] && echo "BROKEN: $(basename $lib) ($anon anon relocs)"
done
```

### Manual Fix

```bash
python3 cross/bin/fix-anon-relocs path/to/lib.so path/to/lib.so
```

## Known Bug (2026-03-29)

fix-anon-relocs is NOT running during rpmbuild. All ~150 shared libraries in staging have
0 PROTECTED symbols despite having anonymous relocations. The tool exists in irix-ld's
post-link step but never executes.

**Suspected causes** (not yet confirmed):
1. Libtool bypasses irix-ld by calling the linker directly
2. The output file name detection in irix-ld fails
3. `python3` not in PATH during rpmbuild's restricted environment
4. CC driver (irix-cc) calls LLD with a different invocation path

**Impact**: Any bundle with >1 shared library is potentially broken. The first library at
preferred base works; all displaced libraries have wrong data pointers. bash+readline was
the first confirmed crash (libtinfo displaced, 1502 wrong function pointers).

**History**: First discovered with libevent in tmux bundle. Logged as "need to investigate
why irix-ld post-processing failed silently." Never investigated. Rediscovered 2026-03-29
during bash crash investigation.

## After Fixing the Build Bug

1. Rebuild ALL shared library packages (or bulk-fix staging with manual fix-anon-relocs)
2. Re-stage all libraries
3. Rebuild and re-bundle executables
4. Verify: every .so with anonymous relocs must have PROTECTED symbols
