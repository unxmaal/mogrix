/*
 * C13: pthread via NEEDED test - main
 *
 * Calls into a library that uses pthreads via NEEDED.
 * On IRIX, dlopen'ing libpthread.so can crash; it must be NEEDED.
 */
#include <stdio.h>

extern int pthread_increment(void);
extern int pthread_get_counter(void);

int main(void) {
    int errors = 0;

    int v1 = pthread_increment();
    int v2 = pthread_increment();
    int v3 = pthread_increment();

    if (v1 != 1) { fprintf(stderr, "C13: inc1=%d (want 1)\n", v1); errors++; }
    if (v2 != 2) { fprintf(stderr, "C13: inc2=%d (want 2)\n", v2); errors++; }
    if (v3 != 3) { fprintf(stderr, "C13: inc3=%d (want 3)\n", v3); errors++; }

    int final = pthread_get_counter();
    if (final != 3) { fprintf(stderr, "C13: counter=%d (want 3)\n", final); errors++; }

    if (errors == 0)
        printf("C13 PASS: pthread via NEEDED works\n");
    else
        printf("C13 FAIL: %d errors\n", errors);
    return errors ? 1 : 0;
}
