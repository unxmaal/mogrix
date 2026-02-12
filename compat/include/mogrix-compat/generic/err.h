/*
 * err.h — BSD error functions for IRIX
 *
 * IRIX has no <err.h>. This provides err(), errx(), warn(), warnx()
 * and their va_list variants for packages that expect BSD error reporting.
 */
#ifndef MOGRIX_COMPAT_ERR_H
#define MOGRIX_COMPAT_ERR_H

#include <stdarg.h>

void err(int eval, const char *fmt, ...);
void errx(int eval, const char *fmt, ...);
void warn(const char *fmt, ...);
void warnx(const char *fmt, ...);
void verr(int eval, const char *fmt, va_list ap);
void verrx(int eval, const char *fmt, va_list ap);
void vwarn(const char *fmt, va_list ap);
void vwarnx(const char *fmt, va_list ap);

#endif /* MOGRIX_COMPAT_ERR_H */
