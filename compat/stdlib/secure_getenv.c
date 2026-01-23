/*
 * secure_getenv - GNU extension for secure environment variable access
 *
 * On Linux/glibc, this returns NULL if the program is running with elevated
 * privileges (setuid/setgid). On IRIX, we implement this as a simple wrapper
 * around getenv() since cross-compiled packages don't have setuid concerns.
 */

#include <stdlib.h>

char *secure_getenv(const char *name)
{
    /* On IRIX, just use regular getenv. The security feature is not critical
     * for cross-compiled packages running in a trusted environment. */
    return getenv(name);
}
