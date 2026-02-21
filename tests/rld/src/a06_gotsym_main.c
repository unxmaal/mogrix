/*
 * A06: GOTSYM threshold test - main
 *
 * Verifies that all exported symbols from the library resolve correctly,
 * which proves GOTSYM partitioning is set up properly.
 */
#include <stdio.h>

extern int gotsym_func_a(void);
extern int gotsym_func_b(void);
extern int gotsym_func_sum(void);

int main(void) {
    int a = gotsym_func_a();
    int b = gotsym_func_b();
    int s = gotsym_func_sum();
    int errors = 0;

    if (a != 100)     { fprintf(stderr, "gotsym_func_a: got %d, want 100\n", a); errors++; }
    if (b != 200)     { fprintf(stderr, "gotsym_func_b: got %d, want 200\n", b); errors++; }
    if (s != 300)     { fprintf(stderr, "gotsym_func_sum: got %d, want 300\n", s); errors++; }

    if (errors == 0) {
        printf("A06 PASS: GOTSYM partitioning correct\n");
    } else {
        printf("A06 FAIL: %d errors\n", errors);
    }
    return errors ? 1 : 0;
}
