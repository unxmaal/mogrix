/* iswblank — IRIX wctype.h declares it but libc doesn't implement it */
#include <wctype.h>

int iswblank(wint_t c) {
    return c == L' ' || c == L'\t';
}
