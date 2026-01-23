/*
 * DICL clang compat sys/socket.h
 * Ensure struct msghdr has msg_flags member for modern socket APIs
 *
 * IRIX's default _SGIAPI msghdr lacks msg_control, msg_controllen, and msg_flags
 * which are required by OpenSSL's datagram BIO and other modern network code.
 *
 * Solution: Define _XOPEN_SOURCE to get the XPG4/XPG5 msghdr definition
 * which includes all the modern members.
 */
#ifndef _DICL_SYS_SOCKET_H
#define _DICL_SYS_SOCKET_H

/*
 * Temporarily undefine _SGI_SOURCE to avoid the old BSD msghdr
 * and get the XPG version with msg_control/msg_flags instead
 */
#ifdef _SGI_SOURCE
#undef _SGI_SOURCE
#define _DICL_RESTORE_SGI_SOURCE 1
#endif

#ifdef _SGIAPI
#undef _SGIAPI
#define _DICL_RESTORE_SGIAPI 1
#endif

/* Define _XOPEN_SOURCE to get modern msghdr */
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 500
#define _DICL_DEFINED_XOPEN_SOURCE 1
#endif

/* Include IRIX socket.h */
#include_next <sys/socket.h>

/* Restore original defines */
#ifdef _DICL_RESTORE_SGI_SOURCE
#define _SGI_SOURCE 1
#undef _DICL_RESTORE_SGI_SOURCE
#endif

#ifdef _DICL_RESTORE_SGIAPI
#define _SGIAPI 1
#undef _DICL_RESTORE_SGIAPI
#endif

#ifdef _DICL_DEFINED_XOPEN_SOURCE
#undef _XOPEN_SOURCE
#undef _DICL_DEFINED_XOPEN_SOURCE
#endif

#endif /* _DICL_SYS_SOCKET_H */
