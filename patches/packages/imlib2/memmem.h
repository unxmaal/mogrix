/* memmem - IRIX lacks this GNU extension */
#ifndef MOGRIX_MEMMEM_H
#define MOGRIX_MEMMEM_H

#include <string.h>

static inline void *
mogrix_memmem(const void *haystack, size_t hlen,
              const void *needle, size_t nlen)
{
    const char *hp = (const char *)haystack;
    const char *np = (const char *)needle;
    size_t i;

    if (nlen == 0)
        return (void *)haystack;
    if (nlen > hlen)
        return NULL;
    for (i = 0; i <= hlen - nlen; i++)
        if (memcmp(hp + i, np, nlen) == 0)
            return (void *)(hp + i);
    return NULL;
}

#define memmem mogrix_memmem

#endif /* MOGRIX_MEMMEM_H */
