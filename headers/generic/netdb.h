#ifndef _DICL_CLANG_COMPAT_NETDB_H
#define _DICL_CLANG_COMPAT_NETDB_H

/*
 * IRIX netdb.h wrapper for clang.
 *
 * struct addrinfo and getaddrinfo/freeaddrinfo are only visible when
 * _NO_XOPEN4 && _NO_XOPEN5, which conflicts with _XOPEN_SOURCE.
 * Define these manually when needed.
 */

#include_next <netdb.h>

/* Expose struct addrinfo if not already visible */
#if !defined(_NO_XOPEN4) || !defined(_NO_XOPEN5)
#ifndef _STRUCT_ADDRINFO_DEFINED
#define _STRUCT_ADDRINFO_DEFINED

struct addrinfo {
    int     ai_flags;       /* AI_PASSIVE, AI_CANONNAME, AI_NUMERICHOST, etc. */
    int     ai_family;      /* PF_xxx */
    int     ai_socktype;    /* SOCK_xxx */
    int     ai_protocol;    /* 0 or IPPROTO_xxx for IPv4 and IPv6 */
    socklen_t ai_addrlen;   /* length of ai_addr */
    char    *ai_canonname;  /* canonical name for hostname */
    struct sockaddr *ai_addr;      /* binary address */
    struct addrinfo *ai_next;      /* next structure in linked list */
};

/* Function declarations */
extern int getaddrinfo(const char *, const char *,
                       const struct addrinfo *, struct addrinfo **);
extern void freeaddrinfo(struct addrinfo *);
extern const char *gai_strerror(int);
extern int getnameinfo(const struct sockaddr *, socklen_t, char *,
                       socklen_t, char *, socklen_t, unsigned int);

#endif /* _STRUCT_ADDRINFO_DEFINED */
#endif /* !_NO_XOPEN4 || !_NO_XOPEN5 */

#endif /* _DICL_CLANG_COMPAT_NETDB_H */
