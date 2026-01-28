/* asprintf.c - provide asprintf() and vasprintf() for IRIX
 *
 * IRIX 6.5 libc does not provide these GNU extensions.
 * Only compiled when __sgi is defined.
 *
 * NOTE: IRIX vsnprintf does NOT support C99 behavior where
 * vsnprintf(NULL, 0, fmt, ap) returns the required buffer size.
 * Instead, IRIX returns -1 for NULL buffer. We use an iterative
 * approach: start with a reasonable buffer and grow if needed.
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
    int size = 256;  /* Initial buffer size */

    if (strp == NULL || fmt == NULL) {
        return -1;
    }

    while (1) {
        str = (char *)malloc(size);
        if (str == NULL) {
            *strp = NULL;
            return -1;
        }

        va_copy(ap_copy, ap);
        len = vsnprintf(str, size, fmt, ap_copy);
        va_end(ap_copy);

        /* Success: len is the number of chars written (not including NUL) */
        if (len >= 0 && len < size) {
            *strp = str;
            return len;
        }

        /* Buffer too small - need to grow */
        free(str);

        /* If len >= size, that's the required size (C99 behavior) */
        if (len >= size) {
            size = len + 1;
        } else {
            /* IRIX returns -1 on truncation - just double the buffer */
            size *= 2;
            /* Safety check - don't allocate more than 1MB */
            if (size > 1024 * 1024) {
                *strp = NULL;
                return -1;
            }
        }
    }
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
