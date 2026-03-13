/* __clrsbti2 — count leading redundant sign bits (128-bit)
 *
 * Returns the number of redundant sign bits in a 128-bit value.
 * Standalone implementation for clang (libgcc2.c requires GCC-specific modes).
 */

int __clrsbdi2(long long x);  /* from clrsbdi2.c */

int __clrsbti2(__int128 x) {
    long long high = (long long)(x >> 64);
    long long low  = (long long)x;

    long long word;
    int add;

    if (high == 0)
        word = low, add = 64;
    else if (high == -1)
        word = ~low, add = 64;
    else if (high >= 0)
        word = high, add = 0;
    else
        word = ~high, add = 0;

    int ret;
    if (word == 0)
        ret = 64;
    else
        ret = __clrsbdi2(word);

    return ret + add;
}
