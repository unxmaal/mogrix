/*
 * explicit_bzero - zero memory that won't be optimized away
 *
 * IRIX lacks explicit_bzero. GnuPG's wipememory() uses a volatile
 * function pointer to memset as a workaround, but this crashes on
 * IRIX because the static initializer relocation for the memset
 * address isn't handled correctly by rld with our cross-compiled
 * binaries.
 *
 * This implementation uses a volatile pointer to prevent optimization.
 */

#include <stddef.h>

void
explicit_bzero(void *buf, size_t len)
{
    volatile unsigned char *p = (volatile unsigned char *)buf;
    while (len--)
        *p++ = 0;
}
