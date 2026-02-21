/*
 * D18: Constructor execution test - library A
 *
 * Tests that .ctors / DT_INIT constructors run properly in shared
 * libraries on IRIX. rld calls DT_INIT → _init → .ctors chain.
 */

static int ctor_a_ran = 0;

__attribute__((constructor))
static void init_a(void) {
    ctor_a_ran = 1;
}

int lib_a_ctor_ran(void) {
    return ctor_a_ran;
}
