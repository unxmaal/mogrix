/*
 * mmap-based malloc for IRIX
 *
 * Contains dlmalloc 2.8.6 (MIT-0 license) configured to use mmap instead
 * of brk/sbrk for all allocations. This bypasses the IRIX n32 heap
 * ceiling caused by libpthread.so mapped at 0x0C080000.
 *
 * With brk-based malloc, the heap is limited to ~176MB (the gap between
 * the program and libpthread). With mmap-based malloc, allocations go
 * to the 1.2GB free region at 0x0FC40000-0x5C000000.
 *
 * See rules/methods/irix-address-space.md for the full address map.
 *
 * Configuration:
 *   HAVE_MORECORE=0   - Don't use sbrk/brk
 *   HAVE_MMAP=1       - Use mmap for all allocations
 *   USE_LOCKS=1        - Thread-safe via spin locks (MIPS ll/sc atomics)
 *   USE_SPIN_LOCKS=1   - Use GCC atomics, no libpthread dependency
 *   MMAP_CLEARS=1     - IRIX mmap returns zeroed pages
 *   malloc_getpagesize=16384 - IRIX page size
 *
 * IRIX notes:
 *   - No MAP_ANONYMOUS: dlmalloc falls back to /dev/zero (built-in)
 *   - MIPS is big-endian: no endianness issues with dlmalloc
 *   - n32 ABI: 32-bit pointers, 32-bit size_t
 *   - Thread safety: IRIX pthreads (sproc-based) can call malloc from
 *     any thread. Without locking, concurrent mallocs corrupt the heap
 *     (confirmed: 6/10 runs crash). Spin locks add ~2 ll/sc instructions
 *     per malloc/free call with zero libpthread overhead.
 */

/* Configuration - must come before dlmalloc source */
#define HAVE_MORECORE 0
#define HAVE_MMAP 1
#define USE_LOCKS 1
#define USE_SPIN_LOCKS 1
#define MMAP_CLEARS 1
#define malloc_getpagesize 16384

/* Do NOT define USE_DL_PREFIX - we want standard malloc/free/realloc names
 * so our implementation overrides libc's. dlmalloc uses #ifndef to check. */
#undef USE_DL_PREFIX

/* Reasonable defaults for embedded/cross-compiled targets */
#define DEFAULT_GRANULARITY ((size_t)256U * (size_t)1024U)
#define DEFAULT_TRIM_THRESHOLD ((size_t)2U * (size_t)1024U * (size_t)1024U)
#define DEFAULT_MMAP_THRESHOLD ((size_t)256U * (size_t)1024U)

/* Safety */
#define ABORT_ON_ASSERT_FAILURE 0
#define PROCEED_ON_ERROR 0
#define INSECURE 0

/* IRIX needs these headers */
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

/*
 * dlmalloc 2.8.6 source inlined below.
 * Downloaded from: https://gee.cs.oswego.edu/pub/misc/malloc.c
 * License: MIT-0 (Copyright 2023 Doug Lea)
 *
 * The source is stored separately in dlmalloc-src.inc for maintainability.
 * Both files are copied to mogrix-compat/ during the build, so the
 * #include finds dlmalloc-src.inc in the same directory.
 */
#include "dlmalloc-src.inc"
