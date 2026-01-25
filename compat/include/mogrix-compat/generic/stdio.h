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

/*
 * getline - read a line (POSIX.1-2008)
 * getdelim - read a delimited line (POSIX.1-2008)
 *
 * IRIX may not have these.
 */
#ifndef getline
ssize_t getline(char **lineptr, size_t *n, FILE *stream);
#endif

#ifndef getdelim
ssize_t getdelim(char **lineptr, size_t *n, int delim, FILE *stream);
#endif

/*
 * snprintf/vsnprintf - C99 formatted print with size limit
 *
 * IRIX libc has these declared in internal/stdio_core.h when
 * internal headers are included. We only declare them if not
 * already present.
 */
#include <stdarg.h>

/*
 * asprintf/vasprintf - formatted print to allocated string (GNU extension)
 *
 * IRIX doesn't have asprintf. We provide our own implementation.
 */
#ifndef asprintf
int asprintf(char **strp, const char *fmt, ...);
#endif

#ifndef vasprintf
int vasprintf(char **strp, const char *fmt, va_list ap);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _MOGRIX_COMPAT_STDIO_H */
