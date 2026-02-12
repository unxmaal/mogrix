/* open_memstream.c - provide open_memstream() for IRIX
 *
 * IRIX 6.5 lacks open_memstream() (POSIX.1-2008).
 * Implements it using funopen() (BSD cookie I/O) which mogrix-compat provides.
 *
 * The returned FILE* writes to a dynamically-growing memory buffer.
 * After fclose(), *bufp points to the buffer and *sizep holds the length.
 * The caller must free(*bufp).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

struct memstream {
    char **bufp;
    size_t *sizep;
    char *buf;
    size_t len;
    size_t cap;
    size_t pos;
};

static int
ms_grow(struct memstream *ms, size_t need)
{
    size_t newcap;
    char *newbuf;

    if (need <= ms->cap)
        return 0;

    newcap = ms->cap ? ms->cap : 128;
    while (newcap < need)
        newcap *= 2;

    newbuf = realloc(ms->buf, newcap);
    if (newbuf == NULL)
        return -1;

    /* Zero newly allocated region */
    memset(newbuf + ms->cap, 0, newcap - ms->cap);
    ms->buf = newbuf;
    ms->cap = newcap;

    /* Update caller's pointer (POSIX requires this after each write) */
    *ms->bufp = ms->buf;
    return 0;
}

static int
ms_write(void *cookie, const char *buf, int nbytes)
{
    struct memstream *ms = cookie;
    size_t need;

    if (nbytes <= 0)
        return 0;

    need = ms->pos + (size_t)nbytes + 1;  /* +1 for NUL */
    if (ms_grow(ms, need) < 0)
        return -1;

    memcpy(ms->buf + ms->pos, buf, (size_t)nbytes);
    ms->pos += (size_t)nbytes;

    if (ms->pos > ms->len)
        ms->len = ms->pos;

    ms->buf[ms->len] = '\0';
    *ms->sizep = ms->len;
    *ms->bufp = ms->buf;

    return nbytes;
}

static off_t
ms_seek(void *cookie, off_t offset, int whence)
{
    struct memstream *ms = cookie;
    size_t newpos;

    switch (whence) {
    case SEEK_SET:
        newpos = (size_t)offset;
        break;
    case SEEK_CUR:
        newpos = ms->pos + (size_t)offset;
        break;
    case SEEK_END:
        newpos = ms->len + (size_t)offset;
        break;
    default:
        errno = EINVAL;
        return -1;
    }

    ms->pos = newpos;
    return (off_t)ms->pos;
}

static int
ms_close(void *cookie)
{
    struct memstream *ms = cookie;

    /* Ensure NUL termination */
    if (ms_grow(ms, ms->len + 1) == 0)
        ms->buf[ms->len] = '\0';

    *ms->bufp = ms->buf;
    *ms->sizep = ms->len;

    free(ms);
    return 0;
}

/* Forward declaration - funopen provided by mogrix-compat */
typedef int (*funopen_readfn)(void *, char *, int);
typedef int (*funopen_writefn)(void *, const char *, int);
typedef off_t (*funopen_seekfn)(void *, off_t, int);
typedef int (*funopen_closefn)(void *);
FILE *funopen(const void *, funopen_readfn, funopen_writefn,
              funopen_seekfn, funopen_closefn);

FILE *
open_memstream(char **bufp, size_t *sizep)
{
    struct memstream *ms;
    FILE *fp;

    if (bufp == NULL || sizep == NULL) {
        errno = EINVAL;
        return NULL;
    }

    ms = calloc(1, sizeof(*ms));
    if (ms == NULL)
        return NULL;

    ms->bufp = bufp;
    ms->sizep = sizep;

    /* Allocate initial buffer */
    ms->cap = 128;
    ms->buf = calloc(1, ms->cap);
    if (ms->buf == NULL) {
        free(ms);
        return NULL;
    }

    *bufp = ms->buf;
    *sizep = 0;

    fp = funopen(ms, NULL, ms_write, ms_seek, ms_close);
    if (fp == NULL) {
        free(ms->buf);
        free(ms);
        return NULL;
    }

    return fp;
}
