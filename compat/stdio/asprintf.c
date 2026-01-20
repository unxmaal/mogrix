/* asprintf.c - provide asprintf() and vasprintf() for IRIX
 *
 * IRIX 6.5 libc does not provide these GNU extensions.
 * Only compiled when __sgi is defined.
 */

#if defined(__sgi)

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

int vasprintf(char **strp, const char *fmt, va_list ap)
{
    va_list ap_copy;
    int len;
    char *str;

    if (strp == NULL || fmt == NULL) {
        return -1;
    }

    /* First, determine the length needed */
    va_copy(ap_copy, ap);
    len = vsnprintf(NULL, 0, fmt, ap_copy);
    va_end(ap_copy);

    if (len < 0) {
        *strp = NULL;
        return -1;
    }

    /* Allocate buffer */
    str = (char *)malloc(len + 1);
    if (str == NULL) {
        *strp = NULL;
        return -1;
    }

    /* Format the string */
    len = vsnprintf(str, len + 1, fmt, ap);
    if (len < 0) {
        free(str);
        *strp = NULL;
        return -1;
    }

    *strp = str;
    return len;
}

int asprintf(char **strp, const char *fmt, ...)
{
    va_list ap;
    int ret;

    va_start(ap, fmt);
    ret = vasprintf(strp, fmt, ap);
    va_end(ap);

    return ret;
}

#endif /* __sgi */
