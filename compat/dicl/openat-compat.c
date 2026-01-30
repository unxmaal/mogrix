/*
 * openat/fstatat and related *at functions for IRIX
 *
 * IRIX lacks the POSIX.1-2008 "at" functions. This provides
 * compatibility implementations using the save-cwd/fchdir approach
 * from gnulib.
 *
 * WARNING: Not thread-safe! Uses chdir internally.
 *
 * License: LGPL-2.1+ (compatible with gnulib)
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <limits.h>
#include <sys/time.h>

/* AT_FDCWD - special value meaning "current working directory" */
#ifndef AT_FDCWD
#define AT_FDCWD (-100)
#endif

/* Flags for fstatat/fchownat/etc */
#ifndef AT_SYMLINK_NOFOLLOW
#define AT_SYMLINK_NOFOLLOW 0x100
#endif

#ifndef AT_REMOVEDIR
#define AT_REMOVEDIR 0x200
#endif

#ifndef AT_EACCESS
#define AT_EACCESS 0x200
#endif

/*
 * Helper: Save the current working directory
 * Returns fd to saved cwd, or -1 on error
 */
static int save_cwd(void) {
    return open(".", O_RDONLY);
}

/*
 * Helper: Restore working directory from saved fd
 */
static int restore_cwd(int saved_cwd) {
    int result = 0;
    if (saved_cwd >= 0) {
        result = fchdir(saved_cwd);
        close(saved_cwd);
    }
    return result;
}

/*
 * Helper: Check if path is absolute
 */
static int is_absolute_path(const char *path) {
    return path && path[0] == '/';
}

/* Debug flag - set to 1 to enable debug output */
static int openat_debug = 0;

/*
 * openat - open file relative to directory file descriptor
 */
int openat(int dirfd, const char *pathname, int flags, ...) {
    mode_t mode = 0;
    int saved_cwd;
    int result;
    int saved_errno;

    /* Check for debug env var on first call */
    static int debug_checked = 0;
    if (!debug_checked) {
        debug_checked = 1;
        if (getenv("OPENAT_DEBUG")) openat_debug = 1;
    }

    /* Handle variable argument for mode when O_CREAT is specified */
    if (flags & O_CREAT) {
        va_list ap;
        va_start(ap, flags);
        mode = va_arg(ap, int);  /* mode_t is promoted to int */
        va_end(ap);
    }

    /* If dirfd is AT_FDCWD or path is absolute, use regular open */
    if (dirfd == AT_FDCWD || is_absolute_path(pathname)) {
        if (flags & O_CREAT) {
            result = open(pathname, flags, mode);
        } else {
            result = open(pathname, flags);
        }
        if (openat_debug) {
            fprintf(stderr, "openat(dirfd=%d, '%s', 0x%x) -> absolute path, result=%d, errno=%d\n",
                    dirfd, pathname, flags, result, result < 0 ? errno : 0);
        }
        return result;
    }

    /* Save current working directory */
    saved_cwd = save_cwd();
    if (saved_cwd < 0) {
        return -1;
    }

    /* Change to the directory specified by dirfd */
    if (fchdir(dirfd) < 0) {
        saved_errno = errno;
        close(saved_cwd);
        errno = saved_errno;
        return -1;
    }

    /* Perform the open */
    if (flags & O_CREAT) {
        result = open(pathname, flags, mode);
    } else {
        result = open(pathname, flags);
    }
    /* Only save errno if open() failed - on success, errno is undefined */
    if (result < 0) {
        saved_errno = errno;
    } else {
        saved_errno = 0;  /* Clear errno on success */
    }

    /* Restore original working directory */
    int restore_result = restore_cwd(saved_cwd);
    if (restore_result < 0 && result >= 0) {
        /* Failed to restore cwd but open succeeded - close and fail */
        if (openat_debug) {
            fprintf(stderr, "openat(dirfd=%d, '%s') -> restore_cwd failed, closing result=%d\n",
                    dirfd, pathname, result);
        }
        close(result);
        errno = saved_errno;
        return -1;
    }

    if (openat_debug) {
        fprintf(stderr, "openat(dirfd=%d, '%s', 0x%x) -> result=%d, saved_errno=%d, restore=%d\n",
                dirfd, pathname, flags, result, saved_errno, restore_result);
    }

    errno = saved_errno;
    return result;
}

/*
 * fstatat - get file status relative to directory file descriptor
 */
int fstatat(int dirfd, const char *pathname, struct stat *statbuf, int flags) {
    int saved_cwd;
    int result;
    int saved_errno;

    /* If dirfd is AT_FDCWD or path is absolute, use regular stat/lstat */
    if (dirfd == AT_FDCWD || is_absolute_path(pathname)) {
        if (flags & AT_SYMLINK_NOFOLLOW) {
            return lstat(pathname, statbuf);
        } else {
            return stat(pathname, statbuf);
        }
    }

    /* Save current working directory */
    saved_cwd = save_cwd();
    if (saved_cwd < 0) {
        return -1;
    }

    /* Change to the directory specified by dirfd */
    if (fchdir(dirfd) < 0) {
        saved_errno = errno;
        close(saved_cwd);
        errno = saved_errno;
        return -1;
    }

    /* Perform the stat */
    if (flags & AT_SYMLINK_NOFOLLOW) {
        result = lstat(pathname, statbuf);
    } else {
        result = stat(pathname, statbuf);
    }
    /* Only save errno if operation failed */
    if (result < 0) {
        saved_errno = errno;
    } else {
        saved_errno = 0;
    }

    /* Restore original working directory */
    restore_cwd(saved_cwd);

    errno = saved_errno;
    return result;
}

/*
 * faccessat - check user's permissions for a file relative to dirfd
 */
int faccessat(int dirfd, const char *pathname, int mode, int flags) {
    int saved_cwd;
    int result;
    int saved_errno;

    /* If dirfd is AT_FDCWD or path is absolute, use regular access */
    if (dirfd == AT_FDCWD || is_absolute_path(pathname)) {
        /* Note: AT_EACCESS flag not supported in this simple implementation */
        return access(pathname, mode);
    }

    /* Save current working directory */
    saved_cwd = save_cwd();
    if (saved_cwd < 0) {
        return -1;
    }

    /* Change to the directory specified by dirfd */
    if (fchdir(dirfd) < 0) {
        saved_errno = errno;
        close(saved_cwd);
        errno = saved_errno;
        return -1;
    }

    /* Perform the access check */
    result = access(pathname, mode);
    /* Only save errno if operation failed */
    if (result < 0) {
        saved_errno = errno;
    } else {
        saved_errno = 0;
    }

    /* Restore original working directory */
    restore_cwd(saved_cwd);

    errno = saved_errno;
    return result;
}

/*
 * mkdirat - create a directory relative to dirfd
 */
int mkdirat(int dirfd, const char *pathname, mode_t mode) {
    int saved_cwd;
    int result;
    int saved_errno;

    /* If dirfd is AT_FDCWD or path is absolute, use regular mkdir */
    if (dirfd == AT_FDCWD || is_absolute_path(pathname)) {
        return mkdir(pathname, mode);
    }

    /* Save current working directory */
    saved_cwd = save_cwd();
    if (saved_cwd < 0) {
        return -1;
    }

    /* Change to the directory specified by dirfd */
    if (fchdir(dirfd) < 0) {
        saved_errno = errno;
        close(saved_cwd);
        errno = saved_errno;
        return -1;
    }

    /* Perform the mkdir */
    result = mkdir(pathname, mode);
    /* Only save errno if operation failed */
    if (result < 0) {
        saved_errno = errno;
    } else {
        saved_errno = 0;
    }

    /* Restore original working directory */
    restore_cwd(saved_cwd);

    errno = saved_errno;
    return result;
}

/*
 * unlinkat - remove a file or directory relative to dirfd
 */
int unlinkat(int dirfd, const char *pathname, int flags) {
    int saved_cwd;
    int result;
    int saved_errno;

    /* If dirfd is AT_FDCWD or path is absolute, use regular unlink/rmdir */
    if (dirfd == AT_FDCWD || is_absolute_path(pathname)) {
        if (flags & AT_REMOVEDIR) {
            return rmdir(pathname);
        } else {
            return unlink(pathname);
        }
    }

    /* Save current working directory */
    saved_cwd = save_cwd();
    if (saved_cwd < 0) {
        return -1;
    }

    /* Change to the directory specified by dirfd */
    if (fchdir(dirfd) < 0) {
        saved_errno = errno;
        close(saved_cwd);
        errno = saved_errno;
        return -1;
    }

    /* Perform the unlink or rmdir */
    if (flags & AT_REMOVEDIR) {
        result = rmdir(pathname);
    } else {
        result = unlink(pathname);
    }
    /* Only save errno if operation failed */
    if (result < 0) {
        saved_errno = errno;
    } else {
        saved_errno = 0;
    }

    /* Restore original working directory */
    restore_cwd(saved_cwd);

    errno = saved_errno;
    return result;
}

/*
 * renameat - rename a file relative to dirfds
 */
int renameat(int olddirfd, const char *oldpath,
             int newdirfd, const char *newpath) {
    int saved_cwd;
    int result;
    int saved_errno;
    char *old_fullpath = NULL;
    char *new_fullpath = NULL;

    /* Simple case: both AT_FDCWD or both absolute */
    if ((olddirfd == AT_FDCWD || is_absolute_path(oldpath)) &&
        (newdirfd == AT_FDCWD || is_absolute_path(newpath))) {
        return rename(oldpath, newpath);
    }

    /* More complex case: need to build full paths
     * This is a simplified implementation that only handles
     * common cases. A full implementation would need to
     * handle relative paths properly.
     */

    /* Save current working directory */
    saved_cwd = save_cwd();
    if (saved_cwd < 0) {
        return -1;
    }

    /* For simplicity, if either is relative to a dirfd,
     * we construct the path by changing to that directory
     * and using getcwd + path. This is not perfect but works
     * for most rpm use cases.
     */

    /* Handle oldpath */
    if (olddirfd != AT_FDCWD && !is_absolute_path(oldpath)) {
        char cwd_buf[PATH_MAX];
        if (fchdir(olddirfd) < 0) {
            saved_errno = errno;
            close(saved_cwd);
            errno = saved_errno;
            return -1;
        }
        if (getcwd(cwd_buf, sizeof(cwd_buf)) == NULL) {
            saved_errno = errno;
            fchdir(saved_cwd);
            close(saved_cwd);
            errno = saved_errno;
            return -1;
        }
        old_fullpath = malloc(strlen(cwd_buf) + strlen(oldpath) + 2);
        if (!old_fullpath) {
            fchdir(saved_cwd);
            close(saved_cwd);
            errno = ENOMEM;
            return -1;
        }
        sprintf(old_fullpath, "%s/%s", cwd_buf, oldpath);
        fchdir(saved_cwd);
    }

    /* Handle newpath */
    if (newdirfd != AT_FDCWD && !is_absolute_path(newpath)) {
        char cwd_buf[PATH_MAX];
        if (fchdir(newdirfd) < 0) {
            saved_errno = errno;
            free(old_fullpath);
            close(saved_cwd);
            errno = saved_errno;
            return -1;
        }
        if (getcwd(cwd_buf, sizeof(cwd_buf)) == NULL) {
            saved_errno = errno;
            free(old_fullpath);
            fchdir(saved_cwd);
            close(saved_cwd);
            errno = saved_errno;
            return -1;
        }
        new_fullpath = malloc(strlen(cwd_buf) + strlen(newpath) + 2);
        if (!new_fullpath) {
            free(old_fullpath);
            fchdir(saved_cwd);
            close(saved_cwd);
            errno = ENOMEM;
            return -1;
        }
        sprintf(new_fullpath, "%s/%s", cwd_buf, newpath);
        fchdir(saved_cwd);
    }

    /* Perform the rename */
    result = rename(old_fullpath ? old_fullpath : oldpath,
                    new_fullpath ? new_fullpath : newpath);
    /* Only save errno if operation failed */
    if (result < 0) {
        saved_errno = errno;
    } else {
        saved_errno = 0;
    }

    /* Cleanup */
    free(old_fullpath);
    free(new_fullpath);
    restore_cwd(saved_cwd);

    errno = saved_errno;
    return result;
}

/*
 * readlinkat - read symbolic link relative to dirfd
 */
ssize_t readlinkat(int dirfd, const char *pathname, char *buf, size_t bufsiz) {
    int saved_cwd;
    ssize_t result;
    int saved_errno;

    /* If dirfd is AT_FDCWD or path is absolute, use regular readlink */
    if (dirfd == AT_FDCWD || is_absolute_path(pathname)) {
        return readlink(pathname, buf, bufsiz);
    }

    /* Save current working directory */
    saved_cwd = save_cwd();
    if (saved_cwd < 0) {
        return -1;
    }

    /* Change to the directory specified by dirfd */
    if (fchdir(dirfd) < 0) {
        saved_errno = errno;
        close(saved_cwd);
        errno = saved_errno;
        return -1;
    }

    /* Perform the readlink */
    result = readlink(pathname, buf, bufsiz);
    /* Only save errno if operation failed */
    if (result < 0) {
        saved_errno = errno;
    } else {
        saved_errno = 0;
    }

    /* Restore original working directory */
    restore_cwd(saved_cwd);

    errno = saved_errno;
    return result;
}

/*
 * symlinkat - create symbolic link relative to dirfd
 */
int symlinkat(const char *target, int newdirfd, const char *linkpath) {
    int saved_cwd;
    int result;
    int saved_errno;

    /* If newdirfd is AT_FDCWD or linkpath is absolute, use regular symlink */
    if (newdirfd == AT_FDCWD || is_absolute_path(linkpath)) {
        return symlink(target, linkpath);
    }

    /* Save current working directory */
    saved_cwd = save_cwd();
    if (saved_cwd < 0) {
        return -1;
    }

    /* Change to the directory specified by newdirfd */
    if (fchdir(newdirfd) < 0) {
        saved_errno = errno;
        close(saved_cwd);
        errno = saved_errno;
        return -1;
    }

    /* Perform the symlink */
    result = symlink(target, linkpath);
    /* Only save errno if operation failed */
    if (result < 0) {
        saved_errno = errno;
    } else {
        saved_errno = 0;
    }

    /* Restore original working directory */
    restore_cwd(saved_cwd);

    errno = saved_errno;
    return result;
}

/*
 * linkat - create hard link relative to dirfds
 */
int linkat(int olddirfd, const char *oldpath,
           int newdirfd, const char *newpath, int flags) {
    int saved_cwd;
    int result;
    int saved_errno;
    char *old_fullpath = NULL;
    char *new_fullpath = NULL;

    /* Note: AT_SYMLINK_FOLLOW flag not supported in simple impl */

    /* Simple case: both AT_FDCWD or both absolute */
    if ((olddirfd == AT_FDCWD || is_absolute_path(oldpath)) &&
        (newdirfd == AT_FDCWD || is_absolute_path(newpath))) {
        return link(oldpath, newpath);
    }

    /* Save current working directory */
    saved_cwd = save_cwd();
    if (saved_cwd < 0) {
        return -1;
    }

    /* Handle oldpath - similar to renameat */
    if (olddirfd != AT_FDCWD && !is_absolute_path(oldpath)) {
        char cwd_buf[PATH_MAX];
        if (fchdir(olddirfd) < 0) {
            saved_errno = errno;
            close(saved_cwd);
            errno = saved_errno;
            return -1;
        }
        if (getcwd(cwd_buf, sizeof(cwd_buf)) == NULL) {
            saved_errno = errno;
            fchdir(saved_cwd);
            close(saved_cwd);
            errno = saved_errno;
            return -1;
        }
        old_fullpath = malloc(strlen(cwd_buf) + strlen(oldpath) + 2);
        if (!old_fullpath) {
            fchdir(saved_cwd);
            close(saved_cwd);
            errno = ENOMEM;
            return -1;
        }
        sprintf(old_fullpath, "%s/%s", cwd_buf, oldpath);
        fchdir(saved_cwd);
    }

    /* Handle newpath */
    if (newdirfd != AT_FDCWD && !is_absolute_path(newpath)) {
        char cwd_buf[PATH_MAX];
        if (fchdir(newdirfd) < 0) {
            saved_errno = errno;
            free(old_fullpath);
            close(saved_cwd);
            errno = saved_errno;
            return -1;
        }
        if (getcwd(cwd_buf, sizeof(cwd_buf)) == NULL) {
            saved_errno = errno;
            free(old_fullpath);
            fchdir(saved_cwd);
            close(saved_cwd);
            errno = saved_errno;
            return -1;
        }
        new_fullpath = malloc(strlen(cwd_buf) + strlen(newpath) + 2);
        if (!new_fullpath) {
            free(old_fullpath);
            fchdir(saved_cwd);
            close(saved_cwd);
            errno = ENOMEM;
            return -1;
        }
        sprintf(new_fullpath, "%s/%s", cwd_buf, newpath);
        fchdir(saved_cwd);
    }

    /* Perform the link */
    result = link(old_fullpath ? old_fullpath : oldpath,
                  new_fullpath ? new_fullpath : newpath);
    /* Only save errno if operation failed */
    if (result < 0) {
        saved_errno = errno;
    } else {
        saved_errno = 0;
    }

    /* Cleanup */
    free(old_fullpath);
    free(new_fullpath);
    restore_cwd(saved_cwd);

    errno = saved_errno;
    return result;
}

/*
 * fchmodat - change file mode relative to dirfd
 */
int fchmodat(int dirfd, const char *pathname, mode_t mode, int flags) {
    int saved_cwd;
    int result;
    int saved_errno;

    /* Note: AT_SYMLINK_NOFOLLOW not supported - chmod can't change symlink mode */

    /* If dirfd is AT_FDCWD or path is absolute, use regular chmod */
    if (dirfd == AT_FDCWD || is_absolute_path(pathname)) {
        return chmod(pathname, mode);
    }

    /* Save current working directory */
    saved_cwd = save_cwd();
    if (saved_cwd < 0) {
        return -1;
    }

    /* Change to the directory specified by dirfd */
    if (fchdir(dirfd) < 0) {
        saved_errno = errno;
        close(saved_cwd);
        errno = saved_errno;
        return -1;
    }

    /* Perform the chmod */
    result = chmod(pathname, mode);
    /* Only save errno if operation failed */
    if (result < 0) {
        saved_errno = errno;
    } else {
        saved_errno = 0;
    }

    /* Restore original working directory */
    restore_cwd(saved_cwd);

    errno = saved_errno;
    return result;
}

/*
 * fchownat - change file owner/group relative to dirfd
 */
int fchownat(int dirfd, const char *pathname, uid_t owner, gid_t group, int flags) {
    int saved_cwd;
    int result;
    int saved_errno;

    /* If dirfd is AT_FDCWD or path is absolute, use regular chown/lchown */
    if (dirfd == AT_FDCWD || is_absolute_path(pathname)) {
        if (flags & AT_SYMLINK_NOFOLLOW) {
            return lchown(pathname, owner, group);
        } else {
            return chown(pathname, owner, group);
        }
    }

    /* Save current working directory */
    saved_cwd = save_cwd();
    if (saved_cwd < 0) {
        return -1;
    }

    /* Change to the directory specified by dirfd */
    if (fchdir(dirfd) < 0) {
        saved_errno = errno;
        close(saved_cwd);
        errno = saved_errno;
        return -1;
    }

    /* Perform the chown */
    if (flags & AT_SYMLINK_NOFOLLOW) {
        result = lchown(pathname, owner, group);
    } else {
        result = chown(pathname, owner, group);
    }
    /* Only save errno if operation failed */
    if (result < 0) {
        saved_errno = errno;
    } else {
        saved_errno = 0;
    }

    /* Restore original working directory */
    restore_cwd(saved_cwd);

    errno = saved_errno;
    return result;
}

/*
 * mkfifoat - create FIFO relative to directory fd
 */
int mkfifoat(int dirfd, const char *pathname, mode_t mode) {
    int saved_cwd;
    int result;
    int saved_errno;

    /* If dirfd is AT_FDCWD or path is absolute, use regular mkfifo */
    if (dirfd == AT_FDCWD || is_absolute_path(pathname)) {
        return mkfifo(pathname, mode);
    }

    /* Save current working directory */
    saved_cwd = save_cwd();
    if (saved_cwd < 0) {
        return -1;
    }

    /* Change to the directory specified by dirfd */
    if (fchdir(dirfd) < 0) {
        saved_errno = errno;
        close(saved_cwd);
        errno = saved_errno;
        return -1;
    }

    /* Perform the mkfifo */
    result = mkfifo(pathname, mode);
    /* Only save errno if operation failed */
    if (result < 0) {
        saved_errno = errno;
    } else {
        saved_errno = 0;
    }

    /* Restore original working directory */
    restore_cwd(saved_cwd);

    errno = saved_errno;
    return result;
}

/*
 * mknodat - create special file relative to directory fd
 */
int mknodat(int dirfd, const char *pathname, mode_t mode, dev_t dev) {
    int saved_cwd;
    int result;
    int saved_errno;

    /* If dirfd is AT_FDCWD or path is absolute, use regular mknod */
    if (dirfd == AT_FDCWD || is_absolute_path(pathname)) {
        return mknod(pathname, mode, dev);
    }

    /* Save current working directory */
    saved_cwd = save_cwd();
    if (saved_cwd < 0) {
        return -1;
    }

    /* Change to the directory specified by dirfd */
    if (fchdir(dirfd) < 0) {
        saved_errno = errno;
        close(saved_cwd);
        errno = saved_errno;
        return -1;
    }

    /* Perform the mknod */
    result = mknod(pathname, mode, dev);
    /* Only save errno if operation failed */
    if (result < 0) {
        saved_errno = errno;
    } else {
        saved_errno = 0;
    }

    /* Restore original working directory */
    restore_cwd(saved_cwd);

    errno = saved_errno;
    return result;
}

/*
 * utimensat - change file timestamps with nanosecond precision relative to dirfd
 *
 * Note: IRIX doesn't have utimensat or utimes with nanosecond precision.
 * This implementation falls back to utimes() which only has microsecond precision.
 */
int utimensat(int dirfd, const char *pathname, const struct timespec times[2], int flags) {
    int saved_cwd;
    int result;
    int saved_errno;
    struct timeval tv[2];
    struct stat sb;

    /* Convert timespec to timeval, losing nanosecond precision */
    if (times) {
        /* Handle special values UTIME_NOW and UTIME_OMIT */
        if (times[0].tv_nsec == 999999999L || times[1].tv_nsec == 999999999L) {
            /* UTIME_NOW or UTIME_OMIT - use current time */
            struct timeval now;
            gettimeofday(&now, NULL);
            if (times[0].tv_nsec == 999999999L) {
                tv[0] = now;
            } else {
                tv[0].tv_sec = times[0].tv_sec;
                tv[0].tv_usec = times[0].tv_nsec / 1000;
            }
            if (times[1].tv_nsec == 999999999L) {
                tv[1] = now;
            } else {
                tv[1].tv_sec = times[1].tv_sec;
                tv[1].tv_usec = times[1].tv_nsec / 1000;
            }
        } else {
            tv[0].tv_sec = times[0].tv_sec;
            tv[0].tv_usec = times[0].tv_nsec / 1000;
            tv[1].tv_sec = times[1].tv_sec;
            tv[1].tv_usec = times[1].tv_nsec / 1000;
        }
    }

    /*
     * Handle AT_SYMLINK_NOFOLLOW for symlinks.
     * IRIX's utimes() follows symlinks, which can fail if the symlink target
     * doesn't exist. When AT_SYMLINK_NOFOLLOW is set and the path is a symlink,
     * we simply succeed since IRIX doesn't have lutimes() and symlink timestamps
     * are rarely critical.
     */
    if (flags & AT_SYMLINK_NOFOLLOW) {
        /* Do an lstat to check if it's a symlink */
        if (dirfd == AT_FDCWD || is_absolute_path(pathname)) {
            if (lstat(pathname, &sb) == 0 && S_ISLNK(sb.st_mode)) {
                if (openat_debug) {
                    fprintf(stderr, "utimensat: '%s' is symlink with AT_SYMLINK_NOFOLLOW, skipping (no lutimes)\n", pathname);
                }
                return 0; /* Success - can't change symlink times on IRIX */
            }
        } else {
            /* For relative paths, need to fchdir first */
            saved_cwd = save_cwd();
            if (saved_cwd >= 0) {
                if (fchdir(dirfd) == 0) {
                    if (lstat(pathname, &sb) == 0 && S_ISLNK(sb.st_mode)) {
                        restore_cwd(saved_cwd);
                        if (openat_debug) {
                            fprintf(stderr, "utimensat: '%s' is symlink with AT_SYMLINK_NOFOLLOW, skipping (no lutimes)\n", pathname);
                        }
                        return 0; /* Success - can't change symlink times on IRIX */
                    }
                }
                restore_cwd(saved_cwd);
            }
        }
    }

    /* If dirfd is AT_FDCWD or path is absolute, use regular utimes */
    if (dirfd == AT_FDCWD || is_absolute_path(pathname)) {
        /* Note: AT_SYMLINK_NOFOLLOW not fully supported */
        result = utimes(pathname, times ? tv : NULL);
        if (openat_debug && result < 0) {
            fprintf(stderr, "utimensat(AT_FDCWD, '%s', ...) -> utimes failed: %d (%s)\n",
                    pathname, errno, strerror(errno));
        }
        return result;
    }

    /* Save current working directory */
    saved_cwd = save_cwd();
    if (saved_cwd < 0) {
        if (openat_debug) {
            fprintf(stderr, "utimensat(%d, '%s', ...) -> save_cwd failed: %d (%s)\n",
                    dirfd, pathname, errno, strerror(errno));
        }
        return -1;
    }

    /* Change to the directory specified by dirfd */
    if (fchdir(dirfd) < 0) {
        saved_errno = errno;
        if (openat_debug) {
            fprintf(stderr, "utimensat(%d, '%s', ...) -> fchdir failed: %d (%s)\n",
                    dirfd, pathname, saved_errno, strerror(saved_errno));
        }
        close(saved_cwd);
        errno = saved_errno;
        return -1;
    }

    /* Perform the utimes */
    result = utimes(pathname, times ? tv : NULL);
    /* Only save errno if operation failed */
    if (result < 0) {
        saved_errno = errno;
        if (openat_debug) {
            fprintf(stderr, "utimensat(%d, '%s', ...) -> utimes failed: %d (%s)\n",
                    dirfd, pathname, saved_errno, strerror(saved_errno));
        }
    } else {
        saved_errno = 0;
        if (openat_debug) {
            fprintf(stderr, "utimensat(%d, '%s', ...) -> success\n", dirfd, pathname);
        }
    }

    /* Restore original working directory */
    restore_cwd(saved_cwd);

    errno = saved_errno;
    return result;
}

/*
 * futimens - change file timestamps with nanosecond precision using fd
 *
 * Note: IRIX doesn't have futimens or futimes.
 * This implementation uses /dev/fd/N to convert fd to path,
 * then uses utimes() which only has microsecond precision.
 */
int futimens(int fd, const struct timespec times[2]) {
    struct timeval tv[2];
    char fdpath[32];

    /* Build path to /dev/fd/N */
    sprintf(fdpath, "/dev/fd/%d", fd);

    /* Convert timespec to timeval, losing nanosecond precision */
    if (times) {
        /* Handle special values UTIME_NOW and UTIME_OMIT */
        /* UTIME_NOW = ((1l << 30) - 1l), UTIME_OMIT = ((1l << 30) - 2l) */
        if (times[0].tv_nsec == ((1L << 30) - 1L) ||
            times[1].tv_nsec == ((1L << 30) - 1L) ||
            times[0].tv_nsec == ((1L << 30) - 2L) ||
            times[1].tv_nsec == ((1L << 30) - 2L)) {
            /* Get current times for UTIME_NOW handling */
            struct timeval now;
            struct stat st;

            gettimeofday(&now, NULL);

            /* For UTIME_OMIT, we need current file times */
            if (times[0].tv_nsec == ((1L << 30) - 2L) ||
                times[1].tv_nsec == ((1L << 30) - 2L)) {
                if (fstat(fd, &st) < 0) {
                    return -1;
                }
            }

            /* Handle atime */
            if (times[0].tv_nsec == ((1L << 30) - 1L)) {
                tv[0] = now;
            } else if (times[0].tv_nsec == ((1L << 30) - 2L)) {
                tv[0].tv_sec = st.st_atime;
                tv[0].tv_usec = 0;
            } else {
                tv[0].tv_sec = times[0].tv_sec;
                tv[0].tv_usec = times[0].tv_nsec / 1000;
            }

            /* Handle mtime */
            if (times[1].tv_nsec == ((1L << 30) - 1L)) {
                tv[1] = now;
            } else if (times[1].tv_nsec == ((1L << 30) - 2L)) {
                tv[1].tv_sec = st.st_mtime;
                tv[1].tv_usec = 0;
            } else {
                tv[1].tv_sec = times[1].tv_sec;
                tv[1].tv_usec = times[1].tv_nsec / 1000;
            }
        } else {
            tv[0].tv_sec = times[0].tv_sec;
            tv[0].tv_usec = times[0].tv_nsec / 1000;
            tv[1].tv_sec = times[1].tv_sec;
            tv[1].tv_usec = times[1].tv_nsec / 1000;
        }
        return utimes(fdpath, tv);
    } else {
        /* NULL times means set to current time */
        return utimes(fdpath, NULL);
    }
}

/*
 * stpcpy - copy a string, returning a pointer to its end
 */
char *stpcpy(char *dest, const char *src) {
    while ((*dest = *src) != '\0') {
        dest++;
        src++;
    }
    return dest;
}

/*
 * stpncpy - copy a fixed-size string, returning a pointer to its end
 */
char *stpncpy(char *dest, const char *src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    if (i < n) {
        /* Pad with zeros */
        char *end = dest + i;
        for (; i < n; i++) {
            dest[i] = '\0';
        }
        return end;
    }
    return dest + n;
}
