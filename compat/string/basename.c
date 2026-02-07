/*
 * mogrix compat: basename()
 * IRIX has basename in libgen.so, not libc.
 * Provide a simple implementation to avoid linking against libgen.
 * This is the GNU basename behavior (doesn't modify the string).
 */
#include <string.h>

char *basename(const char *path)
{
    const char *base;

    if (path == NULL || *path == '\0')
        return (char *)".";

    base = strrchr(path, '/');
    return (char *)(base ? base + 1 : path);
}
