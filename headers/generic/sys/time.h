#ifndef _DICL_CLANG_COMPAT_SYS_TIME_H
#define _DICL_CLANG_COMPAT_SYS_TIME_H

/*
 * IRIX sys/time.h wrapper for clang.
 *
 * struct timezone is normally only visible with _SGIAPI or _BSD_TYPES,
 * but we can't use those with _XOPEN_SOURCE due to select() conflicts.
 * Define struct timezone manually before including the IRIX header.
 */

/* Pre-define struct timezone if it will be needed */
#ifndef _STRUCT_TIMEZONE
#define _STRUCT_TIMEZONE
struct timezone {
    int tz_minuteswest;  /* minutes west of Greenwich */
    int tz_dsttime;      /* type of dst correction */
};
#endif

#include_next <sys/time.h>

/* settimeofday is guarded by _SGIAPI || _BSD_TYPES which conflicts with
   _XOPEN_SOURCE. Declare it manually. */
#ifndef _SETTIMEOFDAY_DECLARED
#define _SETTIMEOFDAY_DECLARED
extern int settimeofday(struct timeval *, ...);
#endif

#endif /* _DICL_CLANG_COMPAT_SYS_TIME_H */
