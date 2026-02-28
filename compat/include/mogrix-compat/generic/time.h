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
 *
 * Forward-declare struct tm at file scope to avoid prototype-scope
 * tag creation. When this header is pulled in early (e.g., via
 * irix-cc's time.h force-include), struct tm may not yet be fully
 * defined. Without this forward declaration, the struct tm in the
 * timegm prototype would be a prototype-scope tag (C99 6.2.1p4),
 * causing "conflicting types" when timegm is later re-declared
 * with the file-scope struct tm.
 */
struct tm;
#ifndef timegm
time_t timegm(struct tm *tm);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _MOGRIX_COMPAT_TIME_H */
