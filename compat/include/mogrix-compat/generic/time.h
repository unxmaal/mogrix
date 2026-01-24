/*
 * mogrix-compat/generic/time.h
 *
 * Wrapper that includes the real time.h and adds GNU extensions
 * for IRIX compatibility.
 */

#ifndef _MOGRIX_COMPAT_TIME_H
#define _MOGRIX_COMPAT_TIME_H

/* Include the real IRIX time.h */
#include_next <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * timegm - Convert broken-down time to time_t in UTC (GNU extension)
 *
 * IRIX doesn't have timegm. We provide our own implementation.
 * This is the inverse of gmtime().
 */
#ifndef timegm
time_t timegm(struct tm *tm);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _MOGRIX_COMPAT_TIME_H */
