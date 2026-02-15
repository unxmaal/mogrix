/*
 * DICL clang compat sys/stat.h
 * Add POSIX.1-2008 fstatat not available in IRIX sys/stat.h
 */
#ifndef _DICL_SYS_STAT_H
#define _DICL_SYS_STAT_H

/* Ensure types are defined before including IRIX sys/stat.h */
#include <sys/types.h>

/* Include <time.h> to ensure struct timespec is properly defined.
 * IRIX sys/timespec.h only maps __timespec→timespec under certain feature
 * macros (_POSIX93 || _ABIAPI || (_XOPEN5 && __TIME_H__)). Including <time.h>
 * sets __TIME_H__ and ensures the mapping is active, so the struct is defined
 * as "struct timespec" rather than the IRIX-internal "struct __timespec". */
#include <time.h>

/* Include IRIX sys/stat.h (which includes sys/timespec.h internally) */
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
