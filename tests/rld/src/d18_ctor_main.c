/*
 * D18: Constructor execution test - main
 *
 * Verifies that constructors in both shared libraries ran.
 * On IRIX, this depends on .ctors + DT_INIT (not .init_array).
 */
#include <stdio.h>

extern int lib_a_ctor_ran(void);
extern int lib_b_ctor_ran(void);

int main(void) {
    int a = lib_a_ctor_ran();
    int b = lib_b_ctor_ran();
    int errors = 0;

    if (!a) { fprintf(stderr, "D18: lib_a constructor did NOT run\n"); errors++; }
    if (!b) { fprintf(stderr, "D18: lib_b constructor did NOT run\n"); errors++; }

    if (errors == 0) {
        printf("D18 PASS: both constructors ran (a=%d, b=%d)\n", a, b);
    } else {
        printf("D18 FAIL: %d constructors didn't run\n", errors);
    }
    return errors ? 1 : 0;
}
