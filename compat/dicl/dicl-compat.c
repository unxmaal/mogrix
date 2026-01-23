/*
 * Mogrix libdicl-compatible library
 *
 * Provides the ld_* function symbols that rpm expects when built
 * with libdicl headers. This is a minimal implementation that wraps
 * IRIX's native libc functions.
 *
 * License: GPL-2.0-or-later (to match libdicl)
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

/* Version function */
const char *libdicl_getversion(void) {
    return "mogrix libdicl-compat 0.1.0";
}

/*
 * Number conversion functions
 * These wrap the native IRIX functions. The libdicl headers
 * #define strtod to ld_strtod, etc., so we need to provide these symbols.
 */

/* Need to temporarily undefine the macros if they're set */
#ifdef strtod
#undef strtod
#endif
#ifdef strtold
#undef strtold
#endif
#ifdef strtol
#undef strtol
#endif
#ifdef strtoll
#undef strtoll
#endif
#ifdef strtoul
#undef strtoul
#endif
#ifdef strtoull
#undef strtoull
#endif

double ld_strtod(const char *nptr, char **endptr) {
    return strtod(nptr, endptr);
}

/*
 * ld_strtold: IRIX doesn't have strtold and LLVM uses 128-bit long double
 * which requires __extenddftf2 runtime function that IRIX lacks.
 * Since rpm and most code doesn't use strtold, we provide a stub that
 * just returns 0.0L (the literal avoids any runtime conversions).
 * The string is still parsed via strtod to set endptr correctly.
 */
long double ld_strtold(const char *nptr, char **endptr) {
    /* Parse with strtod to set endptr, but discard the result */
    (void)strtod(nptr, endptr);
    /* Return 0 to avoid triggering __extenddftf2 */
    return 0.0L;
}

long int ld_strtol(const char *nptr, char **endptr, int base) {
    return strtol(nptr, endptr, base);
}

long long int ld_strtoll(const char *nptr, char **endptr, int base) {
    return strtoll(nptr, endptr, base);
}

unsigned long int ld_strtoul(const char *nptr, char **endptr, int base) {
    return strtoul(nptr, endptr, base);
}

unsigned long long int ld_strtoull(const char *nptr, char **endptr, int base) {
    return strtoull(nptr, endptr, base);
}

/*
 * Program name functions
 * IRIX doesn't have getprogname/setprogname, so we implement them.
 * Note: This is not thread-safe but rpm typically doesn't need that.
 */
static const char *__mogrix_progname = "";

const char *getprogname(void) {
    return __mogrix_progname;
}

void setprogname(const char *progname) {
    const char *p;

    if (progname == NULL) {
        progname = "";
    }

    /* Find the basename */
    p = strrchr(progname, '/');
    if (p != NULL) {
        progname = p + 1;
    }

    __mogrix_progname = progname;
}

/*
 * Environment functions
 * IRIX should have setenv/unsetenv, but provide them just in case.
 */
#ifndef HAVE_SETENV
int setenv(const char *name, const char *value, int overwrite) {
    char *env_str;
    size_t len;

    if (name == NULL || name[0] == '\0' || strchr(name, '=') != NULL) {
        errno = EINVAL;
        return -1;
    }

    if (!overwrite && getenv(name) != NULL) {
        return 0;
    }

    len = strlen(name) + strlen(value) + 2;
    env_str = malloc(len);
    if (env_str == NULL) {
        return -1;
    }

    snprintf(env_str, len, "%s=%s", name, value);
    return putenv(env_str);
}
#endif

#ifndef HAVE_UNSETENV
int unsetenv(const char *name) {
    extern char **environ;
    char **ep, **sp;
    size_t len;

    if (name == NULL || name[0] == '\0' || strchr(name, '=') != NULL) {
        errno = EINVAL;
        return -1;
    }

    len = strlen(name);

    for (ep = environ; *ep != NULL; ) {
        if (strncmp(*ep, name, len) == 0 && (*ep)[len] == '=') {
            /* Found it - shift subsequent entries */
            for (sp = ep; *sp != NULL; sp++) {
                *sp = *(sp + 1);
            }
        } else {
            ep++;
        }
    }

    return 0;
}
#endif

/*
 * mkdtemp - create a unique temporary directory
 * IRIX may not have this, provide implementation if needed.
 */
#ifndef HAVE_MKDTEMP
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

char *mkdtemp(char *template) {
    int len = strlen(template);
    char *suffix;
    int i, tries;
    static const char letters[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

    if (len < 6 || strcmp(template + len - 6, "XXXXXX") != 0) {
        errno = EINVAL;
        return NULL;
    }

    suffix = template + len - 6;

    for (tries = 0; tries < 100; tries++) {
        for (i = 0; i < 6; i++) {
            suffix[i] = letters[rand() % (sizeof(letters) - 1)];
        }

        if (mkdir(template, 0700) == 0) {
            return template;
        }

        if (errno != EEXIST) {
            return NULL;
        }
    }

    errno = EEXIST;
    return NULL;
}
#endif

/*
 * posix_openpt - open a pseudo-terminal master device
 * IRIX uses /dev/ptmx
 */
#ifndef HAVE_POSIX_OPENPT
#include <fcntl.h>

int posix_openpt(int flags) {
    return open("/dev/ptmx", flags);
}
#endif
