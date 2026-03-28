/*
 * irix_posix_compat.h — POSIX.1-2008 stubs for IRIX 6.5
 *
 * Provides utimensat(), openat(), fdopendir(), unlinkat() stubs
 * for libc++ filesystem support. IRIX lacks these functions.
 *
 * Included via -include in build-libcxx.sh CXXFLAGS.
 */
#ifndef _IRIX_POSIX_COMPAT_H
#define _IRIX_POSIX_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

/* utimensat: falls back to utimes() when fd==AT_FDCWD */
#ifndef _IRIX_UTIMENSAT_DEFINED
#define _IRIX_UTIMENSAT_DEFINED
#include <sys/time.h>
extern int utimes(const char *, const struct timeval [2]);
static inline int utimensat(int __fd, const char *__path,
                             const struct timespec __times[2], int __flags) {
    (void)__flags;
    if (__fd != -2 /* AT_FDCWD */) return -1;
    if (!__times) {
        /* NULL means set to current time */
        return utimes(__path, (const struct timeval *)0);
    }
    struct timeval tv[2];
    tv[0].tv_sec = __times[0].tv_sec;
    tv[0].tv_usec = __times[0].tv_nsec / 1000;
    tv[1].tv_sec = __times[1].tv_sec;
    tv[1].tv_usec = __times[1].tv_nsec / 1000;
    return utimes(__path, tv);
}
#endif /* _IRIX_UTIMENSAT_DEFINED */

/* openat: IRIX lacks this. Stub for AT_FDCWD only — falls back to open(). */
#ifndef _IRIX_OPENAT_DEFINED
#define _IRIX_OPENAT_DEFINED
#include <fcntl.h>
static inline int openat(int __fd, const char *__path, int __oflag, ...) {
    if (__fd != -2 /* AT_FDCWD */) return -1;
    return open(__path, __oflag);
}
#endif

/* fdopendir: IRIX lacks this. Not directly implementable without /proc/self/fd.
   Stub that always fails — filesystem remove_all will fall back. */
#ifndef _IRIX_FDOPENDIR_DEFINED
#define _IRIX_FDOPENDIR_DEFINED
#include <dirent.h>
static inline DIR *fdopendir(int __fd) {
    (void)__fd;
    return (DIR *)0; /* always fails */
}
#endif

/* unlinkat: IRIX lacks this. Stub for AT_FDCWD only. */
#ifndef _IRIX_UNLINKAT_DEFINED
#define _IRIX_UNLINKAT_DEFINED
#include <unistd.h>
static inline int unlinkat(int __fd, const char *__path, int __flag) {
    if (__fd != -2 /* AT_FDCWD */) return -1;
    if (__flag & 1 /* AT_REMOVEDIR */)
        return rmdir(__path);
    return unlink(__path);
}
#endif

#ifdef __cplusplus
}
#endif

#endif /* _IRIX_POSIX_COMPAT_H */
