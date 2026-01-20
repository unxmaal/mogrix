#ifndef _DICL_CLANG_COMPAT_ARPA_INET_H
#define _DICL_CLANG_COMPAT_ARPA_INET_H

/*
 * IRIX arpa/inet.h wrapper for clang.
 *
 * inet_aton is guarded by _SGIAPI which conflicts with _XOPEN_SOURCE.
 * Declare it manually after including the IRIX header.
 */

#include_next <arpa/inet.h>

/* Expose inet_aton if not already visible */
#ifndef _INET_ATON_DECLARED
#define _INET_ATON_DECLARED
extern int inet_aton(const char *, struct in_addr *);
#endif

#endif /* _DICL_CLANG_COMPAT_ARPA_INET_H */
