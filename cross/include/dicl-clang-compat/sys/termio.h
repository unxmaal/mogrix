/* sys/termio.h - wrapper for IRIX sys/termio.h
 *
 * IRIX's termio.h uses CTRL() macro but doesn't define it.
 * Define it before including the IRIX header.
 */
#ifndef _CLANG_COMPAT_SYS_TERMIO_H
#define _CLANG_COMPAT_SYS_TERMIO_H

#ifndef CTRL
#define CTRL(x) ((x) & 037)
#endif

#include_next <sys/termio.h>

#endif /* _CLANG_COMPAT_SYS_TERMIO_H */
