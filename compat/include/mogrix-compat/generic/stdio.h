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
 * funopen - open a custom stream (BSD extension)
 *
 * This extension allows creating FILE streams backed by
 * user-defined I/O functions. Required by libsolv.
 * Simpler interface than fopencookie - uses separate function pointers
 * instead of a struct.
 */
#ifndef _MOGRIX_FUNOPEN_DECL
#define _MOGRIX_FUNOPEN_DECL

typedef int (*funopen_readfn)(void *cookie, char *buf, int nbytes);
typedef int (*funopen_writefn)(void *cookie, const char *buf, int nbytes);
typedef off_t (*funopen_seekfn)(void *cookie, off_t offset, int whence);
typedef int (*funopen_closefn)(void *cookie);

FILE *funopen(const void *cookie,
              funopen_readfn readfn,
              funopen_writefn writefn,
              funopen_seekfn seekfn,
              funopen_closefn closefn);

#endif /* _MOGRIX_FUNOPEN_DECL */

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

/*
 * dprintf/vdprintf - formatted print to file descriptor (POSIX.1-2008)
 *
 * IRIX doesn't have dprintf. We provide our own implementation.
 */
int dprintf(int fd, const char *fmt, ...);
int vdprintf(int fd, const char *fmt, va_list ap);

/*
 * open_memstream - open a dynamic memory buffer as a stream (POSIX.1-2008)
 *
 * IRIX doesn't have open_memstream. Implementation uses funopen().
 */
FILE *open_memstream(char **bufp, size_t *sizep);

#ifdef __cplusplus
}
#endif

/*
 * C++ va_list compatibility fix for IRIX
 *
 * IRIX stdio declares vfprintf/vsnprintf/vsprintf/vprintf with 'char *'
 * as the va_list parameter type (IRIX native va_list IS char*).
 * Clang's __builtin_va_list is 'void *' on MIPS.
 * In C, void* implicitly converts to char*. In C++, it does NOT.
 *
 * Fix: provide C++ function OVERLOADS (not macros!) that accept
 * __builtin_va_list. Macros don't work here because libstdc++ <cstdio>
 * does #undef vfprintf/vsnprintf/vsprintf/vprintf. Function overloads
 * survive #undef and participate in C++ overload resolution.
 *
 * When code calls vfprintf(f, fmt, ap) where ap is va_list (void*),
 * overload resolution prefers our exact-match overload over the IRIX
 * char* version (which would require an illegal void*->char* conversion).
 */
#ifdef __cplusplus

static inline int vfprintf(FILE *__f, const char *__fmt, __builtin_va_list __ap) {
    return ::vfprintf(__f, __fmt, (char *)__ap);
}

static inline int vsnprintf(char *__s, size_t __n, const char *__fmt, __builtin_va_list __ap) {
    return ::vsnprintf(__s, __n, __fmt, (char *)__ap);
}

static inline int vsprintf(char *__s, const char *__fmt, __builtin_va_list __ap) {
    return ::vsprintf(__s, __fmt, (char *)__ap);
}

static inline int vprintf(const char *__fmt, __builtin_va_list __ap) {
    return ::vprintf(__fmt, (char *)__ap);
}

#endif /* __cplusplus */

#endif /* _MOGRIX_COMPAT_STDIO_H */
