/*
 * D22: bsearch(nmemb=0) test - main
 *
 * Tests bsearch with nmemb=0 from both exe and shared library.
 * On stock IRIX libc, the shared lib call crashes.
 * With libmogrix_compat.so preloaded, both should work.
 */
#include <stdio.h>
#include <stdlib.h>

static int cmp_int(const void *a, const void *b) {
    return *(const int *)a - *(const int *)b;
}

extern void *lib_bsearch_empty(const void *key, const void *base, int nmemb);
extern void *lib_bsearch_normal(const void *key, const void *base, int nmemb);

int main(void) {
    int errors = 0;
    int arr[] = {1, 3, 5, 7, 9};
    int key;

    /* Test 1: bsearch(nmemb=0) from exe — dlmalloc exe has its own bsearch? */
    key = 5;
    void *r1 = bsearch(&key, arr, 0, sizeof(int), cmp_int);
    if (r1 != NULL) {
        fprintf(stderr, "D22: exe bsearch(0) returned non-NULL\n");
        errors++;
    }

    /* Test 2: bsearch(nmemb=0) from shared library — THE critical test */
    void *r2 = lib_bsearch_empty(&key, arr, 0);
    if (r2 != NULL) {
        fprintf(stderr, "D22: lib bsearch(0) returned non-NULL\n");
        errors++;
    }

    /* Test 3: normal bsearch from lib (sanity) */
    key = 5;
    void *r3 = lib_bsearch_normal(&key, arr, 5);
    if (r3 == NULL || *(int *)r3 != 5) {
        fprintf(stderr, "D22: lib bsearch(5) failed\n");
        errors++;
    }

    /* Test 4: bsearch for missing key */
    key = 4;
    void *r4 = lib_bsearch_normal(&key, arr, 5);
    if (r4 != NULL) {
        fprintf(stderr, "D22: lib bsearch(4) should be NULL\n");
        errors++;
    }

    if (errors == 0)
        printf("D22 PASS: bsearch(nmemb=0) safe from both exe and lib\n");
    else
        printf("D22 FAIL: %d errors\n", errors);
    return errors ? 1 : 0;
}
