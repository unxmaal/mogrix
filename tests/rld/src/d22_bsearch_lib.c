/*
 * D22: bsearch(nmemb=0) test - library
 *
 * Calls bsearch from a shared library with nmemb=0.
 * IRIX libc's bsearch underflows nmemb-1 unsigned → huge value →
 * dereferences garbage → crash.
 *
 * Fix: libmogrix_compat.so preloaded via _RLDN32_LIST overrides bsearch.
 * This test verifies the override works for calls FROM shared libraries.
 */
#include <stdlib.h>

static int cmp_int(const void *a, const void *b) {
    return *(const int *)a - *(const int *)b;
}

void *lib_bsearch_empty(const void *key, const void *base, int nmemb) {
    /* Call bsearch with nmemb=0 — crashes on stock IRIX libc */
    return bsearch(key, base, (size_t)nmemb, sizeof(int), cmp_int);
}

void *lib_bsearch_normal(const void *key, const void *base, int nmemb) {
    return bsearch(key, base, (size_t)nmemb, sizeof(int), cmp_int);
}
