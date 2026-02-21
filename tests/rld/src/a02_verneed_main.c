/*
 * A02: No VERNEED test - main
 */
#include <stdio.h>
extern int verneed_func(int x);

int main(void) {
    if (verneed_func(21) != 42) {
        fprintf(stderr, "A02 FAIL\n");
        return 1;
    }
    printf("A02 PASS: no VERNEED tags, .so loads fine\n");
    return 0;
}
