/*
 * openat/fstatat and related *at functions for IRIX
 *
 * IRIX lacks POSIX.1-2008 "at" functions. This header provides
 * declarations for compatibility implementations.
 *
 * WARNING: These implementations are NOT thread-safe!
 */

#ifndef MOGRIX_OPENAT_COMPAT_H
#define MOGRIX_OPENAT_COMPAT_H

#include <sys/types.h>
#include <sys/stat.h>

#ifdef __cplusplus
extern "C" {
#endif

/* AT_FDCWD - special value meaning "current working directory" */
#ifndef AT_FDCWD
#define AT_FDCWD (-100)
#endif

/* Flags for fstatat/fchownat/unlinkat/etc */
#ifndef AT_SYMLINK_NOFOLLOW
#define AT_SYMLINK_NOFOLLOW 0x100
#endif

#ifndef AT_REMOVEDIR
#define AT_REMOVEDIR 0x200
#endif

#ifndef AT_SYMLINK_FOLLOW
#define AT_SYMLINK_FOLLOW 0x400
#endif

#ifndef AT_EACCESS
#define AT_EACCESS 0x200
#endif

#ifndef AT_EMPTY_PATH
#define AT_EMPTY_PATH 0x1000
#endif

/* Function declarations */

/* openat - open file relative to directory file descriptor */
int openat(int dirfd, const char *pathname, int flags, ...);

/* fstatat - get file status relative to directory file descriptor */
int fstatat(int dirfd, const char *pathname, struct stat *statbuf, int flags);

/* faccessat - check user's permissions relative to dirfd */
int faccessat(int dirfd, const char *pathname, int mode, int flags);

/* mkdirat - create directory relative to dirfd */
int mkdirat(int dirfd, const char *pathname, mode_t mode);

/* unlinkat - remove file or directory relative to dirfd */
int unlinkat(int dirfd, const char *pathname, int flags);

/* renameat - rename file relative to dirfds */
int renameat(int olddirfd, const char *oldpath, int newdirfd, const char *newpath);

/* readlinkat - read symbolic link relative to dirfd */
ssize_t readlinkat(int dirfd, const char *pathname, char *buf, size_t bufsiz);

/* symlinkat - create symbolic link relative to dirfd */
int symlinkat(const char *target, int newdirfd, const char *linkpath);

/* linkat - create hard link relative to dirfds */
int linkat(int olddirfd, const char *oldpath, int newdirfd, const char *newpath, int flags);

/* fchmodat - change file mode relative to dirfd */
int fchmodat(int dirfd, const char *pathname, mode_t mode, int flags);

/* fchownat - change file owner/group relative to dirfd */
int fchownat(int dirfd, const char *pathname, uid_t owner, gid_t group, int flags);

#ifdef __cplusplus
}
#endif

#endif /* MOGRIX_OPENAT_COMPAT_H */
