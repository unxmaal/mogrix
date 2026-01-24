/*
 * Portable strcasestr() implementation for IRIX
 *
 * strcasestr is a GNU extension that performs case-insensitive
 * substring search. IRIX doesn't have it.
 *
 * Licensed under 3-clause BSD license.
 */

#include <ctype.h>
#include <stddef.h>
#include <string.h>

/*
 * strcasestr - case-insensitive substring search
 *
 * Locate the first occurrence of the null-terminated string needle
 * in the null-terminated string haystack, ignoring the case of both.
 *
 * Returns: Pointer to the first character of the first occurrence,
 *          or NULL if not found.
 */
char *strcasestr(const char *haystack, const char *needle)
{
    size_t needle_len;

    if (!haystack || !needle)
        return NULL;

    needle_len = strlen(needle);
    if (needle_len == 0)
        return (char *)haystack;

    while (*haystack) {
        if (tolower((unsigned char)*haystack) == tolower((unsigned char)*needle)) {
            /* Potential match - check the rest */
            const char *h = haystack;
            const char *n = needle;

            while (*n && tolower((unsigned char)*h) == tolower((unsigned char)*n)) {
                h++;
                n++;
            }

            if (*n == '\0')
                return (char *)haystack;
        }
        haystack++;
    }

    return NULL;
}
