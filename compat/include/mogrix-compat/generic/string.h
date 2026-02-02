/*
 * mogrix-compat/generic/string.h
 *
 * Wrapper that includes the real string.h and adds GNU extensions
 * for IRIX compatibility.
 */

#ifndef _MOGRIX_COMPAT_STRING_H
#define _MOGRIX_COMPAT_STRING_H

/* Include the real IRIX string.h */
#include_next <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * strdup - duplicate a string (POSIX.1-2001)
 * strndup - duplicate n bytes of a string (POSIX.1-2008)
 *
 * IRIX may not have declarations in strict C99 mode.
 */
#include <stddef.h>  /* for size_t */

#ifndef strdup
char *strdup(const char *s);
#endif

#ifndef strndup
char *strndup(const char *s, size_t n);
#endif

/*
 * stpcpy/stpncpy - copy string returning pointer to end (POSIX.1-2008)
 *
 * IRIX has stpcpy in libgen.so but the header is non-standard.
 * We provide declaration and implementation via mogrix-compat.
 */
char *stpcpy(char *dest, const char *src);
char *stpncpy(char *dest, const char *src, size_t n);

/*
 * strcasestr - case-insensitive substring search (GNU extension)
 *
 * IRIX doesn't have strcasestr. We provide our own implementation.
 */
#ifndef strcasestr
char *strcasestr(const char *haystack, const char *needle);
#endif

/*
 * strsep - extract token from string (BSD extension)
 *
 * IRIX doesn't have strsep. We provide our own implementation.
 */
#ifndef strsep
char *strsep(char **stringp, const char *delim);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _MOGRIX_COMPAT_STRING_H */
