# mrqs — Mogrix RQS (Library Rebasing Tool)

## Why We Need It

C++ executables cross-compiled for IRIX crash on any `dynamic_cast` because IRIX rld does not process R_MIPS_REL32 relocations for undefined symbols in executables. Typeinfo/RTTI pointers that reference shared library vtables (e.g., `__cxxabiv1::__si_class_type_info` in libstdc++) are never relocated, leaving them with stale link-time addends instead of correct runtime addresses.

The fix for this (in `fix-anon-relocs --pre-resolve-only`) is to pre-resolve these relocations at bundle creation time by reading the shared library symbol tables and writing the correct runtime addresses directly into the executable. But this only works if we know each library's actual runtime load address.

Currently all mogrix-built shared libraries share the same preferred base address (`0x0f800000`). When rld loads a bundle, it can only place one library at `0x0f800000` — all others get displaced to arbitrary addresses. Pre-resolved addresses become wrong because they assumed the link-time base.

IRIX's native solution is `rqs` (quickstart), which assigns unique base addresses to each library and rebases the binary in-place. We decompiled rqs via Ghidra (`docs/rqs/rqs_full_decompile.c`, 953 functions) and fully understand the algorithm. **mrqs** implements the same approach for mogrix-built libraries, running at bundle creation time.

## What It Does

A standalone Python tool (`cross/bin/mrqs`) that:

1. Takes a directory of shared libraries (the bundle's `_lib32/`)
2. Assigns unique, non-overlapping base addresses using a free-space allocator
3. Rebases each library in-place by computing a movement delta and patching all internal addresses
4. After rebasing, `fix-anon-relocs --pre-resolve-only` can pre-resolve executable relocations using the now-correct library addresses

## Address Space Layout

IRIX N32 processes have a 2GB virtual address space (`0x00000000 - 0x80000000`). mrqs allocates from the gaps between fixed regions:

```
0x00000000 - 0x01000000   Reserved (null page + low memory)
0x01000000 - 0x0C080000   Program text + heap (176MB, not available)
0x0C080000 - 0x0C0C0000   libpthread.so (IRIX native, FIXED)
0x0C0C0000 - 0x0F900000   *** ALLOCATABLE: 47MB ***
0x0F900000 - 0x0FC40000   libc.so.1 (IRIX native, FIXED)
0x0FC40000 - 0x5FFE0000   *** ALLOCATABLE: 1,283MB (primary region) ***
0x5FFE0000 - 0x60080000   rld + libgcc_s (FIXED)
0x60080000 - 0x7BFC0000   *** ALLOCATABLE: 447MB ***
0x7BFC0000 - 0x80000000   Stack (64MB)
```

Total allocatable: ~1,777MB. Libraries are sorted largest-first and placed top-down (first-fit descending) for better bin-packing. A dry-run on 241 staging libraries shows all fit with no overlaps.

## Multi-Bundle Handling

Each mogrix bundle is self-contained with its own `_lib32/` directory. Wrapper scripts set `LD_LIBRARYN32_PATH="$dir/_lib32:/usr/lib32"`, so rld loads from the bundle's private directory first. mrqs runs independently per bundle — no cross-bundle coordination needed. Two different bundles can have the same library at different addresses without conflict.

## Rebasing Algorithm

For each library, mrqs computes `delta = new_base - old_base` and patches every address-containing field:

| Target | Fields | Notes |
|--------|--------|-------|
| ELF header | `e_entry` | Only if non-zero |
| Program headers | `p_vaddr`, `p_paddr` | All types |
| Section headers | `sh_addr` | All sections with non-zero addr |
| `.dynamic` | Address-valued tags | DT_MIPS_BASE_ADDRESS, DT_INIT, DT_FINI, DT_HASH, DT_SYMTAB, DT_STRTAB, DT_REL, DT_JMPREL, DT_PLTGOT, DT_MIPS_MSYM, DT_MIPS_RLD_MAP, DT_MIPS_OPTIONS, DT_MIPS_CONFLICT, DT_MIPS_LIBLIST |
| `.dynsym` | `st_value` | Only defined symbols (st_shndx != 0, < 0xFF00) |
| `.got` | Local entries `[0..local_gotno)` | Unconditional adjust if value in old range |
| `.got` | Global entries `[local_gotno..]` | Only for DEFINED symbols (check .dynsym) |
| `.rel.dyn` | `r_offset` | Every non-zero entry |
| `.rel.dyn` targets | `*target` at each r_offset | For DEFINED symbol relocs: unconditional add delta. For anonymous (sym_idx=0): only if value in old range. UND symbol relocs: never adjust. |
| Data segment | Writable LOAD segments | Scan for remaining values in old range, skip .got (already handled) |

Key insight from rqs: the movement model is a single uniform delta applied to everything. This works because shared libraries are position-independent at the segment level — file offsets don't change, only virtual addresses do.

## Pipeline Integration

mrqs runs at bundle creation time inside `mogrix/bundle.py`:

```
BundleBuilder flow:
  1. Resolve dependencies → BundleManifest
  2. Extract RPMs to bundle dir
  3. Copy libraries to _lib32/
  4. Prune unused libs
  5. Strip RPATHs
  ──── mrqs rebases _lib32/*.so* ────
  6. fix-anon-relocs --pre-resolve-only on _bin/* executables
     (uses rebased library symbol tables for correct addresses)
  7. Create wrapper scripts
  8. Package tarball/installer
```

mrqs operates on the bundle's `_lib32/` copies, never the originals in staging. Safe and reversible. Skip with `MOGRIX_NO_MRQS=1`.

## Shared Code

ELF parsing utilities shared between mrqs and fix-anon-relocs are in `cross/lib/elf_utils.py`:
- Byte-level read/write (`read_u8/u16/u32/s32`, `write_u8/u16/u32`)
- Section parsing (`find_sections`, `find_dynstr`, `get_string`)
- Dynamic section (`find_dynamic_tags`, `find_dynamic_section`)
- Program headers (`build_load_segments`, `build_all_program_headers`, `vaddr_to_foff`)
- Library symbols (`parse_library_symbols`, `build_library_symbol_map`)

## irix-ld Changes

With mrqs handling address assignment at bundle time, `irix-ld` reverted to a fixed `--image-base=0x0f800000` for all shared libraries. The previous hash-based approach (cksum of soname → slot in 0x0a000000-0x5e000000 range) had collision probability (~1% for 30 libraries in 340 slots) and couldn't coordinate across bundles. mrqs's proper allocator eliminates collisions entirely.

## Current Status

**47/67 binaries pass** in the worker test bundle with mrqs enabled. 20 binaries still SIGSEGV. Without mrqs (`MOGRIX_NO_MRQS=1`), all 67 pass. The crash occurs after rld successfully loads all libraries at their rebased addresses (verified via par trace — elfmap addresses match mrqs assignments). The SIGSEGV happens during library initialization (`.init`/`.ctors`), suggesting some internal pointer in the rebased libraries isn't being adjusted.

### What's been tried and ruled out
- **Local-only GOT patching** → added global DEFINED entries too, no change
- **Relocation target patching** → both in_range-guarded and unconditional, no change
- **Data segment sweep** → scanned all writable LOAD segments for stale addresses, found 0 extra words
- **All of the above combined** → same 20 failures

### Remaining hypothesis
Something in the libraries' initialization code or data structures references an address that isn't covered by `.rel.dyn`, `.got`, or writable data segments. Possibilities:
- Read-only data (.rodata) containing embedded pointers (e.g., switch jump tables, constant struct initializers)
- `.eh_frame` FDE entries with absolute addresses (though these are typically pc-relative)
- MIPS-specific structures (`.MIPS.options`, reginfo) containing addresses
- Interaction between fix-anon-relocs link-time modifications and mrqs rebasing

The failing binaries all link libraries that have `.ctors` sections (constructor functions), while passing binaries tend to link libraries without constructors. The crash site (immediately after `syssgi(SGI_USE_FP_BCOPY)`) is consistent with a constructor being called at a wrong address.

## Files

| File | Description |
|------|-------------|
| `cross/bin/mrqs` | The rebasing tool (~450 lines Python) |
| `cross/lib/elf_utils.py` | Shared ELF parsing utilities |
| `cross/bin/fix-anon-relocs` | Updated to import from elf_utils, added `--pre-resolve-only` mode |
| `cross/bin/irix-ld` | Reverted to fixed 0x0f800000 base |
| `mogrix/bundle.py` | Calls mrqs + fix-anon-relocs at bundle time |
