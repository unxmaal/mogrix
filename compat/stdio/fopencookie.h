/*
 * Portable fopencookie() header for IRIX
 *
 * Based on portable-fopencookie by zhasha & nsz (musl community)
 * Modified for IRIX by mogrix project
 *
 * Licensed under 3-clause BSD license.
 */

#ifndef MOGRIX_FOPENCOOKIE_H
#define MOGRIX_FOPENCOOKIE_H

#include <stdio.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cookie_io_functions_t {
    ssize_t (*read)(void *, char *, size_t);
    ssize_t (*write)(void *, const char *, size_t);
    int (*seek)(void *, off_t *, int);
    int (*close)(void *);
} cookie_io_functions_t;

FILE *fopencookie(void *cookie, const char *mode, cookie_io_functions_t io);

#ifdef __cplusplus
}
#endif

#endif /* MOGRIX_FOPENCOOKIE_H */
