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

/*
 * llabs - absolute value of long long (C99)
 *
 * IRIX 6.5 may not expose llabs depending on feature macros.
 * IRIX libc doesn't have llabs, but Clang provides it as a builtin.
 * Provide a declaration that Clang can resolve.
 */
#ifndef llabs
long long llabs(long long);
#endif

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
 * setenv/unsetenv - environment manipulation (POSIX.1-2001)
 *
 * IRIX libc has these but may not be declared in strict C99 mode.
 */
#ifndef setenv
int setenv(const char *name, const char *value, int overwrite);
#endif

#ifndef unsetenv
int unsetenv(const char *name);
#endif

/*
 * getprogname/setprogname - program name access (BSD extension)
 *
 * We provide implementations in compat library.
 */
#ifndef getprogname
const char *getprogname(void);
#endif

#ifndef setprogname
void setprogname(const char *progname);
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
