/* sys/time.h - wrapper for IRIX sys/time.h
 *
 * This wrapper just passes through to the real IRIX header.
 * The struct timezone and struct timeval definitions come from the IRIX
 * header based on _SGIAPI/_BSD_TYPES which are set when _SGI_SOURCE is defined.
 */
#ifndef _CLANG_COMPAT_SYS_TIME_H
#define _CLANG_COMPAT_SYS_TIME_H

#include_next <sys/time.h>

/* timersub - BSD macro not available on IRIX */
#ifndef timersub
#define timersub(tvp, uvp, vvp) do { \
    (vvp)->tv_sec = (tvp)->tv_sec - (uvp)->tv_sec; \
    (vvp)->tv_usec = (tvp)->tv_usec - (uvp)->tv_usec; \
    if ((vvp)->tv_usec < 0) { (vvp)->tv_sec--; (vvp)->tv_usec += 1000000; } \
} while (0)
#endif

/* timeradd - BSD macro not available on IRIX */
#ifndef timeradd
#define timeradd(tvp, uvp, vvp) do { \
    (vvp)->tv_sec = (tvp)->tv_sec + (uvp)->tv_sec; \
    (vvp)->tv_usec = (tvp)->tv_usec + (uvp)->tv_usec; \
    if ((vvp)->tv_usec >= 1000000) { (vvp)->tv_sec++; (vvp)->tv_usec -= 1000000; } \
} while (0)
#endif

/* timercmp - BSD macro not available on IRIX */
#ifndef timercmp
#define timercmp(tvp, uvp, cmp) \
    (((tvp)->tv_sec == (uvp)->tv_sec) ? \
     ((tvp)->tv_usec cmp (uvp)->tv_usec) : \
     ((tvp)->tv_sec cmp (uvp)->tv_sec))
#endif

/* timerclear/timerisset - BSD macros */
#ifndef timerclear
#define timerclear(tvp) ((tvp)->tv_sec = (tvp)->tv_usec = 0)
#endif
#ifndef timerisset
#define timerisset(tvp) ((tvp)->tv_sec || (tvp)->tv_usec)
#endif

#endif /* _CLANG_COMPAT_SYS_TIME_H */
