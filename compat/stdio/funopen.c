/*
 * funopen() implementation for IRIX
 *
 * Based on libdicl's funopen_replacements.cpp by danielhams
 * Converted to pure C for mogrix project
 *
 * This provides BSD funopen() for systems that lack it.
 * Limitations:
 *   - No seeking support
 *   - No simultaneous read+write support
 *   - Callbacks run in a separate thread
 *
 * Licensed under LGPL 2.1 (same as libdicl)
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>

/* funopen signature (BSD style) */
typedef int (*funopen_readfn)(void *cookie, char *buf, int nbytes);
typedef int (*funopen_writefn)(void *cookie, const char *buf, int nbytes);
typedef off_t (*funopen_seekfn)(void *cookie, off_t offset, int whence);
typedef int (*funopen_closefn)(void *cookie);

FILE *funopen(const void *cookie,
              funopen_readfn readfn,
              funopen_writefn writefn,
              funopen_seekfn seekfn,
              funopen_closefn closefn);

struct funopen_ctx {
    void *cookie;
    funopen_readfn readfn;
    funopen_writefn writefn;
    funopen_closefn closefn;
    int fd;           /* pipe fd for the thread to use */
    int is_read;      /* 1 if read mode, 0 if write mode */
    char buf[1024];
};

static void *funopen_proxy(void *arg)
{
    struct funopen_ctx *ctx = (struct funopen_ctx *)arg;
    int r;
    int n, k;

    pthread_detach(pthread_self());

    while (1) {
        /* Read from either the callback (read mode) or the pipe (write mode) */
        if (ctx->is_read) {
            r = ctx->readfn(ctx->cookie, ctx->buf, sizeof(ctx->buf));
        } else {
            r = read(ctx->fd, ctx->buf, sizeof(ctx->buf));
        }

        if (r < 0) {
            if (errno == EINTR) continue;
            break;
        }
        if (r == 0) break;  /* EOF */

        n = r;
        k = 0;
        while (n > 0) {
            /* Write to either the pipe (read mode) or the callback (write mode) */
            if (ctx->is_read) {
                r = write(ctx->fd, ctx->buf + k, n);
            } else {
                r = ctx->writefn(ctx->cookie, ctx->buf + k, n);
            }

            if (r < 0) {
                if (errno == EINTR) continue;
                break;
            }
            if (r == 0) break;

            n -= r;
            k += r;
        }
        if (n > 0) break;  /* Write incomplete, exit */
    }

    /* Cleanup */
    if (ctx->closefn) {
        ctx->closefn(ctx->cookie);
    }
    close(ctx->fd);
    free(ctx);
    return NULL;
}

FILE *funopen(const void *cookie,
              funopen_readfn readfn,
              funopen_writefn writefn,
              funopen_seekfn seekfn,
              funopen_closefn closefn)
{
    struct funopen_ctx *ctx = NULL;
    int p[2] = { -1, -1 };
    FILE *f = NULL;
    pthread_t thread;
    const char *mode;
    int rd = 0, wr = 0;

    /* Seeking not supported */
    if (seekfn) {
        errno = ENOTSUP;
        return NULL;
    }

    /* Simultaneous read+write not supported */
    if (readfn && writefn) {
        errno = ENOTSUP;
        return NULL;
    }

    /* Determine mode */
    if (readfn) {
        mode = "r";
        rd = 1;
    } else if (writefn) {
        mode = "w";
        wr = 1;
    } else {
        errno = EINVAL;
        return NULL;
    }

    ctx = (struct funopen_ctx *)malloc(sizeof(*ctx));
    if (!ctx) {
        return NULL;
    }

    if (pipe(p) != 0) {
        free(ctx);
        return NULL;
    }

    /* Set close-on-exec for both ends */
    fcntl(p[0], F_SETFD, FD_CLOEXEC);
    fcntl(p[1], F_SETFD, FD_CLOEXEC);

    /*
     * For read mode:
     *   - Main thread reads from p[0] (read end of pipe)
     *   - Proxy thread reads from callback, writes to p[1] (write end)
     * For write mode:
     *   - Main thread writes to p[1] (write end of pipe)
     *   - Proxy thread reads from p[0] (read end), writes to callback
     */
    if (rd) {
        /* Read mode: main reads from p[0], thread writes to p[1] */
        f = fdopen(p[0], mode);
        if (!f) goto err;
        ctx->fd = p[1];
    } else {
        /* Write mode: main writes to p[1], thread reads from p[0] */
        f = fdopen(p[1], mode);
        if (!f) goto err;
        ctx->fd = p[0];
    }

    ctx->cookie = (void *)cookie;
    ctx->readfn = readfn;
    ctx->writefn = writefn;
    ctx->closefn = closefn;
    ctx->is_read = rd;

    if (pthread_create(&thread, NULL, funopen_proxy, ctx) != 0) {
        goto err;
    }

    /* Thread owns ctx now, don't free it in main thread */
    return f;

err:
    if (p[0] >= 0) close(p[0]);
    if (p[1] >= 0) close(p[1]);
    if (f) fclose(f);
    free(ctx);
    return NULL;
}
