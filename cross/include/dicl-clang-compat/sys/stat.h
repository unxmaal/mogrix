/*
 * DICL clang compat sys/stat.h
 * Add POSIX.1-2008 fstatat not available in IRIX sys/stat.h
 */
#ifndef _DICL_SYS_STAT_H
#define _DICL_SYS_STAT_H

/* Ensure types are defined before including IRIX sys/stat.h */
#include <sys/types.h>

/* Force struct timespec to be defined when IRIX sys/stat.h includes sys/timespec.h.
 * IRIX timespec.h defines "struct __timespec" unconditionally, but only maps
 * __timespec -> timespec under _POSIX93 || _ABIAPI. This #define must come BEFORE
 * #include_next so it's active when IRIX headers parse sys/timespec.h. */
#ifndef __TIMESPEC_DEFINED
#define __timespec timespec
#define __TIMESPEC_DEFINED
#endif

/* Include IRIX sys/stat.h (which includes sys/timespec.h internally) */
#include_next <sys/stat.h>

#ifdef __cplusplus
extern "C" {
#endif

/* fstatat - get file status relative to directory file descriptor
 * Provided by compat library */
#ifndef _DICL_HAVE_FSTATAT
#define _DICL_HAVE_FSTATAT
extern int fstatat(int dirfd, const char *pathname, struct stat *statbuf, int flags);
#endif

/* fchmodat - change file mode relative to directory file descriptor
 * Provided by compat library */
#ifndef _DICL_HAVE_FCHMODAT
#define _DICL_HAVE_FCHMODAT
extern int fchmodat(int dirfd, const char *pathname, mode_t mode, int flags);
#endif

/* mkdirat - create directory relative to directory file descriptor
 * Provided by compat library */
#ifndef _DICL_HAVE_MKDIRAT
#define _DICL_HAVE_MKDIRAT
extern int mkdirat(int dirfd, const char *pathname, mode_t mode);
#endif

/* mkfifoat - create FIFO relative to directory file descriptor
 * Provided by compat library */
#ifndef _DICL_HAVE_MKFIFOAT
#define _DICL_HAVE_MKFIFOAT
extern int mkfifoat(int dirfd, const char *pathname, mode_t mode);
#endif

/* mknodat - create special file relative to directory file descriptor
 * Provided by compat library */
#ifndef _DICL_HAVE_MKNODAT
#define _DICL_HAVE_MKNODAT
extern int mknodat(int dirfd, const char *pathname, mode_t mode, dev_t dev);
#endif

/* utimensat - change file timestamps with nanosecond precision relative to dirfd
 * Provided by compat library */
#ifndef _DICL_HAVE_UTIMENSAT
#define _DICL_HAVE_UTIMENSAT
extern int utimensat(int dirfd, const char *pathname, const struct timespec times[2], int flags);
#endif

/* futimens - change file timestamps with nanosecond precision using fd
 * Provided by compat library */
#ifndef _DICL_HAVE_FUTIMENS
#define _DICL_HAVE_FUTIMENS
extern int futimens(int fd, const struct timespec times[2]);
#endif

/* UTIME_NOW and UTIME_OMIT special values for utimensat/futimens */
#ifndef UTIME_NOW
#define UTIME_NOW  ((1L << 30) - 1L)
#endif
#ifndef UTIME_OMIT
#define UTIME_OMIT ((1L << 30) - 2L)
#endif

#ifdef __cplusplus
}
#endif

#endif /* _DICL_SYS_STAT_H */
