/*
 * mogrix-compat/generic/stdlib.h
 *
 * Wrapper that includes the real stdlib.h and adds missing C99 functions
 * for IRIX compatibility.
 */

#ifndef _MOGRIX_COMPAT_STDLIB_H
#define _MOGRIX_COMPAT_STDLIB_H

/* Include the real IRIX stdlib.h */
#include_next <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* C99 functions missing from IRIX 6.5 */

/* strtof - convert string to float */
#ifndef strtof
float strtof(const char *nptr, char **endptr);
#endif

/* strtold - convert string to long double (if needed) */
#ifndef strtold
long double strtold(const char *nptr, char **endptr);
#endif

/*
 * mkdtemp - create a unique temporary directory (BSD/POSIX)
 *
 * IRIX may not have mkdtemp. We provide our own implementation.
 */
#ifndef mkdtemp
char *mkdtemp(char *template);
#endif

/*
 * qsort_r - qsort with user-provided argument (GNU extension)
 *
 * IRIX doesn't have qsort_r. We provide an implementation using global state.
 * Note: Not thread-safe.
 */
void qsort_r(void *base, size_t nmemb, size_t size,
             int (*compar)(const void *, const void *, void *),
             void *arg);

#ifdef __cplusplus
}
#endif

#endif /* _MOGRIX_COMPAT_STDLIB_H */
