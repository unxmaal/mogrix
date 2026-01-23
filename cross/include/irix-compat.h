#ifndef IRIX_COMPAT_H
#define IRIX_COMPAT_H

/* MIPSpro built-in type used by IRIX headers - clang doesn't have this */
#ifndef __long_long
#define __long_long long long
#endif

/* IRIX-specific type definitions that clang doesn't provide */
/* These must be defined before any IRIX headers are included */
#ifndef __int8_t
typedef signed char __int8_t;
#endif
#ifndef __uint8_t
typedef unsigned char __uint8_t;
#endif
#ifndef __int16_t
typedef short __int16_t;
#endif
#ifndef __uint16_t
typedef unsigned short __uint16_t;
#endif
#ifndef __int32_t
typedef int __int32_t;
#endif
#ifndef __uint32_t
typedef unsigned int __uint32_t;
#endif
#ifndef __int64_t
typedef long long __int64_t;
#endif
#ifndef __uint64_t
typedef unsigned long long __uint64_t;
#endif

/* Scaling types - for n32 ABI, these are 32-bit */
#ifndef __scint_t
typedef __int32_t __scint_t;
#endif
#ifndef __scunsigned_t
typedef __uint32_t __scunsigned_t;
#endif

/* BSD-style short type names used by some IRIX headers */
#ifndef _USHORT_T
#define _USHORT_T
typedef unsigned short ushort;
#endif
#ifndef _UINT_T
#define _UINT_T
typedef unsigned int uint;
#endif
#ifndef _ULONG_T
#define _ULONG_T
typedef unsigned long ulong;
#endif
/* u_ prefixed types */
#ifndef u_char
typedef unsigned char u_char;
#endif
#ifndef u_short
typedef unsigned short u_short;
#endif
#ifndef u_int
typedef unsigned int u_int;
#endif
#ifndef u_long
typedef unsigned long u_long;
#endif

/* Note: Do NOT define __c99 - it causes conflicting va_list declarations
 * in IRIX stdio headers. The non-c99 path works better with clang. */

/* Define va_list using clang's built-in before IRIX headers try to define it
 * as char*. This must be done before any IRIX stdio headers are included. */
#ifndef _VA_LIST_
#define _VA_LIST_
typedef __builtin_va_list va_list;
#endif

/* alloca - IRIX provides alloca as a compiler built-in
 * It's declared in stdlib.h on IRIX, but for cross-compilation
 * with clang, we use the built-in directly */
#ifndef alloca
#define alloca(size) __builtin_alloca(size)
#endif

/* C99 inttypes.h format specifiers - IRIX may not have these */
#ifndef PRId64
#define PRId64 "lld"
#endif
#ifndef PRIu64
#define PRIu64 "llu"
#endif
#ifndef PRIx64
#define PRIx64 "llx"
#endif
#ifndef PRIX64
#define PRIX64 "llX"
#endif
#ifndef PRId32
#define PRId32 "d"
#endif
#ifndef PRIu32
#define PRIu32 "u"
#endif
#ifndef PRIx32
#define PRIx32 "x"
#endif

/* C99 long long limits - IRIX limits.h may not have these */
#ifndef LLONG_MAX
#define LLONG_MAX  9223372036854775807LL
#endif
#ifndef LLONG_MIN
#define LLONG_MIN  (-LLONG_MAX - 1LL)
#endif
#ifndef ULLONG_MAX
#define ULLONG_MAX 18446744073709551615ULL
#endif

/* C99 strto* functions for long long - IRIX doesn't have these in stdlib.h */
extern long long strtoll(const char *, char **, int);
extern unsigned long long strtoull(const char *, char **, int);

/* IRIX compatibility - provide isinf since __c99 isn't defined */
#ifndef isinf
#define isinf(x) ((x) == (1.0/0.0) || (x) == (-1.0/0.0))
#endif

/* log2 is C99 but not in IRIX math.h
 * These macros assume math.h has been included when used */
#ifndef log2
#define log2(x) (log(x) / 0.693147180559945309417)  /* log(x) / log(2) */
#endif
#ifndef log2f
#define log2f(x) ((float)(log2((double)(x))))
#endif

/* C99 floating-point comparison macros - not in IRIX math.h */
#ifndef isunordered
#define isunordered(x, y) (isnan(x) || isnan(y))
#endif
#ifndef isgreater
#define isgreater(x, y) (!isunordered(x, y) && (x) > (y))
#endif
#ifndef isless
#define isless(x, y) (!isunordered(x, y) && (x) < (y))
#endif
#ifndef isgreaterequal
#define isgreaterequal(x, y) (!isunordered(x, y) && (x) >= (y))
#endif
#ifndef islessequal
#define islessequal(x, y) (!isunordered(x, y) && (x) <= (y))
#endif
#ifndef islessgreater
#define islessgreater(x, y) (!isunordered(x, y) && ((x) < (y) || (x) > (y)))
#endif

/* isnan macro if not defined */
#ifndef isnan
#define isnan(x) ((x) != (x))
#endif

/* struct winsize is hidden behind XOPEN conditionals in IRIX termios.h
 * Provide it here since we use _XOPEN_SOURCE=500 */
#ifndef _STRUCT_WINSIZE_DEFINED
#define _STRUCT_WINSIZE_DEFINED
struct winsize {
    unsigned short ws_row;       /* rows, in characters */
    unsigned short ws_col;       /* columns, in characters */
    unsigned short ws_xpixel;    /* horizontal size, pixels */
    unsigned short ws_ypixel;    /* vertical size, pixels */
};
#endif

/* TIOCGWINSZ is also hidden behind XOPEN conditionals */
#ifndef TIOCGWINSZ
/* _IOR('t', 104, struct winsize) expands to this on IRIX */
#define TIOCGWINSZ (0x40000000 | ((sizeof(struct winsize) & 0x1fff) << 16) | (('t') << 8) | (104))
#endif
#ifndef TIOCSWINSZ
#define TIOCSWINSZ (0x80000000 | ((sizeof(struct winsize) & 0x1fff) << 16) | (('t') << 8) | (103))
#endif

/* qsort_r is a GNU extension not available in IRIX
 * Provide a simple shim using a global variable for the context
 * This is not thread-safe but works for most use cases
 * Note: We need stddef.h for size_t and declare qsort since this header
 * is included before stdlib.h via -include */
#ifndef HAVE_QSORT_R
#include <stddef.h>  /* for size_t */
/* Forward declare qsort - will match stdlib.h declaration */
extern void qsort(void *, size_t, size_t, int (*)(const void *, const void *));

static void *_qsort_r_context;
static int (*_qsort_r_compar)(const void *, const void *, void *);

static int _qsort_r_wrapper(const void *a, const void *b)
{
    return _qsort_r_compar(a, b, _qsort_r_context);
}

static inline void qsort_r(void *base, size_t nmemb, size_t size,
                           int (*compar)(const void *, const void *, void *),
                           void *arg)
{
    _qsort_r_context = arg;
    _qsort_r_compar = compar;
    qsort(base, nmemb, size, _qsort_r_wrapper);
}
#define HAVE_QSORT_R 1
#endif

/* strsep is a BSD extension not available in IRIX
 * Provide a simple implementation */
#ifndef HAVE_STRSEP
static inline char *strsep(char **stringp, const char *delim)
{
    char *s, *tok;
    const char *spanp;
    int c, sc;

    if ((s = *stringp) == NULL)
        return NULL;
    for (tok = s;;) {
        c = *s++;
        spanp = delim;
        do {
            if ((sc = *spanp++) == c) {
                if (c == 0)
                    s = NULL;
                else
                    s[-1] = 0;
                *stringp = s;
                return tok;
            }
        } while (sc != 0);
    }
    /* NOTREACHED */
}
#define HAVE_STRSEP 1
#endif

/* setenv/unsetenv are POSIX functions not available in IRIX
 * Implement using putenv which IRIX does have
 * Note: We don't include stdlib.h here since this header is included first
 * via -include. We forward-declare just what we need with correct types. */
#ifndef HAVE_SETENV
extern void *malloc(size_t);
extern void free(void *);
extern int putenv(char *);  /* IRIX with XOPEN_SOURCE>=500 uses char* */
extern char *getenv(const char *);
/* Need string functions but they're declared later, use compiler built-ins */
extern size_t strlen(const char *);
extern char *strcpy(char *, const char *);
extern char *strcat(char *, const char *);
extern char *strchr(const char *, int);

static inline int setenv(const char *name, const char *value, int overwrite)
{
    char *env;
    size_t len;

    if (!name || name[0] == '\0' || strchr(name, '=') != 0) {
        return -1;
    }

    if (!overwrite && getenv(name) != 0) {
        return 0;
    }

    len = strlen(name) + strlen(value) + 2;
    env = (char *)malloc(len);
    if (env == 0) {
        return -1;
    }

    strcpy(env, name);
    strcat(env, "=");
    strcat(env, value);

    /* putenv takes ownership of the string - do not free */
    return putenv(env);
}

static inline int unsetenv(const char *name)
{
    char *env;

    if (!name || name[0] == '\0' || strchr(name, '=') != 0) {
        return -1;
    }

    /* Setting to empty string effectively unsets on many systems */
    /* On IRIX, we'd need to manipulate environ directly for proper unset */
    env = (char *)malloc(strlen(name) + 2);
    if (env == 0) {
        return -1;
    }
    strcpy(env, name);
    strcat(env, "=");
    return putenv(env);
}
#define HAVE_SETENV 1
#endif

/* FNM_CASEFOLD is a GNU extension for case-insensitive fnmatch */
#ifndef FNM_CASEFOLD
#define FNM_CASEFOLD 0x10
#endif

/* strchrnul is a GNU extension - like strchr but returns pointer to null terminator if not found */
#ifndef HAVE_STRCHRNUL
static inline char *strchrnul(const char *s, int c)
{
    while (*s && *s != (char)c)
        s++;
    return (char *)s;
}
#define HAVE_STRCHRNUL 1
#endif

/* Forward declare strcasecmp/strncasecmp - these are in strings.h on IRIX
 * but libsolv doesn't include that header */
extern int strcasecmp(const char *, const char *);
extern int strncasecmp(const char *, const char *, size_t);

/* strcasestr is a GNU extension - case-insensitive strstr */
#ifndef HAVE_STRCASESTR
extern int tolower(int);
static inline char *strcasestr(const char *haystack, const char *needle)
{
    size_t needle_len;
    if (!needle || !*needle)
        return (char *)haystack;
    needle_len = strlen(needle);
    while (*haystack) {
        if (strncasecmp(haystack, needle, needle_len) == 0)
            return (char *)haystack;
        haystack++;
    }
    return 0;
}
#define HAVE_STRCASESTR 1
#endif

/* scandir and alphasort are hidden behind _SGIAPI in IRIX dirent.h
 * when _XOPEN_SOURCE is defined. Provide forward declarations.
 * Note: Use const qualifiers to match BSD/POSIX signatures that libsolv expects */
struct dirent;  /* Forward declaration */
extern int scandir(const char *, struct dirent ***,
                   int (*)(const struct dirent *),
                   int (*)(const struct dirent **, const struct dirent **));
extern int alphasort(const struct dirent **, const struct dirent **);

/* timegm is a BSD/GNU extension - inverse of gmtime()
 * Not available in IRIX, implement using mktime and timezone adjustment */
#ifndef HAVE_TIMEGM
/* Forward declarations - we can't include time.h since this is included before it */
struct tm;
extern long timezone;  /* IRIX provides this global */
extern char *tzname[2];
extern void tzset(void);

/* mktime declaration - matches IRIX */
extern long mktime(struct tm *);

static inline long timegm(struct tm *tm)
{
    long result;
    char *tz;

    /* Save current TZ */
    tz = getenv("TZ");

    /* Set TZ to UTC */
    setenv("TZ", "UTC", 1);
    tzset();

    result = mktime(tm);

    /* Restore TZ */
    if (tz)
        setenv("TZ", tz, 1);
    else
        unsetenv("TZ");
    tzset();

    return result;
}
#define HAVE_TIMEGM 1
#endif

#endif /* IRIX_COMPAT_H */
