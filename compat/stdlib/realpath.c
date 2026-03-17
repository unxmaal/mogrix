/* realpath.c - POSIX.1-2008 realpath(path, NULL) for IRIX
 *
 * IRIX 6.5's realpath() does not support NULL as the second argument.
 * POSIX.1-2008 specifies that realpath(path, NULL) should malloc a
 * buffer and return the resolved path in it. Many modern programs
 * rely on this behavior.
 *
 * This replacement always uses a stack buffer internally, then
 * strdup's the result when the caller passed NULL. When the caller
 * provides a buffer, we use it directly (same as IRIX libc).
 *
 * Linked into executables via the compat archive, overriding libc's
 * realpath at link time.
 */

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>

/* Minimal realpath implementation that handles NULL resolved_path.
 * We implement it from scratch rather than wrapping libc's version,
 * because we can't call libc's realpath by another name from a
 * static archive override.
 */
char *realpath(const char *path, char *resolved_path)
{
    char buf[PATH_MAX];
    char tmp[PATH_MAX];
    char *dst;
    const char *src;
    struct stat st;
    int len;

    if (path == NULL) {
        errno = EINVAL;
        return NULL;
    }

    if (path[0] == '\0') {
        errno = ENOENT;
        return NULL;
    }

    /* Start with cwd if relative path */
    if (path[0] != '/') {
        if (getcwd(buf, sizeof(buf)) == NULL)
            return NULL;
        len = strlen(buf);
    } else {
        buf[0] = '/';
        len = 1;
    }

    src = path;
    while (*src) {
        /* Skip slashes */
        while (*src == '/') src++;
        if (*src == '\0') break;

        /* Find end of component */
        const char *end = src;
        while (*end && *end != '/') end++;
        int complen = end - src;

        if (complen == 1 && src[0] == '.') {
            /* "." — skip */
        } else if (complen == 2 && src[0] == '.' && src[1] == '.') {
            /* ".." — go up */
            while (len > 1 && buf[len - 1] != '/')
                len--;
            if (len > 1)
                len--;  /* remove trailing slash */
        } else {
            /* Regular component */
            if (len > 1) {
                if (len + 1 >= PATH_MAX) { errno = ENAMETOOLONG; return NULL; }
                buf[len++] = '/';
            }
            if (len + complen >= PATH_MAX) { errno = ENAMETOOLONG; return NULL; }
            memcpy(buf + len, src, complen);
            len += complen;
        }

        buf[len] = '\0';

        /* Check for symlink */
        struct stat lst;
        if (lstat(buf, &lst) != 0)
            return NULL;

        if (S_ISLNK(lst.st_mode)) {
            int linklen = readlink(buf, tmp, sizeof(tmp) - 1);
            if (linklen < 0)
                return NULL;
            tmp[linklen] = '\0';

            /* Rebuild: if symlink is absolute, restart; else replace component */
            if (tmp[0] == '/') {
                /* Absolute symlink — restart from root + remaining path */
                char remaining[PATH_MAX];
                snprintf(remaining, sizeof(remaining), "%s/%s", tmp, end);
                strncpy(buf, remaining, sizeof(buf) - 1);
                buf[sizeof(buf) - 1] = '\0';
                /* Recurse (simplified: just call ourselves) */
                return realpath(buf, resolved_path);
            } else {
                /* Relative symlink — replace last component */
                while (len > 1 && buf[len - 1] != '/')
                    len--;
                if (len > 1) len--;
                buf[len] = '\0';

                char remaining[PATH_MAX];
                snprintf(remaining, sizeof(remaining), "%s/%s/%s", buf, tmp, end);
                return realpath(remaining, resolved_path);
            }
        }

        src = end;
    }

    buf[len] = '\0';

    /* Verify the path exists */
    if (stat(buf, &st) != 0)
        return NULL;

    if (resolved_path != NULL) {
        strcpy(resolved_path, buf);
        return resolved_path;
    }

    /* POSIX.1-2008: allocate and return */
    dst = strdup(buf);
    if (dst == NULL)
        errno = ENOMEM;
    return dst;
}
