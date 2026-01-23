/*
 * DICL clang compat stdlib.h
 * Add C99 functions not visible without __c99 mode
 */
#ifndef _DICL_STDLIB_H
#define _DICL_STDLIB_H

/* Include IRIX stdlib.h first */
#include_next <stdlib.h>

/* Add C99 functions that IRIX hides unless __c99 is defined */
#ifdef __cplusplus
extern "C" {
#endif

#ifndef _DICL_C99_STDLIB
#define _DICL_C99_STDLIB
extern long long strtoll(const char * __restrict, char ** __restrict, int);
extern unsigned long long strtoull(const char * __restrict, char ** __restrict, int);
extern long long atoll(const char *);
#endif

/* GNU extension: secure_getenv (provided by compat library) */
#ifndef _DICL_GNU_STDLIB
#define _DICL_GNU_STDLIB
extern char *secure_getenv(const char *);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _DICL_STDLIB_H */
