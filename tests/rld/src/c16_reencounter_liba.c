/*
 * C16: GOT re-encounter test - library A
 *
 * This library has a constructor that sets a global flag.
 * When libb (which NEEDs liba) is dlopen'd after liba is already loaded,
 * rld "re-encounters" liba. If rld re-zeroes liba's GOT during this
 * re-encounter, the constructor's stored state gets corrupted.
 *
 * This is the suspected MiniBrowser crash mechanism:
 * GOT entries become 0xFFFFFFFF after re-encounter.
 */

#include <stdio.h>

static int init_ran = 0;
static int init_value = 0;

/* Constructor — runs when library is first loaded */
__attribute__((constructor))
static void liba_init(void) {
    init_ran = 1;
    init_value = 0xCAFE;  /* sentinel value */
}

int liba_check_init(void) {
    return init_ran;
}

int liba_check_value(void) {
    return init_value;
}

int liba_compute(int x) {
    return x + init_value;
}
