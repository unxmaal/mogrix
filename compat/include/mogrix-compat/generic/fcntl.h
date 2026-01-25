/*
 * mogrix-compat/generic/fcntl.h
 *
 * Wrapper that includes the real fcntl.h and adds POSIX.1-2008
 * constants that IRIX doesn't have natively.
 */

#ifndef _MOGRIX_COMPAT_FCNTL_H
#define _MOGRIX_COMPAT_FCNTL_H

/* Include the real IRIX fcntl.h */
#include_next <fcntl.h>

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * O_NOFOLLOW - don't follow symbolic links
 * IRIX doesn't have this. Define it as 0 so it's ignored in open().
 * Code using this should be prepared for symlinks to be followed anyway.
 */
#ifndef O_NOFOLLOW
#define O_NOFOLLOW 0
#endif

/*
 * O_CLOEXEC - close-on-exec flag
 * IRIX doesn't have this. Define as 0 - caller should use fcntl(FD_CLOEXEC).
 */
#ifndef O_CLOEXEC
#define O_CLOEXEC 0
#endif

/*
 * AT_* constants for *at() functions
 * These are POSIX.1-2008 flags used with functions like openat, fstatat, etc.
 */
#ifndef AT_FDCWD
#define AT_FDCWD (-100)  /* Special value for current working directory */
#endif

#ifndef AT_SYMLINK_NOFOLLOW
#define AT_SYMLINK_NOFOLLOW 0x100  /* Don't follow symbolic links */
#endif

#ifndef AT_REMOVEDIR
#define AT_REMOVEDIR 0x200  /* Remove directory instead of file */
#endif

#ifndef AT_SYMLINK_FOLLOW
#define AT_SYMLINK_FOLLOW 0x400  /* Follow symbolic links (for linkat) */
#endif

#ifndef AT_EACCESS
#define AT_EACCESS 0x200  /* Test access using effective IDs */
#endif

/*
 * openat - open file relative to directory file descriptor
 */
int openat(int dirfd, const char *pathname, int flags, ...);

#ifdef __cplusplus
}
#endif

#endif /* _MOGRIX_COMPAT_FCNTL_H */
