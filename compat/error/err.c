/*
 * err.c — BSD error functions for IRIX
 *
 * Provides err/errx/warn/warnx and va_list variants.
 * These are standard on BSD and Linux but missing on IRIX.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

void vwarn(const char *fmt, va_list ap)
{
    int saved_errno = errno;
    if (fmt != NULL) {
        vfprintf(stderr, fmt, ap);
        fprintf(stderr, ": ");
    }
    fprintf(stderr, "%s\n", strerror(saved_errno));
}

void vwarnx(const char *fmt, va_list ap)
{
    if (fmt != NULL)
        vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
}

void warn(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vwarn(fmt, ap);
    va_end(ap);
}

void warnx(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vwarnx(fmt, ap);
    va_end(ap);
}

void verr(int eval, const char *fmt, va_list ap)
{
    vwarn(fmt, ap);
    exit(eval);
}

void verrx(int eval, const char *fmt, va_list ap)
{
    vwarnx(fmt, ap);
    exit(eval);
}

void err(int eval, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    verr(eval, fmt, ap);
    va_end(ap);
}

void errx(int eval, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    verrx(eval, fmt, ap);
    va_end(ap);
}
