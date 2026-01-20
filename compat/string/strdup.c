/* strdup.c - provide strdup() for IRIX
 *
 * IRIX 6.5 libc does not provide strdup().
 * Only compiled when __sgi is defined.
 */

#if defined(__sgi)

#include <stdlib.h>
#include <string.h>

char *strdup(const char *s)
{
    size_t len;
    char *dup;

    if (s == NULL)
        return NULL;

    len = strlen(s) + 1;
    dup = (char *)malloc(len);
    if (dup != NULL)
        memcpy(dup, s, len);

    return dup;
}

#endif /* __sgi */
