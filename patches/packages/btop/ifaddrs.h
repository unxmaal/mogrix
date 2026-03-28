/*
 * ifaddrs.h — getifaddrs() implementation for IRIX 6.5
 *
 * IRIX lacks getifaddrs(). This implements it using SIOCGIFCONF/SIOCGIFADDR/
 * SIOCGIFFLAGS ioctls, which IRIX does support.
 *
 * Provides: getifaddrs(), freeifaddrs()
 */
#ifndef _IFADDRS_H_IRIX
#define _IFADDRS_H_IRIX

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/soioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

struct ifaddrs {
    struct ifaddrs  *ifa_next;
    char            *ifa_name;
    unsigned int     ifa_flags;
    struct sockaddr *ifa_addr;
    struct sockaddr *ifa_netmask;
    struct sockaddr *ifa_broadaddr;
    void            *ifa_data;
};

static inline void freeifaddrs(struct ifaddrs *ifa) {
    while (ifa) {
        struct ifaddrs *next = ifa->ifa_next;
        free(ifa->ifa_name);
        free(ifa->ifa_addr);
        free(ifa->ifa_netmask);
        free(ifa);
        ifa = next;
    }
}

static inline int getifaddrs(struct ifaddrs **ifap) {
    *ifap = NULL;

    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) return -1;

    /* Get interface list via SIOCGIFCONF */
    struct ifconf ifc;
    char buf[4096];
    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;

    if (ioctl(fd, SIOCGIFCONF, &ifc) < 0) {
        close(fd);
        return -1;
    }

    struct ifaddrs *head = NULL;
    struct ifaddrs *tail = NULL;

    /* Walk the ifreq array returned by SIOCGIFCONF */
    char *ptr = ifc.ifc_buf;
    char *end = ptr + ifc.ifc_len;

    while (ptr < end) {
        struct ifreq *ifr = (struct ifreq *)ptr;

        /* Advance pointer — IRIX uses fixed-size ifreq entries */
        ptr += sizeof(struct ifreq);

        /* Allocate ifaddrs entry */
        struct ifaddrs *ifa = (struct ifaddrs *)calloc(1, sizeof(struct ifaddrs));
        if (!ifa) { freeifaddrs(head); close(fd); return -1; }

        /* Interface name */
        ifa->ifa_name = strdup(ifr->ifr_name);

        /* Get flags */
        struct ifreq freq;
        memset(&freq, 0, sizeof(freq));
        strncpy(freq.ifr_name, ifr->ifr_name, IFNAMSIZ - 1);
        if (ioctl(fd, SIOCGIFFLAGS, &freq) == 0) {
            ifa->ifa_flags = freq.ifr_flags;
        }

        /* Copy address from SIOCGIFCONF result */
        if (ifr->ifr_addr.sa_family == AF_INET) {
            ifa->ifa_addr = (struct sockaddr *)malloc(sizeof(struct sockaddr_in));
            if (ifa->ifa_addr)
                memcpy(ifa->ifa_addr, &ifr->ifr_addr, sizeof(struct sockaddr_in));
        }

        /* Get netmask */
        memset(&freq, 0, sizeof(freq));
        strncpy(freq.ifr_name, ifr->ifr_name, IFNAMSIZ - 1);
        if (ioctl(fd, SIOCGIFNETMASK, &freq) == 0) {
            ifa->ifa_netmask = (struct sockaddr *)malloc(sizeof(struct sockaddr_in));
            if (ifa->ifa_netmask)
                memcpy(ifa->ifa_netmask, &freq.ifr_addr, sizeof(struct sockaddr_in));
        }

        /* Append to linked list */
        ifa->ifa_next = NULL;
        if (tail) {
            tail->ifa_next = ifa;
            tail = ifa;
        } else {
            head = tail = ifa;
        }
    }

    close(fd);
    *ifap = head;
    return 0;
}

#endif /* _IFADDRS_H_IRIX */
