#ifndef _DICL_CLANG_COMPAT_SYS_TERMIO_H
#define _DICL_CLANG_COMPAT_SYS_TERMIO_H

/*
 * IRIX sys/termio.h wrapper for clang.
 *
 * IRIX's termio.h uses CTRL() macro but doesn't define it.
 * Define it before including the IRIX header.
 */

#ifndef CTRL
#define CTRL(x) ((x) & 037)
#endif

#include_next <sys/termio.h>

#endif /* _DICL_CLANG_COMPAT_SYS_TERMIO_H */
