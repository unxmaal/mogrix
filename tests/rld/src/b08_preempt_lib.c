/*
 * B08: No exe preemption test - library
 *
 * This library calls get_value(). On Linux, if the exe defines get_value(),
 * the exe's version would be called (symbol preemption). On IRIX, rld does
 * NOT preempt — the .so resolves symbols from its own NEEDED chain only.
 *
 * We define get_value() in this library as a fallback. The main exe also
 * defines it. The test verifies which version gets called.
 */

int get_value(void) {
    return 100;  /* library's version */
}

int call_get_value(void) {
    return get_value();  /* which get_value() does rld resolve? */
}
