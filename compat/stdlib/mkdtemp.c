/*
 * Portable mkdtemp() implementation for IRIX
 *
 * mkdtemp is a BSD/POSIX function that creates a unique temporary directory.
 * IRIX may not have it.
 *
 * Licensed under 3-clause BSD license.
 */

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>

/*
 * mkdtemp - create a unique temporary directory
 *
 * The template must end in "XXXXXX" which is replaced with a unique
 * suffix. Returns the template on success, NULL on failure.
 */
char *mkdtemp(char *template)
{
    static const char chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    char *suffix;
    size_t len;
    int tries;
    unsigned int seed;

    if (!template) {
        errno = EINVAL;
        return NULL;
    }

    len = strlen(template);
    if (len < 6 || strcmp(template + len - 6, "XXXXXX") != 0) {
        errno = EINVAL;
        return NULL;
    }

    suffix = template + len - 6;
    seed = (unsigned int)time(NULL) ^ (unsigned int)getpid();

    /* Try up to 100 times to create a unique directory */
    for (tries = 0; tries < 100; tries++) {
        int i;

        /* Generate random suffix */
        for (i = 0; i < 6; i++) {
            seed = seed * 1103515245 + 12345;
            suffix[i] = chars[(seed >> 16) % (sizeof(chars) - 1)];
        }

        /* Try to create the directory */
        if (mkdir(template, 0700) == 0) {
            return template;
        }

        /* If error is not EEXIST, give up */
        if (errno != EEXIST) {
            return NULL;
        }
    }

    errno = EEXIST;
    return NULL;
}
