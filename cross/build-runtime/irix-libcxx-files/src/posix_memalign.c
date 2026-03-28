#ifndef _POSIX_MEMALIGN_H
#define _POSIX_MEMALIGN_H

#include <stdlib.h>
#include <errno.h>

int
posix_memalign (void **memptr, size_t alignment, size_t size)
{
    *memptr = memalign(alignment, size);
    if (!(*memptr)) {
        return errno;
    }
    return 0;
}

#endif
