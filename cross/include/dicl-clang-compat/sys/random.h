/*
 * DICL clang compat sys/random.h
 * IRIX doesn't have getrandom() - provide stub that returns ENOSYS
 *
 * OpenSSL will fall back to /dev/urandom when getrandom() returns -1/ENOSYS.
 */
#ifndef _DICL_SYS_RANDOM_H
#define _DICL_SYS_RANDOM_H

#include <sys/types.h>
#include <errno.h>

/* Flags for getrandom() - define for source compatibility */
#define GRND_NONBLOCK   0x0001
#define GRND_RANDOM     0x0002

#ifdef __cplusplus
extern "C" {
#endif

/* getrandom() stub - always returns -1 with ENOSYS to force /dev/urandom fallback */
static __inline__ ssize_t getrandom(void *buf, size_t buflen, unsigned int flags)
{
    (void)buf;
    (void)buflen;
    (void)flags;
    errno = ENOSYS;
    return -1;
}

#ifdef __cplusplus
}
#endif

#endif /* _DICL_SYS_RANDOM_H */
