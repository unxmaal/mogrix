/*
 * B09: _RLDN32_LIST preload test - main
 *
 * Links against libb09_base.so (preload_func returns 0).
 * When run with _RLDN32_LIST=libb09_override.so:DEFAULT,
 * preload_func should return 42.
 *
 * This tests the mechanism we use for libmogrix_compat.so
 * to override buggy IRIX libc functions.
 */
#include <stdio.h>

extern int preload_func(void);

int main(void) {
    int result = preload_func();

    printf("B09: preload_func() = %d\n", result);

    if (result == 42) {
        printf("B09 PASS: _RLDN32_LIST override works\n");
        return 0;
    } else if (result == 0) {
        /* No preload — either _RLDN32_LIST not set or not supported */
        fprintf(stderr, "B09 FAIL: preload not active (got base value 0)\n");
        return 1;
    } else {
        fprintf(stderr, "B09 FAIL: unexpected value %d\n", result);
        return 1;
    }
}
