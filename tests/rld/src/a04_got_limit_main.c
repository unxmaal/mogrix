/*
 * A04: GOT entry limit test - main
 *
 * Calls functions from the library with many GOT entries.
 * Verifies they all resolve correctly at runtime.
 */
#include <stdio.h>
#include <stdlib.h>

extern int got_fn_0(void);
extern int got_fn_10(void);
extern int got_fn_25(void);
extern int got_fn_49(void);
extern int got_total_count(void);

int main(void) {
    int errors = 0;

    if (got_fn_0() != 0)   { fprintf(stderr, "got_fn_0 wrong\n"); errors++; }
    if (got_fn_10() != 10)  { fprintf(stderr, "got_fn_10 wrong\n"); errors++; }
    if (got_fn_25() != 25)  { fprintf(stderr, "got_fn_25 wrong\n"); errors++; }
    if (got_fn_49() != 49)  { fprintf(stderr, "got_fn_49 wrong\n"); errors++; }
    if (got_total_count() != 50) { fprintf(stderr, "got_total_count wrong\n"); errors++; }

    if (errors == 0) {
        printf("A04 PASS: all GOT entries resolved correctly\n");
    } else {
        printf("A04 FAIL: %d errors\n", errors);
    }
    return errors ? 1 : 0;
}
