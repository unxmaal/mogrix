/* reallocarray.c - provide reallocarray() for IRIX
 *
 * reallocarray() is a safe realloc that checks for overflow.
 * Only compiled when __sgi is defined.
 */

#if defined(__sgi)

#include <stdlib.h>
#include <errno.h>
#include <stdint.h>

void *reallocarray(void *ptr, size_t nmemb, size_t size)
{
    /* Check for overflow */
    if (nmemb != 0 && size > SIZE_MAX / nmemb) {
        errno = ENOMEM;
        return NULL;
    }

    return realloc(ptr, nmemb * size);
}

#endif /* __sgi */
