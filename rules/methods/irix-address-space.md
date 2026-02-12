# IRIX n32 Address Space Layout

> **For AI agents**: This documents the IRIX MIPS n32 address space layout.
> Programs that use brk() for heap are limited to 176MB by libpthread's fixed mapping.
> Use mmap-based malloc (in mogrix-compat) to access the full 1.2GB free region.

> **Source**: SGI "Topics in IRIX Programming" (007-2478-008), Chapter 1: Process Address Space.
> Our probed map below is confirmed by the official documentation. Key official facts are
> annotated with `[SGI]` tags.

---

## Official Address Space Rules (from SGI docs)

- **Total user space**: 0x00000000 - 0x7FFFFFFF (2 GB). Above 0x80000000 is kernel. `[SGI p.3]`
- **Conventional lowest allocation**: 0x00400000 (4 MB). Below this is intentionally undefined to trap null pointers. `[SGI p.4]` *(Note: n32 cross-compiled binaries use 0x10000 as image base, well below the 4MB convention.)*
- **DSO addresses declared in** `/usr/lib/so_locations` — rld uses this for quickstart placement. `[SGI p.4]`
- **MIPS ABI reserved region**: 0x30000000 - 0x3FFC0000 reserved for user-defined mmap segments. `[SGI p.23]` *(This falls within our 1.2GB free gap.)*
- **brk() extends heap to a specific virtual address** — malloc() calls brk() as required. `[SGI p.6]`
- **Addresses cannot be undefined** once allocated by brk()/malloc(). Only process termination releases them. mmap'd regions can be munmap'd. `[SGI p.6]`
- **Virtual swap** (default since IRIX 5.2): malloc() never returns NULL until resource limit — pages are only defined when accessed. If swap exhausted at access time, process gets **SIGKILL** (not NULL). `[SGI p.7-8]`
- **Page size**: 4096 bytes on 32-bit systems (O2, etc.), larger and configurable on 64-bit systems. `[SGI p.5]`

### Resource Limits (systune defaults)

| Parameter | Default | Hex |
|-----------|---------|-----|
| rlimit_vmem_max | 536,870,912 | 0x20000000 (512 MB) |
| rlimit_stack_cur | 67,108,864 | 0x04000000 (64 MB) |

These can be changed via `systune` or `setrlimit()`. `[SGI p.6-7]`

**Warning**: Each `sproc()` child that doesn't supply its own stack reserves rlimit_stack_max bytes of address space. Programs creating many processes can hit rlimit_vmem_max quickly. `[SGI p.7]`

---

## Full Address Map (probed 2026-02-04, IP30/Octane2)

```
Address Range             Size      Contents
────────────────────────────────────────────────────────────
0x00000000 - 0x00010000   64 KB     Reserved (null page + low memory)
0x00010000 - 0x001XXXXX   varies    ** PROGRAM ** (default image base 0x10000)
0x001XXXXX - 0x00200000   varies    Heap (brk-based, ~1.8MB max at default base)
0x00200000 - 0x00240000   256 KB    rld quickstart table (RW, mostly zeros)
0x00240000 - 0x00AB0000   8 MB      FREE
0x00AB0000 - 0x00C00000   1.3 MB    libcrypto.so (OpenSSL)
0x00C00000 - 0x00F90000   3.5 MB    FREE
0x00F90000 - 0x01040000   704 KB    libm.so (IRIX math library)
0x01040000 - 0x0C080000   176 MB    FREE (heap grows here with high image base)
0x0C080000 - 0x0C0C0000   256 KB    libpthread.so (IRIX pthreads) ** HEAP CEILING **
0x0C0C0000 - 0x0F910000   56 MB     FREE
0x0F910000 - 0x0F980000   448 KB    libc.so text (part 1, basename etc.)
0x0F9E0000 - 0x0FBC0000   1.8 MB    libc.so text (part 2, aio/threads)
0x0FBD0000 - 0x0FC40000   448 KB    libc.so data (RW, placement policies)
0x0FC40000 - 0x5C000000   1,219 MB  FREE ** LARGEST GAP - mmap target **
0x5C000000 - 0x5C240000   2.2 MB    mogrix-compat libraries (text)
0x5C248000 - 0x5C9C0000   7.5 MB    mogrix-compat libraries (data, RW)
0x5C9D0000 - 0x5CB00000   1.2 MB    lua/other library text
0x5CB00000 - 0x5FFE0000   52 MB     FREE
0x5FFE0000 - 0x60080000   640 KB    rld text
0x60080000 - 0x7BFC1000   447 MB    FREE
0x7BFC1000 - 0x80000000   64 MB     STACK
0x80000000 - 0xFFFFFFFF   2 GB      Kernel space (inaccessible)
```

---

## Key Constraints

### brk() heap ceiling: libpthread.so at 0x0C080000

The heap grows via `brk()` from the end of BSS upward. It **cannot grow past the next mapped region**. For any program that links libpthread (directly or transitively), this ceiling is `0x0C080000`.

| Image Base | Heap Start | Heap Ceiling | Max Heap |
|------------|-----------|--------------|----------|
| 0x00010000 (default) | ~0x00038000 | 0x00200000 (rld) | **1.8 MB** |
| 0x01000000 (current) | ~0x0102C000 | 0x0C080000 (pthread) | **176 MB** |
| 0x02000000 (safer) | ~0x0202C000 | 0x0C080000 (pthread) | **160 MB** |

### The 176 MB ceiling is HARD

No image base change can exceed 176 MB of brk-based heap when libpthread is loaded. Mozilla/WebKitGTK need more than this.

### mmap has 1.2 GB available

The gap at `0x0FC40000 - 0x5C000000` (1,219 MB) is available for mmap allocations. A malloc implementation that uses mmap instead of brk() can access this entire region.

---

## Library Load Addresses

These are **fixed** by rld quickstart (declared in `/usr/lib/so_locations`). They do NOT change based on the program's image base. `[SGI p.4]`

When IRIX loads a DSO not declared in so_locations, rld seeks a segment that doesn't overlap any declared DSO and won't interfere with stack growth. `[SGI p.4]`

| Library | Address | Size |
|---------|---------|------|
| rld quickstart table | 0x00200000 | 256 KB |
| libcrypto.so | 0x00AB0000 | 1.3 MB |
| libm.so | 0x00F90000 | 704 KB |
| libpthread.so | 0x0C080000 | 256 KB |
| libc.so (text) | 0x0F910000 | ~2.3 MB |
| libc.so (data) | 0x0FBD0000 | 448 KB |
| Cross-compiled libs | 0x5C000000 | ~12 MB |
| rld (text) | 0x5FFE0000 | 640 KB |

**Note**: Cross-compiled libraries (sqlite, libxml2, etc.) are placed in the 0x5C region. IRIX system libraries are placed in the 0x0F9 region. libpthread is isolated at 0x0C08 due to its special requirements.

**Segment alignment**: In the MIPS ABI reserved region (0x30000000-0x3FFC0000), no two mapped segments can occupy the same 256 KB unit. This ensures segments start on different pages even with maximum page size. `[SGI p.23]`

---

## Solution: mmap-based malloc

mogrix-compat includes a malloc replacement (dlmalloc) configured for mmap-only mode:

```c
#define HAVE_MORECORE 0     /* Don't use sbrk/brk */
#define HAVE_MMAP 1         /* Use mmap for all allocations */
```

This gives every program access to the 1.2 GB mmap region regardless of image base. The `--image-base` linker flag is no longer needed as a workaround.

### How it works

1. `malloc()` calls `mmap(MAP_ANONYMOUS)` for new memory chunks
2. mmap places pages in the large free gap (0x0FC40000-0x5C000000)
3. `free()` calls `munmap()` to release pages back to the OS
4. Small allocations are served from larger mmap'd chunks (no per-allocation syscall)

### Why mmap-based malloc is superior to brk()

Per the SGI docs `[SGI p.6]`: brk-allocated addresses **cannot be undefined** — they persist until process exit. Even `free()` only makes memory reusable within the process; the pages remain defined in page tables and swap is still allocated. In contrast, `munmap()` truly releases pages back to the OS.

Additionally, with virtual swap enabled `[SGI p.7-8]`, brk-based malloc() never returns NULL on failure — instead the process gets **SIGKILL** when it finally touches an overcommitted page. mmap-based malloc avoids this because each allocation actually maps pages.

### Why not just increase image base?

- The heap ceiling is **always** 0x0C080000 (libpthread) regardless of base
- Max 176 MB of brk heap, not enough for large programs
- Changing image base can overlap with library mappings (0x00F90000 libm overlap discovered)
- mmap-based malloc works regardless of address layout

### mmap details (from SGI docs)

- `MAP_AUTOGROW`: IRIX-specific flag that extends the segment when storing past its end (zero-filled). `[SGI p.14]`
- `MAP_AUTORESRV`: IRIX-specific flag that delays reserving swap space until a store is done. `[SGI p.14]`
- `MAP_LOCAL`: IRIX-specific flag for sproc() — mapped segment not shared with child processes. `[SGI p.15]`
- `madvise()`: Can tell IRIX pages are unneeded — they stay defined but go to top of reclaim list. `[SGI p.29]`
- Segments based on `/dev/zero` with `MAP_AUTOGROW` delay page definition until accessed. `[SGI p.15]`

---

## How This Was Discovered

1. `rpm -Uvh ncurses-term` failed: "memory alloc (176 bytes) returned NULL"
2. par trace: `brk(0x244000) errno = 12 (Not enough space)`
3. Address space probe found rld mapping at 0x200000 blocking heap
4. `--image-base=0x1000000` moved program above rld, giving 176MB
5. Comprehensive probe revealed libpthread at 0x0C080000 as the true ceiling
6. mmap-based malloc bypasses brk() entirely, using the 1.2GB gap

---

## Memory Locking (real-time relevance)

Per `[SGI p.23-27]`: Page faults take 10-50ms. For real-time apps, lock pages with `mlock()`, `mlockall()`, `mpin()`, or `plock()`. Locking forces all pages to be defined (may trigger SIGKILL under virtual swap). Key rules:

- `plock(PROCLOCK)` locks text + data + stack + MAP_PRIVATE segments
- `plock()` does NOT lock MAP_SHARED segments — use `mpin()` for those
- Locking an `MAP_AUTOGROW` segment with `MCL_FUTURE` auto-locks new pages
- `mpin()` maintains a lock counter per page — must `munpin()` same number of times

Not directly relevant to mogrix bundles, but useful context for why certain IRIX programs have specific memory requirements.

---

## Probing the Address Space

The probe programs are in `/tmp/` (not committed). To re-probe on a different IRIX host:

```bash
# Cross-compile with same libraries as the target program
irix-cc -o probe addrspace-probe.c -lrpm -lrpmio -lpopt -lsqlite3 -ldl
# Copy to IRIX and run (via MCP tools)
irix_copy_to probe /tmp/probe
irix_exec "LD_LIBRARYN32_PATH=<paths> /tmp/probe"
```

Different IRIX hardware (IP27, IP30, IP32, IP35) or different library sets may produce different layouts. Always probe before assuming addresses.

### Checking so_locations

The official DSO placement file is at `/usr/lib/so_locations`. To see what addresses rld has reserved:

```bash
cat /usr/lib/so_locations
```

This file declares base addresses for all system DSOs. When a DSO is not listed, rld picks an address that doesn't overlap existing mappings. `[SGI p.4]`

### Checking resource limits

```bash
# From IRIX shell:
systune -i
systune-> rlimit_vmem_max    # default: 0x20000000 (512 MB)
systune-> rlimit_data_max     # max heap size
systune-> rlimit_stack_max    # max stack size
```

Or programmatically: `getrlimit(RLIMIT_VMEM, &rl)`, `setrlimit()` to change. `[SGI p.6-7]`

---

## Reference

- **SGI "Topics in IRIX Programming"** (007-2478-008): Chapter 1, pages 3-42. Official documentation for the IRIX process address space, mmap, memory locking, and CC-NUMA placement.
- **rld(1)**, **dso(5)**: Man pages for the IRIX dynamic linker and DSO format.
- **MIPSpro Compiling, Linking, and Performance Tuning Guide**: Additional details on image base, DSO placement.
