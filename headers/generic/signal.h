#ifndef _DICL_CLANG_COMPAT_SIGNAL_H
#define _DICL_CLANG_COMPAT_SIGNAL_H

/*
 * IRIX signal.h wrapper for clang.
 * Pass through to the IRIX header - sigset_t is defined in IRIX headers.
 */

#include_next <signal.h>

#endif /* _DICL_CLANG_COMPAT_SIGNAL_H */
