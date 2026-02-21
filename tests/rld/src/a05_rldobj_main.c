/*
 * A05: __rld_obj_head test - main
 *
 * On IRIX, rld provides __rld_obj_head which points to the linked
 * list of loaded objects. Our toolchain must export this symbol
 * (or a compatible stub) in the exe's .dynsym.
 *
 * At runtime, we just check the binary loads and can call the lib.
 * The real check is static (verify_elf: check_rld_obj_head).
 */
#include <stdio.h>

extern int a05_func(void);

int main(void) {
    if (a05_func() != 5) {
        fprintf(stderr, "A05 FAIL\n");
        return 1;
    }
    printf("A05 PASS: binary loads, __rld_obj_head check is static\n");
    return 0;
}
