/*
 * mogrix-compat/generic/sys/stat.h
 *
 * Wrapper that includes the real sys/stat.h and adds POSIX.1-2008
 * *at() function declarations that IRIX doesn't have.
 */

#ifndef _MOGRIX_COMPAT_SYS_STAT_H
#define _MOGRIX_COMPAT_SYS_STAT_H

/* Include the real IRIX sys/stat.h */
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
 * utimensat - change file timestamps relative to directory fd
 */
struct timespec;
int utimensat(int dirfd, const char *pathname, const struct timespec times[2], int flags);

/*
 * futimens - change file timestamps via fd
 */
int futimens(int fd, const struct timespec times[2]);

#ifdef __cplusplus
}
#endif

#endif /* _MOGRIX_COMPAT_SYS_STAT_H */
