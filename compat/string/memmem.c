/*
 * memmem - find a byte sequence within a memory region.
 * IRIX libc lacks this GNU extension.
 */
#include <string.h>
#include <stddef.h>

void *memmem(const void *haystack, size_t haystacklen,
             const void *needle, size_t needlelen)
{
    const unsigned char *h = (const unsigned char *)haystack;
    const unsigned char *n = (const unsigned char *)needle;

    if (needlelen == 0)
        return (void *)haystack;
    if (needlelen > haystacklen)
        return NULL;

    size_t last = haystacklen - needlelen;
    for (size_t i = 0; i <= last; i++) {
        if (h[i] == n[0] && memcmp(h + i, n, needlelen) == 0)
            return (void *)(h + i);
    }
    return NULL;
}
