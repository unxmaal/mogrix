/*
 * B11: DT_BIND_NOW test - main
 *
 * Linked with -z now (DT_BIND_NOW). If rld honors this on MIPS,
 * all GOT entries are resolved at load time rather than lazily.
 * This is exploratory — we want to know if this flag works on IRIX.
 */
#include <stdio.h>

extern int bindnow_func(int x);
extern int bindnow_func2(int x);

int main(void) {
    int errors = 0;

    if (bindnow_func(31) != 42) { fprintf(stderr, "B11: func1 failed\n"); errors++; }
    if (bindnow_func2(21) != 42) { fprintf(stderr, "B11: func2 failed\n"); errors++; }

    if (errors == 0)
        printf("B11 PASS: DT_BIND_NOW binary works\n");
    else
        printf("B11 FAIL: %d errors\n", errors);
    return errors ? 1 : 0;
}
