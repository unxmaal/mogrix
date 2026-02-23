/* shm_open.c - POSIX shared memory for IRIX 6.5
 *
 * IRIX libc's shm_open() literally calls open() on the name as-is,
 * meaning shm_open("/foo") creates a file at the root directory "/foo".
 * Non-root users get EACCES because they can't write to /.
 *
 * This compat version prepends /tmp to the name, so:
 *   shm_open("/WK2SharedMemory.abc") -> open("/tmp/WK2SharedMemory.abc")
 *   shm_open("WK2SharedMemory.abc")  -> open("/tmp/WK2SharedMemory.abc")
 *
 * Must be in libmogrix_compat.so (preloaded via _RLDN32_LIST) to
 * override the libc version for calls from shared libraries.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

int
shm_open(const char *name, int oflag, mode_t mode)
{
    char path[1024];
    const char *p;
    int fd;

    if (name == NULL) {
        errno = EINVAL;
        return -1;
    }

    /* Strip leading slash if present (POSIX names start with /) */
    p = (name[0] == '/') ? name + 1 : name;
    if (p[0] == '\0') {
        errno = EINVAL;
        return -1;
    }

    /* Build /tmp/<name> path */
    if (snprintf(path, sizeof(path), "/tmp/.shm_%s", p) >= (int)sizeof(path)) {
        errno = ENAMETOOLONG;
        return -1;
    }

    fd = open(path, oflag, mode);
    if (fd >= 0) {
        /* Set close-on-exec */
        fcntl(fd, F_SETFD, FD_CLOEXEC);
    }
    return fd;
}

int
shm_unlink(const char *name)
{
    char path[1024];
    const char *p;

    if (name == NULL) {
        errno = EINVAL;
        return -1;
    }

    p = (name[0] == '/') ? name + 1 : name;
    if (p[0] == '\0') {
        errno = EINVAL;
        return -1;
    }

    if (snprintf(path, sizeof(path), "/tmp/.shm_%s", p) >= (int)sizeof(path)) {
        errno = ENAMETOOLONG;
        return -1;
    }

    return unlink(path);
}
