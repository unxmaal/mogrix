/*
 * D18: Constructor execution test - library B
 *
 * Second library with its own constructor. Tests that multiple
 * libraries' constructors all run.
 */

static int ctor_b_ran = 0;
static int ctor_b_order = 0;
static int global_order_counter = 0;

__attribute__((constructor))
static void init_b(void) {
    ctor_b_ran = 1;
    ctor_b_order = ++global_order_counter;
}

int lib_b_ctor_ran(void) {
    return ctor_b_ran;
}

int lib_b_ctor_order(void) {
    return ctor_b_order;
}
