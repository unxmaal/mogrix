/*
 * D21: Function pointer callback test - main
 *
 * Registers a function pointer from the exe into the library,
 * then asks the library to call it. Tests the GType/GObject
 * pattern where type registration stores vtable pointers that
 * get called later from different modules.
 */
#include <stdio.h>

extern void register_callback(int (*cb)(int));
extern int invoke_callback(int x);
extern int test_local_callback(int x);

static int my_triple(int x) {
    return x * 3;
}

int main(void) {
    int errors = 0;

    /* Test 1: local callback within library (baseline) */
    int local_result = test_local_callback(7);
    if (local_result != 14) {
        fprintf(stderr, "D21: local callback: got %d, want 14\n", local_result);
        errors++;
    }

    /* Test 2: cross-module callback (the GType pattern) */
    register_callback(my_triple);
    int cross_result = invoke_callback(14);
    if (cross_result != 42) {
        fprintf(stderr, "D21: cross callback: got %d, want 42\n", cross_result);
        errors++;
    }

    /* Test 3: call with different value to catch $gp corruption */
    int cross_result2 = invoke_callback(100);
    if (cross_result2 != 300) {
        fprintf(stderr, "D21: cross callback(100): got %d, want 300\n", cross_result2);
        errors++;
    }

    if (errors == 0) {
        printf("D21 PASS: function pointer callbacks work across modules\n");
    } else {
        printf("D21 FAIL: %d errors\n", errors);
    }
    return errors ? 1 : 0;
}
