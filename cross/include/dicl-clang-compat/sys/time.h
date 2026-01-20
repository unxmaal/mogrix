/* sys/time.h - wrapper for IRIX sys/time.h
 *
 * This wrapper just passes through to the real IRIX header.
 * The struct timezone and struct timeval definitions come from the IRIX
 * header based on _SGIAPI/_BSD_TYPES which are set when _SGI_SOURCE is defined.
 */
#ifndef _CLANG_COMPAT_SYS_TIME_H
#define _CLANG_COMPAT_SYS_TIME_H

#include_next <sys/time.h>

#endif /* _CLANG_COMPAT_SYS_TIME_H */
