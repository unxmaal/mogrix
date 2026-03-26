/* ifaddrs.h stub for IRIX — getifaddrs() not available */
#ifndef _IFADDRS_H_IRIX_STUB
#define _IFADDRS_H_IRIX_STUB

#include <sys/socket.h>

struct ifaddrs {
    struct ifaddrs  *ifa_next;
    char            *ifa_name;
    unsigned int     ifa_flags;
    struct sockaddr *ifa_addr;
    struct sockaddr *ifa_netmask;
    struct sockaddr *ifa_broadaddr;
    void            *ifa_data;
};

static inline int getifaddrs(struct ifaddrs **ifap) {
    *ifap = NULL;
    return -1;  /* not supported */
}

static inline void freeifaddrs(struct ifaddrs *ifa) {
    (void)ifa;
}

#endif /* _IFADDRS_H_IRIX_STUB */
