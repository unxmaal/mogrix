/*
 * B10: Strict NEEDED test - main
 *
 * Links against libb only. libb NEEDs liba.
 * Calls needed_b_func() which calls needed_a_func().
 * Tests that transitive NEEDED resolution works.
 */
#include <stdio.h>

extern int needed_b_func(void);

int main(void) {
    int result = needed_b_func();
    if (result != 30) {
        fprintf(stderr, "B10 FAIL: needed_b_func=%d (want 30)\n", result);
        return 1;
    }
    printf("B10 PASS: transitive NEEDED works (10+20=%d)\n", result);
    return 0;
}
