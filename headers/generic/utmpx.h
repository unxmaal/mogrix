#ifndef _DICL_CLANG_COMPAT_UTMPX_H
#define _DICL_CLANG_COMPAT_UTMPX_H

/*
 * IRIX utmpx.h wrapper for clang.
 *
 * utmpxname is guarded by _NO_XOPEN4 && _NO_XOPEN5 which conflicts
 * with _XOPEN_SOURCE. Declare it manually.
 */

#include_next <utmpx.h>

#ifndef _UTMPXNAME_DECLARED
#define _UTMPXNAME_DECLARED
extern int utmpxname(const char *);
#endif

#endif /* _DICL_CLANG_COMPAT_UTMPX_H */
