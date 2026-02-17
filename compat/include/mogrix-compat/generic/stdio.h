/*
 * mogrix-compat/generic/stdio.h
 *
 * Wrapper that includes the real stdio.h and adds GNU extensions
 * for IRIX compatibility.
 *
 * IRIX gates C99 stdio declarations (snprintf, vfscanf, etc.) behind
 * _SGIAPI and _NO_ANSIMODE expression-macros that use defined() in
 * expansion (UB in clang). Force the gates open so IRIX always provides
 * these declarations.
 */

#ifndef _MOGRIX_COMPAT_STDIO_H
#define _MOGRIX_COMPAT_STDIO_H

/*
 * Force IRIX stdio_core.h to expose C99 declarations.
 * Pre-include standards.h so its include guard prevents it from
 * re-defining _SGIAPI inside #include_next.
 *
 * IMPORTANT: Do NOT force __c99 here — IRIX wchar.h includes stdio.h
 * transitively, and __c99=1 would cause dual vsscanf declarations
 * (char* vs va_list) in stdio_core.h to conflict.
 */
#include <standards.h>

#pragma push_macro("_SGIAPI")
#pragma push_macro("_NO_ANSIMODE")
#undef _SGIAPI
#define _SGIAPI 1
#undef _NO_ANSIMODE
#define _NO_ANSIMODE 1

#include_next <stdio.h>

#pragma pop_macro("_NO_ANSIMODE")
#pragma pop_macro("_SGIAPI")

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
 * snprintf/vsnprintf/vfscanf/vscanf/vsscanf are now provided by IRIX
 * stdio_core.h via the forced _SGIAPI=1 gate above.
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
 * C++ va_list compatibility note:
 *
 * IRIX stdio declares vfprintf/vsnprintf/vsprintf/vprintf with 'char *'
 * as the va_list parameter type. Our stdarg.h now defines va_list as
 * 'char *' in C++ mode (matching IRIX), so no overloads are needed.
 * The previous __builtin_va_list overloads caused ambiguity when code
 * took the address of these functions (e.g. groff's configure).
 */

#endif /* _MOGRIX_COMPAT_STDIO_H */
