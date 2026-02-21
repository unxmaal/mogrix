/*
 * C14: Circular dependency test - main
 *
 * Loads both libraries, calls functions from each.
 * Tests rld's handling of mutual dependencies.
 */
#include <stdio.h>

extern int libx_get(void);
extern int libx_set(int v);
extern int liby_get(void);
extern int liby_set(int v);

int main(void) {
    int errors = 0;

    if (libx_get() != 10) { fprintf(stderr, "C14: libx initial=%d\n", libx_get()); errors++; }
    if (liby_get() != 20) { fprintf(stderr, "C14: liby initial=%d\n", liby_get()); errors++; }

    libx_set(30);
    liby_set(40);

    if (libx_get() != 30) { fprintf(stderr, "C14: libx after set=%d\n", libx_get()); errors++; }
    if (liby_get() != 40) { fprintf(stderr, "C14: liby after set=%d\n", liby_get()); errors++; }

    if (errors == 0)
        printf("C14 PASS: both libraries work together\n");
    else
        printf("C14 FAIL: %d errors\n", errors);
    return errors ? 1 : 0;
}
