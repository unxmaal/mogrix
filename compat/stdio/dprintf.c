/* dprintf.c - provide dprintf() for IRIX
 *
 * IRIX 6.5 lacks dprintf() (POSIX.1-2008).
 * Implements it using fdopen() + vfprintf() + fflush().
 */

#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>

int
dprintf(int fd, const char *fmt, ...)
{
    va_list ap;
    FILE *fp;
    int ret;
    int dupfd;

    /* dup the fd so fclose() doesn't close the caller's descriptor */
    dupfd = dup(fd);
    if (dupfd < 0)
        return -1;

    fp = fdopen(dupfd, "w");
    if (fp == NULL) {
        close(dupfd);
        return -1;
    }

    va_start(ap, fmt);
    ret = vfprintf(fp, fmt, ap);
    va_end(ap);

    fflush(fp);
    fclose(fp);
    return ret;
}

int
vdprintf(int fd, const char *fmt, va_list ap)
{
    FILE *fp;
    int ret;
    int dupfd;

    dupfd = dup(fd);
    if (dupfd < 0)
        return -1;

    fp = fdopen(dupfd, "w");
    if (fp == NULL) {
        close(dupfd);
        return -1;
    }

    ret = vfprintf(fp, fmt, ap);
    fflush(fp);
    fclose(fp);
    return ret;
}
