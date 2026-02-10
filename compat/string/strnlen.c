/* strnlen.c - provide strnlen() for IRIX
 *
 * IRIX 6.5 libc does not provide strnlen().
 * Only compiled when __sgi is defined.
 */

#if defined(__sgi)

#include <stddef.h>

size_t strnlen(const char *s, size_t maxlen)
{
    size_t len;

    for (len = 0; len < maxlen && s[len] != '\0'; len++)
        ;

    return len;
}

#endif /* __sgi */
