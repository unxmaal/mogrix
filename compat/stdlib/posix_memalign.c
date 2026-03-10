/*
 * posix_memalign - aligned memory allocation (POSIX.1-2001)
 *
 * IRIX 6.5 has memalign() but not posix_memalign().
 * This wraps memalign() with POSIX semantics:
 *   - alignment must be a power of 2 and a multiple of sizeof(void*)
 *   - returns 0 on success, EINVAL or ENOMEM on failure
 *   - memory is freeable with free()
 */
#include <stdlib.h>
#include <errno.h>
#include <malloc.h>  /* IRIX memalign() */

int posix_memalign(void **memptr, size_t alignment, size_t size) {
    void *p;

    /* alignment must be a power of 2 and multiple of sizeof(void*) */
    if (alignment % sizeof(void *) != 0 ||
        (alignment & (alignment - 1)) != 0 ||
        alignment == 0) {
        return EINVAL;
    }

    if (size == 0) {
        *memptr = NULL;
        return 0;
    }

    p = memalign(alignment, size);
    if (p == NULL)
        return ENOMEM;

    *memptr = p;
    return 0;
}
