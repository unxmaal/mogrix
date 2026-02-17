/* sys/time.h - wrapper for IRIX sys/time.h
 *
 * This wrapper passes through to the real IRIX header with a fix:
 * IRIX sys/time.h provides a static select() wrapper around __xpg4_select()
 * when _XOPEN4UX/_XOPEN5 is active (which it is under _SGI_SOURCE).
 * The static function emits a symbol in every .o that includes this header.
 * When these .o files are archived into a static library, lld reports
 * duplicate symbols. Fix: force the extern declaration path by setting
 * _NO_XOPEN4=1 _NO_XOPEN5=1 before including the real header.
 */
#ifndef _CLANG_COMPAT_SYS_TIME_H
#define _CLANG_COMPAT_SYS_TIME_H

/* Include <time.h> FIRST to ensure struct timespec is properly defined.
 * The _NO_XOPEN5 workaround below prevents time_core.h from including
 * sys/timespec.h. Pre-including <time.h> sets __TIME_H__ which makes
 * sys/timespec.h define struct timespec under the correct name. */
#include <time.h>

/* Save and override XOPEN flags to get extern select() instead of static */
#pragma push_macro("_NO_XOPEN4")
#pragma push_macro("_NO_XOPEN5")
#undef _NO_XOPEN4
#define _NO_XOPEN4 1
#undef _NO_XOPEN5
#define _NO_XOPEN5 1

#include_next <sys/time.h>

#pragma pop_macro("_NO_XOPEN4")
#pragma pop_macro("_NO_XOPEN5")

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
