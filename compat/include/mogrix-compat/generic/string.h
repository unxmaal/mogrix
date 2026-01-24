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
