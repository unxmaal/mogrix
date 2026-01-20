/*
 * strtof - Convert string to float (C99 function)
 *
 * IRIX 6.5 doesn't have strtof, but has strtod.
 * This is a simple wrapper that calls strtod and casts to float.
 */

#include <stdlib.h>
#include <errno.h>
#include <math.h>

float strtof(const char *nptr, char **endptr)
{
    double d;
    float f;
    int saved_errno = errno;

    errno = 0;
    d = strtod(nptr, endptr);

    /* Check for overflow when converting to float */
    if (d != 0.0 && errno == 0) {
        if (d > 3.402823466e+38 || d < -3.402823466e+38) {
            errno = ERANGE;
            f = (d > 0) ? HUGE_VALF : -HUGE_VALF;
        } else if (d != 0.0 && d > -1.175494351e-38 && d < 1.175494351e-38) {
            /* Underflow - number too small for float */
            f = 0.0f;
        } else {
            f = (float)d;
        }
    } else {
        f = (float)d;
    }

    /* Preserve errno from strtod if it set an error */
    if (errno == 0)
        errno = saved_errno;

    return f;
}
