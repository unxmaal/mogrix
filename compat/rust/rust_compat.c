/*
 * rust_compat.c — Stub implementations for functions missing from IRIX libc
 * that the Rust standard library requires.
 *
 * These are linked into Rust binaries via librust_irix_compat.a
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

/* dirfd — return file descriptor associated with directory stream.
 * IRIX doesn't have dirfd() but the DIR struct has dd_fd. */
int dirfd(DIR *dirp) {
    if (!dirp) {
        errno = EINVAL;
        return -1;
    }
    /* IRIX DIR struct has dd_fd as the file descriptor */
    return dirp->dd_fd;
}

/* fdopendir — open directory from file descriptor.
 * Minimal implementation: uses /proc/self/fd/N to get path, then opendir().
 * Fallback: not easily implementable on IRIX without /proc.
 * For Rust std, this is used in fs operations. We provide a stub that
 * uses fchdir + opendir(".") pattern. */
DIR *fdopendir(int fd) {
    int saved_cwd = open(".", O_RDONLY);
    if (saved_cwd < 0) return NULL;

    if (fchdir(fd) < 0) {
        close(saved_cwd);
        return NULL;
    }

    DIR *dir = opendir(".");

    /* Restore original working directory */
    fchdir(saved_cwd);
    close(saved_cwd);

    return dir;
}

/* pthread_condattr_setclock — set clock for condition variable.
 * IRIX pthreads don't support clock selection for condition variables.
 * Accept CLOCK_REALTIME (1), reject others. */
int pthread_condattr_setclock(pthread_condattr_t *attr, int clock_id) {
    (void)attr;
    if (clock_id == 1) { /* CLOCK_REALTIME */
        return 0;
    }
    /* IRIX doesn't support CLOCK_MONOTONIC for condvars */
    errno = EINVAL;
    return EINVAL;
}

/* preadv/pwritev — IRIX doesn't have these. Emulate with pread/pwrite loops. */
#include <sys/uio.h>
#include <unistd.h>

ssize_t preadv(int fd, const struct iovec *iov, int iovcnt, off_t offset) {
    ssize_t total = 0;
    for (int i = 0; i < iovcnt; i++) {
        ssize_t n = pread(fd, iov[i].iov_base, iov[i].iov_len, offset);
        if (n < 0) return n;
        total += n;
        offset += n;
        if ((size_t)n < iov[i].iov_len) break;
    }
    return total;
}

ssize_t pwritev(int fd, const struct iovec *iov, int iovcnt, off_t offset) {
    ssize_t total = 0;
    for (int i = 0; i < iovcnt; i++) {
        ssize_t n = pwrite(fd, iov[i].iov_base, iov[i].iov_len, offset);
        if (n < 0) return n;
        total += n;
        offset += n;
        if ((size_t)n < iov[i].iov_len) break;
    }
    return total;
}

/* dup3 — IRIX doesn't have this. Emulate with dup2 + fcntl. */
int dup3(int oldfd, int newfd, int flags) {
    if (oldfd == newfd) { errno = EINVAL; return -1; }
    int fd = dup2(oldfd, newfd);
    if (fd < 0) return fd;
    if (flags & 0x80000) { /* O_CLOEXEC */
        fcntl(fd, F_SETFD, FD_CLOEXEC);
    }
    return fd;
}

/* posix_fadvise — IRIX doesn't have this. No-op stub. */
int posix_fadvise(int fd, off_t offset, off_t len, int advice) {
    (void)fd; (void)offset; (void)len; (void)advice;
    return 0; /* Success — silently ignore advice */
}

/* posix_fallocate — IRIX doesn't have this. Emulate with write. */
int posix_fallocate(int fd, off_t offset, off_t len) {
    /* Simple fallback: seek to end and write zeros if needed */
    struct stat st;
    if (fstat(fd, &st) < 0) return errno;
    if (st.st_size >= offset + len) return 0;
    /* Write a zero byte at the end to extend the file */
    off_t end = offset + len - 1;
    char zero = 0;
    if (pwrite(fd, &zero, 1, end) < 0) return errno;
    return 0;
}

/* cfmakeraw — sets terminal to raw mode. Missing from IRIX libc. */
#include <termios.h>
void cfmakeraw(struct termios *t) {
    t->c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    t->c_oflag &= ~OPOST;
    t->c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    t->c_cflag &= ~(CSIZE | PARENB);
    t->c_cflag |= CS8;
    t->c_cc[VMIN] = 1;
    t->c_cc[VTIME] = 0;
}

/* sync — IRIX has this in libc, but declare for safety */
/* void sync(void); — should be in IRIX libc, no stub needed */

/* flock — IRIX may not have BSD flock, emulate with fcntl */
int flock(int fd, int operation) {
    struct flock fl;
    fl.l_whence = 0; /* SEEK_SET */
    fl.l_start = 0;
    fl.l_len = 0;
    fl.l_type = (operation & 1) ? F_RDLCK : /* LOCK_SH */
                (operation & 2) ? F_WRLCK : /* LOCK_EX */
                F_UNLCK;                    /* LOCK_UN */
    return fcntl(fd, (operation & 4) ? F_SETLK : F_SETLKW, &fl); /* LOCK_NB */
}
