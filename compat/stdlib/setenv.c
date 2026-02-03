/*
 * setenv.c - Set environment variable
 *
 * IRIX doesn't have setenv(), only putenv(). This provides a setenv()
 * implementation using direct environ manipulation.
 */
#include <stdlib.h>
#include <string.h>

extern char **environ;

int setenv(const char *name, const char *value, int overwrite)
{
    char *env_str;
    char **ep;
    size_t name_len, value_len;

    if (name == NULL || name[0] == '\0' || strchr(name, '=') != NULL) {
        return -1;
    }

    name_len = strlen(name);

    /* Check if variable already exists */
    if (environ) {
        for (ep = environ; *ep != NULL; ep++) {
            if (strncmp(*ep, name, name_len) == 0 && (*ep)[name_len] == '=') {
                if (!overwrite) {
                    return 0;  /* Already exists and don't overwrite */
                }
                break;
            }
        }
    }

    /* Create new "name=value" string */
    value_len = value ? strlen(value) : 0;
    env_str = malloc(name_len + 1 + value_len + 1);
    if (env_str == NULL) {
        return -1;
    }

    memcpy(env_str, name, name_len);
    env_str[name_len] = '=';
    if (value) {
        memcpy(env_str + name_len + 1, value, value_len);
    }
    env_str[name_len + 1 + value_len] = '\0';

    /* Use putenv if available, otherwise manually add to environ */
    /* Note: This leaks the old value if overwriting, but that's standard behavior */
    if (putenv(env_str) != 0) {
        free(env_str);
        return -1;
    }

    return 0;
}

int unsetenv(const char *name)
{
    char **ep, **dp;
    size_t name_len;

    if (name == NULL || name[0] == '\0' || strchr(name, '=') != NULL) {
        return -1;
    }

    if (environ == NULL) {
        return 0;
    }

    name_len = strlen(name);

    /* Find and remove all matching entries */
    dp = environ;
    for (ep = environ; *ep != NULL; ep++) {
        if (strncmp(*ep, name, name_len) == 0 && (*ep)[name_len] == '=') {
            /* Skip this entry (effectively removes it) */
            continue;
        }
        *dp++ = *ep;
    }
    *dp = NULL;

    return 0;
}
