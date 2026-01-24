/* getprogname.c - provide getprogname() for IRIX
 *
 * IRIX's /proc interface is different from Linux and the gnulib
 * implementation tries to use procfs structures that don't exist
 * or have different types on IRIX.
 *
 * This simplified version just returns "?" since the program name
 * is primarily used for error messages and is not critical.
 */

#include <stdlib.h>

/* Global to store program name if set via setprogname */
static const char *_progname = "?";

const char *
getprogname(void)
{
    return _progname;
}

void
setprogname(const char *name)
{
    if (name != NULL) {
        /* Find the basename */
        const char *p = name;
        const char *base = name;
        while (*p) {
            if (*p == '/')
                base = p + 1;
            p++;
        }
        _progname = base;
    }
}
