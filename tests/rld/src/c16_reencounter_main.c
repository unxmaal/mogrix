/*
 * C16: GOT re-encounter test - main
 *
 * 1. dlopen liba (its constructor runs, sets init_value = 0xCAFE)
 * 2. dlopen libb (which NEEDs liba — rld re-encounters liba)
 * 3. Through libb, verify liba's constructor state is still intact
 *
 * If rld re-zeroes liba's GOT on re-encounter, step 3 fails.
 * This is the suspected MiniBrowser crash pattern.
 */

#include <stdio.h>
#include <dlfcn.h>

int main(void) {
    /* Step 1: Load liba */
    void *ha = dlopen("libc16a.so", RTLD_LAZY);
    if (!ha) {
        fprintf(stderr, "C16 FAIL: dlopen(libc16a.so): %s\n", dlerror());
        return 1;
    }

    /* Verify liba's constructor ran */
    int (*check_init)(void) = (int (*)(void))dlsym(ha, "liba_check_init");
    if (!check_init || !check_init()) {
        fprintf(stderr, "C16 FAIL: liba constructor didn't run after dlopen\n");
        return 1;
    }

    int (*check_value)(void) = (int (*)(void))dlsym(ha, "liba_check_value");
    if (!check_value || check_value() != 0xCAFE) {
        fprintf(stderr, "C16 FAIL: liba init_value wrong before re-encounter: 0x%X\n",
                check_value ? check_value() : -1);
        return 1;
    }

    printf("C16: liba loaded, constructor OK (init_value=0x%X)\n", check_value());

    /* Step 2: Load libb (which NEEDs liba — triggers re-encounter) */
    void *hb = dlopen("libc16b.so", RTLD_LAZY);
    if (!hb) {
        fprintf(stderr, "C16 FAIL: dlopen(libc16b.so): %s\n", dlerror());
        return 1;
    }

    printf("C16: libb loaded (re-encounter of liba happened)\n");

    /* Step 3: Check if liba's state survived the re-encounter */
    /* First check directly */
    int val_after = check_value();
    if (val_after != 0xCAFE) {
        fprintf(stderr, "C16 FAIL: liba init_value corrupted after re-encounter: 0x%X (want 0xCAFE)\n",
                val_after);
        return 1;
    }

    /* Then check through libb (cross-library call path) */
    int (*verify)(void) = (int (*)(void))dlsym(hb, "libb_verify_liba");
    if (!verify) {
        fprintf(stderr, "C16 FAIL: dlsym(libb_verify_liba): %s\n", dlerror());
        return 1;
    }

    int result = verify();
    if (result != 1) {
        fprintf(stderr, "C16 FAIL: libb_verify_liba returned %d (want 1)\n", result);
        return 1;
    }

    dlclose(hb);
    dlclose(ha);

    printf("C16 PASS: liba state survived re-encounter via libb\n");
    return 0;
}
