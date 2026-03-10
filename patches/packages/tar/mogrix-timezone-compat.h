/*
 * mogrix-timezone-compat.h — IRIX shim for NetBSD timezone API
 * IRIX lacks timezone_t, mktime_z, localtime_rz used by gnulib nstrftime.
 * gnulib normally generates these via time.in.h → time.h, but the cross-
 * compiler's sysroot time.h shadows the generated one.
 * This header provides compatible types and fallback implementations.
 */
#ifndef MOGRIX_TIMEZONE_COMPAT_H
#define MOGRIX_TIMEZONE_COMPAT_H

#include <time.h>

/* Minimal timezone struct matching gnulib's expectations.
   nstrftime.c accesses tz->tzname_copy when !HAVE_STRUCT_TM_TM_ZONE. */
struct mogrix_timezone {
    char *tzname_copy[2];
};
typedef struct mogrix_timezone *timezone_t;

static inline time_t mktime_z(timezone_t tz, struct tm *tm) {
    (void)tz;
    return mktime(tm);
}

static inline struct tm *localtime_rz(timezone_t tz, const time_t *t, struct tm *tm) {
    (void)tz;
    return localtime_r(t, tm);
}

#endif /* MOGRIX_TIMEZONE_COMPAT_H */
