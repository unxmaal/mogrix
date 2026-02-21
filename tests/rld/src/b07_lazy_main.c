/*
 * B07: Lazy binding test - main
 */
#include <stdio.h>

extern int lazy_add(int a, int b);
extern int lazy_mul(int a, int b);
extern int lazy_len(const char *s);

int main(void) {
    int errors = 0;

    if (lazy_add(3, 4) != 7) { fprintf(stderr, "B07: add failed\n"); errors++; }
    if (lazy_mul(6, 7) != 42) { fprintf(stderr, "B07: mul failed\n"); errors++; }
    if (lazy_len("hello") != 5) { fprintf(stderr, "B07: len failed\n"); errors++; }

    if (errors == 0)
        printf("B07 PASS: lazy binding works\n");
    else
        printf("B07 FAIL: %d errors\n", errors);
    return errors ? 1 : 0;
}
