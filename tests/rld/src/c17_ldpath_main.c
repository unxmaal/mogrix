/*
 * C17: LD_LIBRARYN32_PATH test - main
 *
 * This binary links against libc17_path.so. At runtime, rld must
 * find it via LD_LIBRARYN32_PATH. Also implicitly tests that
 * system libs (/usr/lib32) are reachable.
 */
#include <stdio.h>
#include <string.h>

extern int ldpath_func(void);

int main(void) {
    int errors = 0;

    /* Test our library loads via LD_LIBRARYN32_PATH */
    if (ldpath_func() != 17) {
        fprintf(stderr, "C17 FAIL: ldpath_func()=%d\n", ldpath_func());
        errors++;
    }

    /* Test a libc function to prove system libs are found */
    if (strlen("test") != 4) {
        fprintf(stderr, "C17 FAIL: strlen broken\n");
        errors++;
    }

    if (errors == 0)
        printf("C17 PASS: LD_LIBRARYN32_PATH resolves correctly\n");
    else
        printf("C17 FAIL: %d errors\n", errors);
    return errors ? 1 : 0;
}
