/* IRIX replacement: avoid volatile function pointer static initializer */
/* Original used: static void (* volatile ssh_bzero)(void *, size_t) = bzero; */
/* which crashes on IRIX rld (relocation for bzero in static init fails). */
#include "includes.h"
#include <string.h>
#ifndef HAVE_EXPLICIT_BZERO
void
explicit_bzero(void *p, size_t n)
{
    volatile unsigned char *vp = (volatile unsigned char *)p;
    size_t i;
    if (n == 0)
        return;
    for (i = 0; i < n; i++)
        vp[i] = 0;
}
#endif /* HAVE_EXPLICIT_BZERO */
