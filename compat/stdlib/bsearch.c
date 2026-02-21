/* bsearch.c - POSIX-compliant bsearch() for IRIX 6.5
 *
 * IRIX libc's bsearch() does not handle nmemb=0 correctly.
 * When called with base=NULL and nmemb=0, it computes:
 *   last = base + size * (nmemb - 1)
 *        = NULL + size * 0xFFFFFFFF
 *        = huge pointer (unsigned wraparound)
 * The overflow check (last < base) fails because 0xFFFFFFE0 >= 0,
 * so bsearch proceeds to call the comparator with a garbage pointer,
 * causing SIGSEGV.
 *
 * POSIX and ISO C require bsearch to return NULL when nmemb=0.
 * This implementation provides correct behavior.
 */

#include <stddef.h>

void *
bsearch(const void *key, const void *base, size_t nmemb, size_t size,
        int (*compar)(const void *, const void *))
{
    size_t lo, hi, mid;
    const unsigned char *p;
    int cmp;

    if (nmemb == 0 || size == 0)
        return NULL;

    lo = 0;
    hi = nmemb;
    while (lo < hi) {
        mid = lo + (hi - lo) / 2;
        p = (const unsigned char *)base + mid * size;
        cmp = compar(key, p);
        if (cmp < 0) {
            hi = mid;
        } else if (cmp > 0) {
            lo = mid + 1;
        } else {
            return (void *)p;
        }
    }
    return NULL;
}
