/* strerror_r.c - provide strerror_r() for IRIX
 *
 * IRIX 6.5 has strerror() but not the thread-safe strerror_r().
 * This provides the GNU version (returns char*) which is what most
 * software like OpenSSL expects.
 * Only compiled when __sgi is defined.
 */

#if defined(__sgi)

#include <string.h>
#include <errno.h>

/* GNU-style strerror_r: returns pointer to error string
 * May return either the provided buffer or a static string */
char *strerror_r(int errnum, char *buf, size_t buflen)
{
    const char *msg;
    size_t len;

    /* Use the non-thread-safe strerror - on IRIX this is acceptable
     * for most uses since we're typically single-threaded or using
     * it in error paths where thread safety is less critical. */
    msg = strerror(errnum);
    if (msg == NULL) {
        /* Unknown error number */
        msg = "Unknown error";
    }

    if (buf == NULL || buflen == 0) {
        /* Return static string if no buffer provided */
        return (char *)msg;
    }

    len = strlen(msg);
    if (len >= buflen) {
        /* Truncate to fit buffer */
        memcpy(buf, msg, buflen - 1);
        buf[buflen - 1] = '\0';
    } else {
        memcpy(buf, msg, len + 1);
    }

    return buf;
}

#endif /* __sgi */
