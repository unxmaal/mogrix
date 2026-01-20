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

#ifdef __cplusplus
}
#endif

#endif /* _MOGRIX_COMPAT_STDLIB_H */
