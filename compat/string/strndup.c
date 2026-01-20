/* strndup.c - provide strndup() for IRIX
 *
 * IRIX 6.5 libc does not provide strndup().
 * Only compiled when __sgi is defined.
 */

#if defined(__sgi)

#include <stdlib.h>
#include <string.h>

char *strndup(const char *s, size_t n)
{
    size_t len;
    char *dup;

    if (s == NULL)
        return NULL;

    len = strlen(s);
    if (len > n)
        len = n;

    dup = (char *)malloc(len + 1);
    if (dup != NULL) {
        memcpy(dup, s, len);
        dup[len] = '\0';
    }

    return dup;
}

#endif /* __sgi */
