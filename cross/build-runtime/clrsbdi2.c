/* __clrsbdi2 — count leading redundant sign bits (64-bit)
 *
 * Returns the number of redundant sign bits in x, i.e., the number
 * of bits following the most significant bit that are identical to it.
 * This is equivalent to __builtin_clrsbll(x) for 64-bit values.
 *
 * Standalone implementation for clang (libgcc2.c requires GCC-specific modes).
 */

int __clrsbdi2(long long x) {
    /* Split into high and low 32-bit halves */
    unsigned int high = (unsigned int)(x >> 32);
    unsigned int low  = (unsigned int)x;

    unsigned int word;
    int add;

    if ((int)high == 0)
        word = low, add = 32;
    else if ((int)high == -1)
        word = ~low, add = 32;
    else if ((int)high >= 0)
        word = high, add = 0;
    else
        word = ~high, add = 0;

    int ret;
    if (word == 0)
        ret = 32;
    else
        ret = __builtin_clz(word) - 1;

    return ret + add;
}
