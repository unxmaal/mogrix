/* qsort_r - GNU extension for qsort with user data
 * IRIX doesn't have this, so we implement it using global state
 * NOTE: This is NOT thread-safe
 */
#include <stdlib.h>
#include <stddef.h>

/* Global state for qsort_r emulation */
static int (*_qsort_r_cmp)(const void *, const void *, void *);
static void *_qsort_r_arg;

static int _qsort_r_wrapper(const void *a, const void *b)
{
    return _qsort_r_cmp(a, b, _qsort_r_arg);
}

void qsort_r(void *base, size_t nmemb, size_t size,
             int (*compar)(const void *, const void *, void *),
             void *arg)
{
    _qsort_r_cmp = compar;
    _qsort_r_arg = arg;
    qsort(base, nmemb, size, _qsort_r_wrapper);
}
