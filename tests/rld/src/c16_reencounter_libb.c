/*
 * C16: GOT re-encounter test - library B
 *
 * This library NEEDs libc16a.so. When dlopen'd, rld must load libb
 * and re-encounter the already-loaded liba. The test checks whether
 * liba's state (set by its constructor) survives the re-encounter.
 */

extern int liba_check_init(void);
extern int liba_check_value(void);
extern int liba_compute(int);

int libb_verify_liba(void) {
    /* Check that liba's constructor ran and state is intact */
    if (!liba_check_init()) return -1;
    if (liba_check_value() != 0xCAFE) return -2;
    if (liba_compute(1) != 0xCAFE + 1) return -3;
    return 1;  /* success */
}
