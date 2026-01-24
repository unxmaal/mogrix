/*
 * Portable fopencookie() implementation for IRIX
 *
 * Based on portable-fopencookie by zhasha & nsz (musl community)
 * Modified for IRIX by mogrix project
 *
 * This provides a limited fopencookie() for systems that lack it.
 * Limitations:
 *   - No seeking support
 *   - No r+/w+ support
 *   - Callbacks run in a separate thread
 *
 * Licensed under 3-clause BSD license.
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <errno.h>

/*
 * Include our stdio.h compat wrapper which provides the fopencookie
 * types and declaration. The compat wrapper uses _MOGRIX_FOPENCOOKIE_DECL
 * to guard against double-definition.
 */
#ifndef _MOGRIX_FOPENCOOKIE_DECL
#define _MOGRIX_FOPENCOOKIE_DECL

typedef struct cookie_io_functions_t {
    ssize_t (*read)(void *, char *, size_t);
    ssize_t (*write)(void *, const char *, size_t);
    int (*seek)(void *, off_t *, int);
    int (*close)(void *);
} cookie_io_functions_t;

FILE *fopencookie(void *cookie, const char *mode, cookie_io_functions_t io);

#endif /* _MOGRIX_FOPENCOOKIE_DECL */

struct ctx {
    int fd;
    void *cookie;
    cookie_io_functions_t io;
    char buf[1024];
};

static void *proxy(void *arg)
{
    struct ctx *ctx = arg;
    ssize_t r;
    size_t n, k;

    pthread_detach(pthread_self());

    while (1) {
        r = ctx->io.read ?
            (ctx->io.read)(ctx->cookie, ctx->buf, sizeof(ctx->buf)) :
            read(ctx->fd, ctx->buf, sizeof(ctx->buf));
        if (r < 0) {
            if (errno != EINTR) { break; }
            continue;
        }
        if (r == 0) { break; }

        n = r;
        k = 0;
        while (n > 0) {
            r = ctx->io.write ?
                (ctx->io.write)(ctx->cookie, ctx->buf + k, n) :
                write(ctx->fd, ctx->buf + k, n);
            if (r < 0) {
                if (errno != EINTR) { break; }
                continue;
            }
            if (r == 0) { break; }

            n -= r;
            k += r;
        }
        if (n > 0) { break; }
    }

    if (ctx->io.close) { (ctx->io.close)(ctx->cookie); }
    close(ctx->fd);
    free(ctx);
    return NULL;
}

FILE *fopencookie(void *cookie, const char *mode, cookie_io_functions_t io)
{
    struct ctx *ctx = NULL;
    int rd = 0, wr = 0;
    int p[2] = { -1, -1 };
    FILE *f = NULL;
    pthread_t dummy;

    switch (mode[0]) {
        case 'a':
        case 'w': wr = 1; break;
        case 'r': rd = 1; break;
        default:
            errno = EINVAL;
            return NULL;
    }
    switch (mode[1]) {
        case '\0': break;
        case '+':
            if (mode[2] == '\0') {
                errno = ENOTSUP;
                return NULL;
            }
            /* fall through */
        default:
            errno = EINVAL;
            return NULL;
    }
    if (io.seek) {
        errno = ENOTSUP;
        return NULL;
    }

    ctx = malloc(sizeof(*ctx));
    if (!ctx) { return NULL; }

    /* Use pipe() + fcntl() instead of pipe2() for IRIX compatibility */
    if (pipe(p) != 0) { goto err; }
    /* Set close-on-exec for both ends */
    fcntl(p[0], F_SETFD, FD_CLOEXEC);
    fcntl(p[1], F_SETFD, FD_CLOEXEC);

    if ((f = fdopen(p[wr], mode)) == NULL) { goto err; }
    p[wr] = -1;
    ctx->fd = p[rd];
    ctx->cookie = cookie;
    ctx->io.read = rd ? io.read : NULL;
    ctx->io.write = wr ? io.write : NULL;
    ctx->io.seek = NULL;
    ctx->io.close = io.close;
    if (pthread_create(&dummy, NULL, proxy, ctx) != 0) { goto err; }

    return f;

err:
    if (p[0] >= 0) { close(p[0]); }
    if (p[1] >= 0) { close(p[1]); }
    if (f) { fclose(f); }
    free(ctx);
    return NULL;
}
