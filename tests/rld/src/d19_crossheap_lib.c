/*
 * D19: dlmalloc cross-heap test - library
 *
 * Receives a pointer malloc'd in the exe and frees it.
 * If exe uses dlmalloc but .so uses IRIX libc malloc, this
 * crosses heap boundaries and crashes with abort().
 *
 * Our toolchain puts dlmalloc in executables only. Libraries use
 * undefined malloc/free which resolve to the exe's dlmalloc via GOT.
 * This test verifies that works.
 */
#include <stdlib.h>
#include <string.h>

void lib_free(void *p) {
    free(p);
}

void *lib_malloc(int size) {
    return malloc(size);
}

/* Allocate in lib, return to caller to free */
char *lib_strdup(const char *s) {
    char *p = malloc(strlen(s) + 1);
    if (p) strcpy(p, s);
    return p;
}

/* GOT integrity check — called after heap abuse to verify
   cross-library calls still work (tests $gp, GOT entries) */
int lib_crosscheck(int x) {
    /* Do some work that requires GOT (strlen is in libc) */
    char buf[32];
    int i;
    for (i = 0; i < x; i++) buf[i] = 'A';
    buf[x] = '\0';
    return (int)strlen(buf) * x;  /* 6 * 6 = 36 */
}
