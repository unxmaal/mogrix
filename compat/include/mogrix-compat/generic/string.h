/*
 * mogrix-compat/generic/string.h
 *
 * Wrapper that includes the real string.h and adds GNU/POSIX extensions
 * for IRIX compatibility.
 *
 * IMPORTANT: Declarations are unconditional. Multiple identical declarations
 * are legal C. Conditional (#ifndef) guards caused silent failures when
 * macros defined the function names.
 */

#ifndef _MOGRIX_COMPAT_STRING_H
#define _MOGRIX_COMPAT_STRING_H

/* Include the real IRIX string.h */
#include_next <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>  /* for size_t */

/* strdup/strndup - POSIX string duplication */
char *strdup(const char *s);
char *strndup(const char *s, size_t n);

/* strnlen - POSIX.1-2008, bounded string length */
size_t strnlen(const char *s, size_t maxlen);

/* stpcpy/stpncpy - POSIX.1-2008, copy returning pointer to end */
char *stpcpy(char *dest, const char *src);
char *stpncpy(char *dest, const char *src, size_t n);

/* strcasestr - GNU extension, case-insensitive search */
char *strcasestr(const char *haystack, const char *needle);

/* strsep - BSD extension, token extraction */
char *strsep(char **stringp, const char *delim);

/* explicit_bzero - OpenBSD/glibc, zero memory without optimization */
void explicit_bzero(void *buf, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* _MOGRIX_COMPAT_STRING_H */
