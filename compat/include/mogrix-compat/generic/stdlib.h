/*
 * mogrix-compat/generic/stdlib.h
 *
 * Wrapper that includes the real stdlib.h and adds missing C99 functions
 * for IRIX compatibility.
 *
 * IRIX gates C99 long long declarations (lldiv_t, llabs, strtoll, etc.)
 * behind: #if defined(__c99) || ((_SGIAPI || _ABIAPI) && _NO_ANSIMODE)
 * Since _SGIAPI uses defined() in macro expansion (UB in clang), it
 * evaluates unpredictably. Force the gate open so IRIX always provides
 * these declarations, avoiding typedef conflicts from our own copies.
 */

#ifndef _MOGRIX_COMPAT_STDLIB_H
#define _MOGRIX_COMPAT_STDLIB_H

/*
 * Force IRIX stdlib_core.h to expose C99 long long declarations.
 * Pre-include standards.h so its include guard prevents it from
 * re-defining _SGIAPI inside #include_next (which would override
 * our forced value).
 */
#include <standards.h>

#pragma push_macro("_SGIAPI")
#pragma push_macro("_NO_ANSIMODE")
#undef _SGIAPI
#define _SGIAPI 1
#undef _NO_ANSIMODE
#define _NO_ANSIMODE 1

#include_next <stdlib.h>

#pragma pop_macro("_NO_ANSIMODE")
#pragma pop_macro("_SGIAPI")

#ifdef __cplusplus
extern "C" {
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
char *mkdtemp(char *tmpl);
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
 * posix_memalign - aligned memory allocation (POSIX.1-2001)
 *
 * IRIX 6.5 has memalign() in <malloc.h> but not posix_memalign().
 * Provide a declaration so gnulib's wrapper can see it.
 * The actual implementation comes from dlmalloc (dlposix_memalign → posix_memalign).
 * If dlmalloc is not linked, memalign-based fallback would be needed.
 */
#ifndef posix_memalign
int posix_memalign(void **memptr, size_t alignment, size_t size);
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

/*
 * _Exit - immediate program termination (C99 7.20.4.4)
 *
 * IRIX has _Exit in libc but the declaration is gated behind __c99.
 */
#ifndef _Exit
void _Exit(int) __attribute__((__noreturn__));
#endif

#ifdef __cplusplus
}
#endif

#endif /* _MOGRIX_COMPAT_STDLIB_H */
