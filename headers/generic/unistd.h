#ifndef _DICL_CLANG_COMPAT_UNISTD_H
#define _DICL_CLANG_COMPAT_UNISTD_H

/*
 * IRIX unistd.h wrapper for clang.
 *
 * Several functions are guarded by _SGIAPI which conflicts with
 * _XOPEN_SOURCE. Declare them manually after including the IRIX header.
 */

#include_next <unistd.h>

/* setgroups is guarded by _SGIAPI */
#ifndef _SETGROUPS_DECLARED
#define _SETGROUPS_DECLARED
extern int setgroups(int, const gid_t *);
#endif

#endif /* _DICL_CLANG_COMPAT_UNISTD_H */
