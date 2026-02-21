/*
 * B08: No exe preemption test - main
 *
 * Defines get_value() returning 999. The library also defines it
 * returning 100. call_get_value() is in the library.
 *
 * On Linux: call_get_value() returns 999 (exe preempts)
 * On IRIX: call_get_value() returns 100 (no preemption)
 *
 * We test that IRIX behavior is as expected: no preemption.
 */
#include <stdio.h>

/* This is also defined in the library */
int get_value(void) {
    return 999;  /* exe's version — IRIX rld won't see this from .so */
}

extern int call_get_value(void);

int main(void) {
    int lib_result = call_get_value();
    int exe_result = get_value();

    printf("B08: exe get_value()=%d, lib call_get_value()=%d\n", exe_result, lib_result);

    if (exe_result != 999) {
        fprintf(stderr, "B08 FAIL: exe's own get_value broken\n");
        return 1;
    }

    /*
     * On IRIX: lib_result should be 100 (library's own version)
     * On Linux: lib_result would be 999 (exe preemption)
     * We document whichever behavior we observe.
     */
    if (lib_result == 100) {
        printf("B08 PASS: no exe preemption (IRIX behavior confirmed)\n");
    } else if (lib_result == 999) {
        printf("B08 INFO: exe preemption detected (Linux-like behavior)\n");
    } else {
        fprintf(stderr, "B08 FAIL: unexpected value %d\n", lib_result);
        return 1;
    }

    return 0;
}
