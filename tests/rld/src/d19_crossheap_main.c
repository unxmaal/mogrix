/*
 * D19: dlmalloc cross-heap STRESS test - main
 *
 * Simulates WebKit-scale allocation: 200MB+ total heap, mixed sizes,
 * cross-library alloc/free, fragmentation, then verifies GOT/function
 * pointers still work after the heap has been beaten to death.
 *
 * What we're hunting:
 * - sbrk arena growing into rld's shared library mapping region
 * - mmap'd chunks landing on GOT or .text pages
 * - dlmalloc metadata corruption at scale
 * - address space exhaustion on MIPS n32 (4GB limit)
 * - function pointer / GOT corruption after massive heap growth
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern void lib_free(void *p);
extern void *lib_malloc(int size);
extern char *lib_strdup(const char *s);

/* Import a function we'll call AFTER heap abuse to check GOT integrity */
extern int lib_crosscheck(int x);

/* Function pointer we'll store before heap abuse and call after */
static int (*saved_fn)(int) = NULL;

static int my_multiply(int x) { return x * 7; }

/* Track high-water mark of addresses to detect address space issues */
static unsigned long addr_min = 0xFFFFFFFF;
static unsigned long addr_max = 0;

static void track_addr(void *p) {
    unsigned long a = (unsigned long)p;
    if (a < addr_min) addr_min = a;
    if (a > addr_max) addr_max = a;
}

/*
 * Phase 1: Grow the heap to 200MB+ with retained allocations.
 * Mix exe and lib allocations to stress cross-heap.
 */
static int phase1_grow_heap(void) {
    printf("Phase 1: Growing heap past 200MB...\n");

    /* Hold onto these — don't free. Force heap to stay large. */
    #define HOLD_COUNT 2048
    static void *held[HOLD_COUNT];
    int i;
    size_t total = 0;

    for (i = 0; i < HOLD_COUNT; i++) {
        /* Alternate between exe and lib allocation */
        /* Mix sizes: mostly 128KB chunks but sprinkle in big ones */
        size_t sz;
        if (i % 50 == 0) {
            sz = 4 * 1024 * 1024;  /* 4MB chunks every 50th */
        } else if (i % 10 == 0) {
            sz = 1024 * 1024;      /* 1MB chunks every 10th */
        } else {
            sz = 128 * 1024;       /* 128KB chunks normally */
        }

        if (i % 2 == 0) {
            held[i] = malloc(sz);
        } else {
            held[i] = lib_malloc(sz);
        }

        if (!held[i]) {
            fprintf(stderr, "  alloc failed at i=%d, total=%zuMB\n",
                    i, total / (1024*1024));
            /* Free what we have and report */
            int j;
            for (j = 0; j < i; j++) {
                if (j % 2 == 0) lib_free(held[j]);  /* cross-free */
                else free(held[j]);                   /* cross-free */
            }
            return 1;
        }

        /* Write a pattern to catch heap corruption */
        memset(held[i], (unsigned char)(i & 0xFF), sz);
        track_addr(held[i]);
        total += sz;
    }

    printf("  Allocated %zuMB in %d chunks\n", total / (1024*1024), HOLD_COUNT);
    printf("  Address range: 0x%08lX - 0x%08lX (span %luMB)\n",
           addr_min, addr_max, (addr_max - addr_min) / (1024*1024));

    /* Verify patterns survived */
    int corrupt = 0;
    for (i = 0; i < HOLD_COUNT; i++) {
        unsigned char expected = (unsigned char)(i & 0xFF);
        unsigned char *p = (unsigned char *)held[i];
        if (p[0] != expected || p[1] != expected) {
            fprintf(stderr, "  CORRUPT at chunk %d: got 0x%02X, want 0x%02X\n",
                    i, p[0], expected);
            corrupt++;
        }
    }
    if (corrupt) {
        fprintf(stderr, "  %d chunks corrupted!\n", corrupt);
        return 1;
    }
    printf("  Pattern verification: OK\n");

    /* Now cross-free everything (exe allocs freed by lib and vice versa) */
    for (i = 0; i < HOLD_COUNT; i++) {
        if (i % 2 == 0)
            lib_free(held[i]);   /* exe-allocated, lib-freed */
        else
            free(held[i]);       /* lib-allocated, exe-freed */
    }
    printf("  Cross-free complete\n");
    return 0;
}

/*
 * Phase 2: Fragment the heap badly, then do large allocations.
 * This catches issues where freed small chunks corrupt metadata
 * that large-chunk allocation relies on.
 */
static int phase2_fragment_and_reuse(void) {
    printf("Phase 2: Fragment heap then large-alloc...\n");

    /* Allocate 10000 small chunks alternating exe/lib */
    #define FRAG_COUNT 10000
    static void *frags[FRAG_COUNT];
    int i;

    for (i = 0; i < FRAG_COUNT; i++) {
        size_t sz = 64 + (i % 256) * 16;  /* 64 to 4144 bytes */
        if (i % 2 == 0)
            frags[i] = malloc(sz);
        else
            frags[i] = lib_malloc(sz);
        if (!frags[i]) {
            fprintf(stderr, "  frag alloc failed at i=%d\n", i);
            return 1;
        }
        memset(frags[i], 0xDE, sz);
    }

    /* Free every other one to create fragmentation holes */
    for (i = 0; i < FRAG_COUNT; i += 2) {
        lib_free(frags[i]);   /* cross-free */
        frags[i] = NULL;
    }

    /* Now try large allocations through the fragmented heap */
    void *big1 = lib_malloc(8 * 1024 * 1024);  /* 8MB via lib */
    void *big2 = malloc(8 * 1024 * 1024);       /* 8MB via exe */
    if (!big1 || !big2) {
        fprintf(stderr, "  large alloc through fragmented heap failed\n");
        return 1;
    }
    memset(big1, 0xAA, 8 * 1024 * 1024);
    memset(big2, 0xBB, 8 * 1024 * 1024);
    track_addr(big1);
    track_addr(big2);

    /* Verify the remaining small chunks weren't corrupted */
    int corrupt = 0;
    for (i = 1; i < FRAG_COUNT; i += 2) {
        unsigned char *p = (unsigned char *)frags[i];
        if (p[0] != 0xDE) {
            corrupt++;
            if (corrupt <= 3)
                fprintf(stderr, "  frag[%d] corrupted: 0x%02X\n", i, p[0]);
        }
    }
    if (corrupt) {
        fprintf(stderr, "  %d small chunks corrupted after large allocs!\n", corrupt);
        return 1;
    }

    /* Clean up */
    free(big1);        /* cross-free */
    lib_free(big2);    /* cross-free */
    for (i = 1; i < FRAG_COUNT; i += 2) {
        free(frags[i]);  /* cross-free */
    }

    printf("  Fragmentation + large-alloc: OK\n");
    return 0;
}

/*
 * Phase 3: After all that heap abuse, verify GOT and function pointers
 * still work. This is the real test — if heap growth stomped on GOT
 * pages or corrupted rld structures, these calls will crash or
 * return garbage.
 */
static int phase3_verify_got_integrity(void) {
    printf("Phase 3: Verifying GOT/function pointer integrity...\n");
    int errors = 0;

    /* Call through GOT to library function */
    int r1 = lib_crosscheck(6);
    if (r1 != 36) {
        fprintf(stderr, "  lib_crosscheck(6)=%d, want 36 — GOT CORRUPTED?\n", r1);
        errors++;
    }

    /* Call through saved function pointer (stored before heap abuse) */
    if (saved_fn) {
        int r2 = saved_fn(6);
        if (r2 != 42) {
            fprintf(stderr, "  saved_fn(6)=%d, want 42 — FUNC PTR CORRUPTED?\n", r2);
            errors++;
        }
    } else {
        fprintf(stderr, "  saved_fn is NULL — pointer corrupted?\n");
        errors++;
    }

    /* Call lib_strdup — tests malloc + strlen + strcpy through GOT */
    char *s = lib_strdup("integrity_check");
    if (!s || strcmp(s, "integrity_check") != 0) {
        fprintf(stderr, "  lib_strdup failed after heap abuse — GOT CORRUPTED?\n");
        errors++;
    }
    if (s) free(s);

    /* Do one more malloc/free cycle to verify allocator is coherent */
    void *p = lib_malloc(1024 * 1024);
    if (p) {
        memset(p, 0xFF, 1024 * 1024);
        lib_free(p);
    } else {
        fprintf(stderr, "  post-abuse malloc failed\n");
        errors++;
    }

    if (errors == 0)
        printf("  GOT and function pointers intact\n");
    return errors;
}

int main(void) {
    int errors = 0;

    printf("D19: WebKit-scale cross-heap stress test\n");
    printf("  Testing: 200MB+ heap, fragmentation, cross-free, GOT integrity\n\n");

    /* Save a function pointer BEFORE heap abuse */
    saved_fn = my_multiply;

    /* Verify it works now */
    if (saved_fn(6) != 42) {
        fprintf(stderr, "D19: saved_fn broken before test even starts!\n");
        return 1;
    }

    errors += phase1_grow_heap();
    if (errors) {
        printf("D19 FAIL: phase 1 (heap growth) failed\n");
        return 1;
    }

    errors += phase2_fragment_and_reuse();
    if (errors) {
        printf("D19 FAIL: phase 2 (fragmentation) failed\n");
        return 1;
    }

    errors += phase3_verify_got_integrity();

    printf("\n  Final address range: 0x%08lX - 0x%08lX (span %luMB)\n",
           addr_min, addr_max, (addr_max - addr_min) / (1024*1024));

    if (errors == 0)
        printf("D19 PASS: cross-heap works at WebKit scale\n");
    else
        printf("D19 FAIL: %d errors\n", errors);
    return errors ? 1 : 0;
}
