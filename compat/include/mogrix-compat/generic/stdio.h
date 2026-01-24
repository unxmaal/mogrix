/*
 * mogrix-compat/generic/stdio.h
 *
 * Wrapper that includes the real stdio.h and adds GNU extensions
 * for IRIX compatibility.
 */

#ifndef _MOGRIX_COMPAT_STDIO_H
#define _MOGRIX_COMPAT_STDIO_H

/* Include the real IRIX stdio.h */
#include_next <stdio.h>

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * fopencookie - open a custom stream (glibc extension)
 *
 * This extension allows creating FILE streams backed by
 * user-defined I/O functions. Required by libsolv.
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

#ifdef __cplusplus
}
#endif

#endif /* _MOGRIX_COMPAT_STDIO_H */
