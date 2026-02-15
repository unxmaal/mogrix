/*
 * dlmalloc-test.c — Focused test suite for dlmalloc on IRIX
 *
 * Cross-compile:
 *   irix-cc -O2 -o dlmalloc-test dlmalloc-test.c -lpthread
 *
 * Tests: page size, /dev/zero fd, basic ops, calloc zeroing, realloc
 * preservation, memalign, large allocs, stress, fork safety, threads.
 *
 * Thread safety is the highest-risk area: USE_LOCKS=0 means no mutex
 * protection. If threads corrupt the heap, we need to enable USE_LOCKS=1.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <errno.h>
#include <pthread.h>

/* memalign may come from malloc.h or stdlib.h depending on platform */
extern void *memalign(size_t, size_t);
extern int posix_memalign(void **, size_t, size_t);

static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

/*
 * Volatile sink: writing a pointer here prevents the compiler from
 * optimizing away the corresponding malloc/calloc/realloc call.
 * Without this, -O2 can eliminate malloc+free pairs entirely.
 */
static volatile void *alloc_sink;

static void test_ok(const char *name) {
    tests_run++;
    tests_passed++;
    printf("  %-55s PASS\n", name);
}

static void test_fail(const char *name, const char *msg) {
    tests_run++;
    tests_failed++;
    printf("  %-55s FAIL: %s\n", name, msg);
}

/* ------------------------------------------------------------------ */
/*  1. Page size                                                       */
/* ------------------------------------------------------------------ */

static void test_pagesize(void) {
    long ps = sysconf(_SC_PAGESIZE);
    if (ps == 16384)
        test_ok("sysconf page size == 16384");
    else {
        char buf[64];
        sprintf(buf, "expected 16384, got %ld", ps);
        test_fail("sysconf page size == 16384", buf);
    }
}

/* ------------------------------------------------------------------ */
/*  2. /dev/zero fd sanity                                             */
/* ------------------------------------------------------------------ */

static void test_devzero_fd(void) {
    /* Force dlmalloc to open /dev/zero by doing an allocation.
       alloc_sink prevents -O2 from eliminating the malloc+free pair. */
    void *p = malloc(1024);
    alloc_sink = p;
    if (!p) {
        test_fail("/dev/zero fd >= 128", "initial malloc returned NULL");
        return;
    }
    memset(p, 0x42, 1024);
    free(p);

    /* Scan fds 128-255 for the cached /dev/zero fd */
    int found_fd = -1;
    int i;
    for (i = 128; i < 256; i++) {
        if (fcntl(i, F_GETFD) >= 0) {
            found_fd = i;
            break;
        }
    }

    if (found_fd >= 0)
        test_ok("/dev/zero fd >= 128");
    else
        test_fail("/dev/zero fd >= 128", "no open fd in 128-255 range");

    if (found_fd >= 0) {
        int flags = fcntl(found_fd, F_GETFD);
        if (flags & FD_CLOEXEC)
            test_ok("/dev/zero fd has FD_CLOEXEC");
        else
            test_fail("/dev/zero fd has FD_CLOEXEC", "flag not set");
    } else {
        test_fail("/dev/zero fd has FD_CLOEXEC", "no fd to check");
    }

    /* Verify the fd is still usable for mmap */
    if (found_fd >= 0) {
        void *m = mmap(0, 16384, PROT_READ|PROT_WRITE, MAP_PRIVATE,
                        found_fd, 0);
        if (m != MAP_FAILED) {
            /* Write and read back */
            ((char *)m)[0] = 0x42;
            if (((char *)m)[0] == 0x42)
                test_ok("/dev/zero fd usable for mmap");
            else
                test_fail("/dev/zero fd usable for mmap", "read-back mismatch");
            munmap(m, 16384);
        } else {
            char buf[64];
            sprintf(buf, "mmap failed: errno %d", errno);
            test_fail("/dev/zero fd usable for mmap", buf);
        }
    }

    /* Simulate fd cleanup: close low fds and verify dlmalloc still works */
    test_ok("(info) testing post-fd-cleanup malloc");
    /* Close a range of low fds that a shell might close.
       Skip 0/1/2 (stdin/stdout/stderr) so we can still print results.
       Close fds 3-20 like tcsh does. */
    for (i = 3; i <= 20; i++) {
        close(i);
    }
    p = malloc(4096);
    alloc_sink = p;
    if (p) {
        memset(p, 0xAA, 4096);
        test_ok("malloc works after closing fds 3-20");
        free(p);
    } else {
        test_fail("malloc works after closing fds 3-20",
                  "returned NULL (fd caching bug?)");
    }
}

/* ------------------------------------------------------------------ */
/*  3. Basic operations                                                */
/* ------------------------------------------------------------------ */

static void test_basic(void) {
    void *p;

    p = malloc(0);
    alloc_sink = p;
    if (p != NULL) test_ok("malloc(0) returns non-NULL");
    else test_fail("malloc(0) returns non-NULL", "returned NULL");
    free(p);

    p = malloc(1);
    alloc_sink = p;
    if (p != NULL) { ((char *)p)[0] = 1; test_ok("malloc(1) returns non-NULL"); }
    else test_fail("malloc(1) returns non-NULL", "returned NULL");
    free(p);

    p = malloc(1000000);
    alloc_sink = p;
    if (p != NULL) { memset(p, 0xBB, 1000000); test_ok("malloc(1000000) returns non-NULL"); }
    else test_fail("malloc(1000000) returns non-NULL", "returned NULL");
    free(p);

    p = realloc(NULL, 100);
    alloc_sink = p;
    if (p != NULL) { memset(p, 0xCC, 100); test_ok("realloc(NULL, 100) works like malloc"); }
    else test_fail("realloc(NULL, 100) works like malloc", "returned NULL");
    free(p);

    /* realloc(p, 0): dlmalloc frees and returns NULL or minimal chunk */
    p = malloc(100);
    alloc_sink = p;
    if (p) {
        memset(p, 0xDD, 100);
        void *q = realloc(p, 0);
        alloc_sink = q;
        test_ok("realloc(p, 0) doesn't crash");
        if (q) free(q);
    } else {
        test_fail("realloc(p, 0) doesn't crash", "initial malloc failed");
    }

    /* Double free detection: dlmalloc should detect this.
       We don't test it because it may abort. Just note the behavior. */
}

/* ------------------------------------------------------------------ */
/*  4. calloc zeroing                                                  */
/* ------------------------------------------------------------------ */

static void test_calloc(void) {
    int ok, i;

    int *p = (int *)calloc(256, sizeof(int));
    alloc_sink = p;
    ok = 1;
    if (p) {
        for (i = 0; i < 256; i++) {
            if (p[i] != 0) { ok = 0; break; }
        }
        if (ok) test_ok("calloc zeroes memory (1KB)");
        else test_fail("calloc zeroes memory (1KB)", "non-zero byte found");
        free(p);
    } else {
        test_fail("calloc zeroes memory (1KB)", "calloc returned NULL");
    }

    /* Large calloc — crosses mmap threshold (256K default) */
    char *q = (char *)calloc(1, 1024 * 1024);
    alloc_sink = q;
    ok = 1;
    if (q) {
        for (i = 0; i < 1024 * 1024; i++) {
            if (q[i] != 0) { ok = 0; break; }
        }
        if (ok) test_ok("calloc zeroes memory (1MB, crosses mmap threshold)");
        else test_fail("calloc zeroes memory (1MB)", "non-zero byte found");
        free(q);
    } else {
        test_fail("calloc zeroes memory (1MB)", "calloc returned NULL");
    }

    /* calloc overflow check: huge n * size should fail gracefully */
    volatile size_t big = 0x7FFFFFFF;  /* prevent constant folding */
    q = (char *)calloc(big, big);
    alloc_sink = q;
    if (q == NULL)
        test_ok("calloc(huge, huge) returns NULL (overflow)");
    else {
        test_fail("calloc(huge, huge) returns NULL", "didn't detect overflow");
        free(q);
    }
}

/* ------------------------------------------------------------------ */
/*  5. realloc content preservation                                    */
/* ------------------------------------------------------------------ */

static void test_realloc_preserve(void) {
    int ok, i;

    /* Grow */
    char *p = (char *)malloc(100);
    if (!p) { test_fail("realloc grow preserves content", "malloc failed"); return; }
    memset(p, 0xAB, 100);
    p = (char *)realloc(p, 4096);
    ok = 1;
    if (p) {
        for (i = 0; i < 100; i++) {
            if ((unsigned char)p[i] != 0xAB) { ok = 0; break; }
        }
        if (ok) test_ok("realloc grow preserves content (100 -> 4096)");
        else test_fail("realloc grow preserves content", "data changed");
        free(p);
    } else {
        test_fail("realloc grow preserves content", "realloc returned NULL");
    }

    /* Shrink */
    p = (char *)malloc(4096);
    if (!p) { test_fail("realloc shrink preserves content", "malloc failed"); return; }
    memset(p, 0xCD, 4096);
    p = (char *)realloc(p, 100);
    ok = 1;
    if (p) {
        for (i = 0; i < 100; i++) {
            if ((unsigned char)p[i] != 0xCD) { ok = 0; break; }
        }
        if (ok) test_ok("realloc shrink preserves content (4096 -> 100)");
        else test_fail("realloc shrink preserves content", "data changed");
        free(p);
    } else {
        test_fail("realloc shrink preserves content", "realloc returned NULL");
    }

    /* Grow across mmap threshold boundary */
    p = (char *)malloc(200 * 1024);  /* 200KB, below 256K threshold */
    if (!p) { test_fail("realloc across mmap threshold", "malloc failed"); return; }
    memset(p, 0xEF, 200 * 1024);
    p = (char *)realloc(p, 512 * 1024);  /* 512KB, above threshold */
    ok = 1;
    if (p) {
        for (i = 0; i < 200 * 1024; i++) {
            if ((unsigned char)p[i] != 0xEF) { ok = 0; break; }
        }
        if (ok) test_ok("realloc across mmap threshold preserves content");
        else test_fail("realloc across mmap threshold", "data changed");
        free(p);
    } else {
        test_fail("realloc across mmap threshold", "realloc returned NULL");
    }
}

/* ------------------------------------------------------------------ */
/*  6. memalign / posix_memalign                                       */
/* ------------------------------------------------------------------ */

static void test_memalign(void) {
    void *p;
    char buf[80];

    p = memalign(16, 1024);
    if (p && ((unsigned long)p % 16) == 0)
        test_ok("memalign(16, 1024) aligned");
    else if (!p)
        test_fail("memalign(16, 1024) aligned", "returned NULL");
    else {
        sprintf(buf, "addr %p not 16-aligned", p);
        test_fail("memalign(16, 1024) aligned", buf);
    }
    free(p);

    p = memalign(4096, 4096);
    if (p && ((unsigned long)p % 4096) == 0)
        test_ok("memalign(4096, 4096) aligned");
    else if (!p)
        test_fail("memalign(4096, 4096) aligned", "returned NULL");
    else {
        sprintf(buf, "addr %p not 4096-aligned", p);
        test_fail("memalign(4096, 4096) aligned", buf);
    }
    free(p);

    p = memalign(16384, 16384);
    if (p && ((unsigned long)p % 16384) == 0)
        test_ok("memalign(16384, 16384) page-aligned");
    else if (!p)
        test_fail("memalign(16384, 16384) page-aligned", "returned NULL");
    else {
        sprintf(buf, "addr %p not page-aligned", p);
        test_fail("memalign(16384, 16384) page-aligned", buf);
    }
    free(p);

    /* posix_memalign */
    int rc = posix_memalign(&p, 64, 512);
    if (rc == 0 && p && ((unsigned long)p % 64) == 0)
        test_ok("posix_memalign(64, 512) aligned");
    else if (rc != 0) {
        sprintf(buf, "returned error %d", rc);
        test_fail("posix_memalign(64, 512) aligned", buf);
    } else {
        sprintf(buf, "addr %p not 64-aligned", p);
        test_fail("posix_memalign(64, 512) aligned", buf);
    }
    free(p);
}

/* ------------------------------------------------------------------ */
/*  7. Large allocations                                               */
/* ------------------------------------------------------------------ */

static void test_large(void) {
    void *p;
    char buf[80];

    struct { size_t size; const char *label; int must_pass; } cases[] = {
        { 1 * 1024 * 1024,   "malloc 1MB",   1 },
        { 10 * 1024 * 1024,  "malloc 10MB",  1 },
        { 100 * 1024 * 1024, "malloc 100MB", 1 },
        { 500 * 1024 * 1024, "malloc 500MB", 0 },  /* may fail on n32 */
    };
    int i;
    int ncases = sizeof(cases) / sizeof(cases[0]);

    for (i = 0; i < ncases; i++) {
        p = malloc(cases[i].size);
        alloc_sink = p;
        if (p) {
            /* Touch first and last page to verify mapping */
            ((char *)p)[0] = 0xFF;
            ((char *)p)[cases[i].size - 1] = 0xFF;
            test_ok(cases[i].label);
            free(p);
        } else if (cases[i].must_pass) {
            test_fail(cases[i].label, "returned NULL");
        } else {
            sprintf(buf, "%s (NULL, acceptable on n32)", cases[i].label);
            test_ok(buf);
        }
    }

    /* Allocation + free + re-allocation: verify memory is returned to OS */
    p = malloc(50 * 1024 * 1024);
    if (p) {
        memset(p, 0xBB, 50 * 1024 * 1024);
        free(p);
        /* Allocate again — should reuse or get new mapping */
        p = malloc(50 * 1024 * 1024);
        if (p) {
            test_ok("50MB alloc-free-alloc cycle");
            free(p);
        } else {
            test_fail("50MB alloc-free-alloc cycle", "second alloc failed");
        }
    }
}

/* ------------------------------------------------------------------ */
/*  8. Stress: random alloc/free                                       */
/* ------------------------------------------------------------------ */

#define STRESS_PTRS 1000
#define STRESS_ITERS 50000

static void test_stress(void) {
    void *ptrs[STRESS_PTRS];
    size_t sizes[STRESS_PTRS];
    unsigned seed = 12345;
    int ok = 1;
    int i;

    memset(ptrs, 0, sizeof(ptrs));
    memset(sizes, 0, sizeof(sizes));

    for (i = 0; i < STRESS_ITERS; i++) {
        seed = seed * 1103515245 + 12345;
        int idx = (seed >> 16) % STRESS_PTRS;
        if (ptrs[idx]) {
            /* Verify tag byte before freeing */
            unsigned char tag = (unsigned char)(idx & 0xFF);
            if (((unsigned char *)ptrs[idx])[0] != tag) {
                ok = 0;
                printf("    corruption at iter %d, idx %d: expected 0x%02X got 0x%02X\n",
                       i, idx, tag, ((unsigned char *)ptrs[idx])[0]);
                break;
            }
            free(ptrs[idx]);
            ptrs[idx] = NULL;
        } else {
            size_t sz = ((seed >> 8) % 65536) + 1;
            ptrs[idx] = malloc(sz);
            if (!ptrs[idx]) {
                ok = 0;
                printf("    OOM at iter %d, size %lu\n", i, (unsigned long)sz);
                break;
            }
            sizes[idx] = sz;
            /* Tag first byte for corruption detection */
            ((unsigned char *)ptrs[idx])[0] = (unsigned char)(idx & 0xFF);
        }
    }

    for (i = 0; i < STRESS_PTRS; i++) {
        if (ptrs[i]) free(ptrs[i]);
    }

    if (ok) test_ok("50000 random alloc/free cycles (sizes 1-64KB)");
    else test_fail("50000 random alloc/free cycles", "see above");
}

/* ------------------------------------------------------------------ */
/*  9. Fork safety                                                     */
/* ------------------------------------------------------------------ */

static void test_fork(void) {
    int i;
    /* Allocate in parent */
    char *parent_buf = (char *)malloc(4096);
    if (!parent_buf) {
        test_fail("fork + malloc in child", "parent malloc failed");
        return;
    }
    memset(parent_buf, 0xAA, 4096);

    /* Also do several allocations to exercise internal bookkeeping */
    void *extra[10];
    for (i = 0; i < 10; i++) {
        extra[i] = malloc(1024 * (i + 1));
        if (extra[i]) memset(extra[i], 0x55, 1024 * (i + 1));
    }

    pid_t pid = fork();
    if (pid < 0) {
        test_fail("fork + malloc in child", "fork() failed");
        free(parent_buf);
        for (i = 0; i < 10; i++) free(extra[i]);
        return;
    }

    if (pid == 0) {
        /* Child process */
        int ok = 1;

        /* Verify parent buffer is intact (COW) */
        for (i = 0; i < 4096; i++) {
            if ((unsigned char)parent_buf[i] != 0xAA) { ok = 0; break; }
        }
        if (!ok) _exit(2);

        /* Allocate in child */
        char *child_buf = (char *)malloc(8192);
        if (!child_buf) _exit(1);
        memset(child_buf, 0xBB, 8192);

        /* Free parent's allocations in child (should work on COW copy) */
        free(parent_buf);
        for (i = 0; i < 10; i++) free(extra[i]);

        /* More allocations */
        void *child_extra[20];
        for (i = 0; i < 20; i++) {
            child_extra[i] = malloc(512 * (i + 1));
            if (!child_extra[i]) _exit(3);
            memset(child_extra[i], 0xCC, 512 * (i + 1));
        }
        for (i = 0; i < 20; i++) free(child_extra[i]);

        free(child_buf);
        _exit(0);
    }

    /* Parent: wait for child */
    int status;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status)) {
        switch (WEXITSTATUS(status)) {
        case 0:  test_ok("fork + malloc in child"); break;
        case 1:  test_fail("fork + malloc in child", "child malloc failed"); break;
        case 2:  test_fail("fork + malloc in child", "parent buffer corrupted in child"); break;
        case 3:  test_fail("fork + malloc in child", "child extra alloc failed"); break;
        default: test_fail("fork + malloc in child", "child unknown exit"); break;
        }
    } else if (WIFSIGNALED(status)) {
        char buf[64];
        sprintf(buf, "child killed by signal %d", WTERMSIG(status));
        test_fail("fork + malloc in child", buf);
    } else {
        test_fail("fork + malloc in child", "child abnormal exit");
    }

    free(parent_buf);
    for (i = 0; i < 10; i++) free(extra[i]);
}

/* ------------------------------------------------------------------ */
/* 10. Thread safety                                                   */
/* ------------------------------------------------------------------ */

#define THREAD_ITERS  100000
#define NUM_THREADS   4
#define THREAD_PTRS   100

static volatile int thread_error = 0;

static void *thread_worker(void *arg) {
    int id = *(int *)arg;
    unsigned seed = (unsigned)id * 7919 + 42;
    void *ptrs[THREAD_PTRS];
    int i;

    memset(ptrs, 0, sizeof(ptrs));

    for (i = 0; i < THREAD_ITERS; i++) {
        seed = seed * 1103515245 + 12345;
        int idx = (seed >> 16) % THREAD_PTRS;

        if (ptrs[idx]) {
            /* Verify tag byte before freeing */
            unsigned char expected = (unsigned char)(id & 0xFF);
            unsigned char actual = ((unsigned char *)ptrs[idx])[0];
            if (actual != expected) {
                thread_error = 1;
                /* Don't free — heap may be corrupt */
                ptrs[idx] = NULL;
                return (void *)(long)1;
            }
            free(ptrs[idx]);
            ptrs[idx] = NULL;
        } else {
            size_t sz = ((seed >> 8) % 4096) + 16;
            ptrs[idx] = malloc(sz);
            if (!ptrs[idx]) {
                thread_error = 2;
                return (void *)(long)2;
            }
            /* Fill first 16 bytes with tag for corruption detection */
            memset(ptrs[idx], (char)(id & 0xFF), 16);
        }
    }

    for (i = 0; i < THREAD_PTRS; i++) {
        if (ptrs[i]) free(ptrs[i]);
    }
    return NULL;
}

static void test_threads(void) {
    pthread_t threads[NUM_THREADS];
    int ids[NUM_THREADS];
    int ok = 1;
    int i;

    printf("  (USE_LOCKS=1, USE_SPIN_LOCKS=1 — should be thread-safe)\n");

    for (i = 0; i < NUM_THREADS; i++) {
        ids[i] = i + 1;  /* Avoid tag byte 0 */
        if (pthread_create(&threads[i], NULL, thread_worker, &ids[i]) != 0) {
            char buf[64];
            sprintf(buf, "pthread_create failed for thread %d (errno %d)", i, errno);
            test_fail("4 threads x 100k malloc/free", buf);
            /* Join any already-created threads */
            int j;
            for (j = 0; j < i; j++) pthread_join(threads[j], NULL);
            return;
        }
    }

    for (i = 0; i < NUM_THREADS; i++) {
        void *ret;
        pthread_join(threads[i], &ret);
        long rv = (long)ret;
        if (rv == 1) {
            printf("    thread %d: CORRUPTION detected\n", ids[i]);
            ok = 0;
        } else if (rv == 2) {
            printf("    thread %d: OOM\n", ids[i]);
            ok = 0;
        }
    }

    if (ok)
        test_ok("4 threads x 100k malloc/free (no corruption)");
    else
        test_fail("4 threads x 100k malloc/free",
                  "heap corruption or OOM (need USE_LOCKS=1?)");
}

/* ------------------------------------------------------------------ */
/* main                                                                */
/* ------------------------------------------------------------------ */

int main(void) {
    printf("dlmalloc test suite for IRIX\n");
    printf("============================\n");
    printf("Config: HAVE_MORECORE=0, HAVE_MMAP=1, USE_LOCKS=1\n");
    printf("        USE_SPIN_LOCKS=1, malloc_getpagesize=16384\n");
    printf("        MMAP_CLEARS=1, /dev/zero mmap (no MAP_ANONYMOUS)\n\n");

    printf("[1. Page size]\n");
    test_pagesize();

    printf("\n[2. /dev/zero fd]\n");
    test_devzero_fd();

    printf("\n[3. Basic operations]\n");
    test_basic();

    printf("\n[4. calloc zeroing]\n");
    test_calloc();

    printf("\n[5. realloc content preservation]\n");
    test_realloc_preserve();

    printf("\n[6. memalign / posix_memalign]\n");
    test_memalign();

    printf("\n[7. Large allocations]\n");
    test_large();

    printf("\n[8. Stress: random alloc/free]\n");
    test_stress();

    printf("\n[9. Fork safety]\n");
    test_fork();

    printf("\n[10. Thread safety]\n");
    test_threads();

    printf("\n============================\n");
    printf("Results: %d/%d passed, %d failed\n",
           tests_passed, tests_run, tests_failed);

    if (tests_failed > 0) {
        printf("\nACTION NEEDED: See failed tests above.\n");
        return 1;
    }
    printf("\nAll tests passed.\n");
    return 0;
}
