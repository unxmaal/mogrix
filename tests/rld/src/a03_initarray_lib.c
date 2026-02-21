/*
 * A03: No .init_array test - library
 *
 * Has a constructor. verify_elf checks that the toolchain emits
 * .ctors (which IRIX rld processes) and NOT .init_array (which it ignores).
 */
static int a03_init_ran = 0;

__attribute__((constructor))
static void a03_init(void) {
    a03_init_ran = 1;
}

int a03_check(void) {
    return a03_init_ran;
}
