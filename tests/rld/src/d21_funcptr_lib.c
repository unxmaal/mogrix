/*
 * D21: Function pointer callback test - library
 *
 * GType pattern: the library stores a function pointer registered by
 * the executable, then calls it later. On MIPS/IRIX this requires
 * correct $t9/$gp setup for cross-module function pointer calls.
 *
 * If $t9 (PIC call register) isn't set correctly when calling through
 * the stored pointer, $gp will be wrong and GOT-relative loads will
 * crash or return garbage.
 */

typedef int (*callback_t)(int);

static callback_t registered_cb = 0;

void register_callback(callback_t cb) {
    registered_cb = cb;
}

int invoke_callback(int x) {
    if (!registered_cb) return -1;
    return registered_cb(x);
}

/* Also test: store and invoke from same library (should always work) */
static int local_double(int x) { return x * 2; }

int test_local_callback(int x) {
    callback_t local = local_double;
    return local(x);
}
