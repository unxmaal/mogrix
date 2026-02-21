/*
 * A03: No .init_array test - main
 */
#include <stdio.h>
extern int a03_check(void);

int main(void) {
    if (!a03_check()) {
        fprintf(stderr, "A03 FAIL: constructor didn't run (likely using .init_array)\n");
        return 1;
    }
    printf("A03 PASS: constructor ran via .ctors/DT_INIT\n");
    return 0;
}
