/* strerror_r.c - provide strerror_r() for IRIX
 *
 * IRIX 6.5 has strerror() but not the thread-safe strerror_r().
 * This provides the POSIX (XSI-compliant) version.
 * Only compiled when __sgi is defined.
 */

#if defined(__sgi)

#include <string.h>
#include <errno.h>

int strerror_r(int errnum, char *buf, size_t buflen)
{
    const char *msg;
    size_t len;

    if (buf == NULL || buflen == 0) {
        return ERANGE;
    }

    /* Use the non-thread-safe strerror - on IRIX this is acceptable
     * for most uses since we're typically single-threaded or using
     * it in error paths where thread safety is less critical. */
    msg = strerror(errnum);
    if (msg == NULL) {
        /* Unknown error number */
        msg = "Unknown error";
    }

    len = strlen(msg);
    if (len >= buflen) {
        /* Truncate and return error */
        memcpy(buf, msg, buflen - 1);
        buf[buflen - 1] = '\0';
        return ERANGE;
    }

    memcpy(buf, msg, len + 1);
    return 0;
}

#endif /* __sgi */
