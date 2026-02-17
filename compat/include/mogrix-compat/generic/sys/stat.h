/*
 * mogrix-compat/generic/sys/stat.h
 *
 * Wrapper that includes the real sys/stat.h and adds POSIX.1-2008
 * *at() function declarations that IRIX doesn't have.
 */

#ifndef _MOGRIX_COMPAT_SYS_STAT_H
#define _MOGRIX_COMPAT_SYS_STAT_H

/* Include <time.h> to ensure struct timespec is properly defined.
 * IRIX sys/timespec.h only maps __timespec→timespec under certain feature
 * macros (_POSIX93 || _ABIAPI || (_XOPEN5 && __TIME_H__)). Including <time.h>
 * sets __TIME_H__ and ensures the mapping is active, so the struct is defined
 * as "struct timespec" rather than the IRIX-internal "struct __timespec". */
#include <time.h>

/* Include the real IRIX sys/stat.h (goes through dicl-clang-compat if present) */
#include_next <sys/stat.h>

/* Undo IRIX time_core.h's reverse mapping (#define timespec __timespec).
 * After <time.h> defines the struct as "struct timespec", time_core.h maps
 * timespec back to __timespec. This #undef ensures "struct timespec" in our
 * declarations below resolves directly to the defined struct. */
#ifdef timespec
#undef timespec
#endif

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
 */
int utimensat(int dirfd, const char *pathname, const struct timespec times[2], int flags);
int futimens(int fd, const struct timespec times[2]);

#ifdef __cplusplus
}
#endif

#endif /* _MOGRIX_COMPAT_SYS_STAT_H */
