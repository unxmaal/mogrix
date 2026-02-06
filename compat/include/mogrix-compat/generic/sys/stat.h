/*
 * mogrix-compat/generic/sys/stat.h
 *
 * Wrapper that includes the real sys/stat.h and adds POSIX.1-2008
 * *at() function declarations that IRIX doesn't have.
 */

#ifndef _MOGRIX_COMPAT_SYS_STAT_H
#define _MOGRIX_COMPAT_SYS_STAT_H

/* Force struct timespec mapping before IRIX headers parse sys/timespec.h.
 * dicl-clang-compat/sys/stat.h also sets this, but in case we're included
 * without dicl-clang-compat in the include path, set it here too. */
#ifndef __TIMESPEC_DEFINED
#define __timespec timespec
#define __TIMESPEC_DEFINED
#endif

/* Include the real IRIX sys/stat.h (goes through dicl-clang-compat if present) */
#include_next <sys/stat.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * fstatat - get file status relative to directory fd
 */
int fstatat(int dirfd, const char *pathname, struct stat *statbuf, int flags);

/*
 * mkdirat - create directory relative to directory fd
 */
int mkdirat(int dirfd, const char *pathname, mode_t mode);

/*
 * fchmodat - change file mode relative to directory fd
 */
int fchmodat(int dirfd, const char *pathname, mode_t mode, int flags);

/*
 * mknodat - create special file relative to directory fd
 */
int mknodat(int dirfd, const char *pathname, mode_t mode, dev_t dev);

/*
 * mkfifoat - create FIFO relative to directory fd
 */
int mkfifoat(int dirfd, const char *pathname, mode_t mode);

/*
 * utimensat/futimens - nanosecond timestamp functions
 * struct timespec is defined via the __timespec mapping above.
 */
int utimensat(int dirfd, const char *pathname, const struct timespec times[2], int flags);
int futimens(int fd, const struct timespec times[2]);

#ifdef __cplusplus
}
#endif

#endif /* _MOGRIX_COMPAT_SYS_STAT_H */
