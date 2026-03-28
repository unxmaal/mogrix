/*
 * xlocale_irix.h — POSIX.1-2008 xlocale API for IRIX 6.5
 *
 * IRIX has setlocale() but lacks the thread-local locale API:
 *   locale_t, newlocale, freelocale, uselocale, duplocale
 *   *_l functions (strtod_l, toupper_l, strcoll_l, etc.)
 *
 * This implementation wraps the global locale. The *_l functions
 * temporarily switch the global locale, call the base function,
 * then restore. NOT thread-safe for locale-sensitive operations,
 * but sufficient for single-locale programs (everything on IRIX).
 *
 * Include this BEFORE libc++ headers. Deployed to staging via
 * mogrix setup-cross.
 */
#ifndef _MOGRIX_XLOCALE_IRIX_H
#define _MOGRIX_XLOCALE_IRIX_H

#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <wctype.h>
#include <wchar.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ─── locale_t type ─── */

/* LC_*_MASK constants for newlocale() category_mask */
#ifndef LC_COLLATE_MASK
#define LC_COLLATE_MASK   (1 << LC_COLLATE)
#define LC_CTYPE_MASK     (1 << LC_CTYPE)
#define LC_MONETARY_MASK  (1 << LC_MONETARY)
#define LC_NUMERIC_MASK   (1 << LC_NUMERIC)
#define LC_TIME_MASK      (1 << LC_TIME)
#define LC_MESSAGES_MASK  (1 << LC_MESSAGES)
#define LC_ALL_MASK       (LC_COLLATE_MASK | LC_CTYPE_MASK | LC_MONETARY_MASK | \
                           LC_NUMERIC_MASK | LC_TIME_MASK | LC_MESSAGES_MASK)
#endif

/* locale_t: opaque handle to a locale object */
typedef struct _mogrix_locale {
    char name[64];  /* locale name (e.g., "C", "POSIX", "en_US.UTF-8") */
} *locale_t;

/* Global "current" locale object for uselocale() tracking */
/* LC_GLOBAL_LOCALE sentinel — means "use the global locale" */
#define LC_GLOBAL_LOCALE ((locale_t)-1)

/* ─── Locale management ─── */

static inline locale_t newlocale(int __mask, const char *__locale, locale_t __base) {
    locale_t loc;
    if (__base && __base != LC_GLOBAL_LOCALE) {
        loc = __base;  /* reuse */
    } else {
        loc = (locale_t)malloc(sizeof(struct _mogrix_locale));
        if (!loc) return (locale_t)0;
    }
    if (__locale)
        strncpy(loc->name, __locale, sizeof(loc->name) - 1);
    else
        strncpy(loc->name, "C", sizeof(loc->name) - 1);
    loc->name[sizeof(loc->name) - 1] = '\0';
    (void)__mask;
    return loc;
}

static inline void freelocale(locale_t __loc) {
    if (__loc && __loc != LC_GLOBAL_LOCALE)
        free(__loc);
}

static inline locale_t duplocale(locale_t __loc) {
    locale_t dup = (locale_t)malloc(sizeof(struct _mogrix_locale));
    if (!dup) return (locale_t)0;
    if (__loc && __loc != LC_GLOBAL_LOCALE)
        memcpy(dup, __loc, sizeof(struct _mogrix_locale));
    else
        strncpy(dup->name, "C", sizeof(dup->name));
    return dup;
}

/* uselocale: set thread-local locale (we fake it with global) */
static inline locale_t uselocale(locale_t __loc) {
    static struct _mogrix_locale _current = { "C" };
    locale_t prev = &_current;
    if (__loc && __loc != LC_GLOBAL_LOCALE) {
        /* "Switch" to the new locale — set global */
        setlocale(LC_ALL, __loc->name);
        memcpy(&_current, __loc, sizeof(struct _mogrix_locale));
    }
    return prev;
}

/* ─── *_l functions: temporarily switch locale, call base, restore ─── */

/* String-to-number */
static inline float strtof_l(const char *__nptr, char **__endptr, locale_t __loc) {
    (void)__loc;
    return strtof(__nptr, __endptr);
}

static inline double strtod_l(const char *__nptr, char **__endptr, locale_t __loc) {
    (void)__loc;
    return strtod(__nptr, __endptr);
}

static inline long double strtold_l(const char *__nptr, char **__endptr, locale_t __loc) {
    (void)__loc;
    return strtold(__nptr, __endptr);
}

static inline long long strtoll_l(const char *__nptr, char **__endptr, int __base, locale_t __loc) {
    (void)__loc;
    return strtoll(__nptr, __endptr, __base);
}

static inline unsigned long long strtoull_l(const char *__nptr, char **__endptr, int __base, locale_t __loc) {
    (void)__loc;
    return strtoull(__nptr, __endptr, __base);
}

/* Character classification */
static inline int toupper_l(int __c, locale_t __loc) { (void)__loc; return toupper(__c); }
static inline int tolower_l(int __c, locale_t __loc) { (void)__loc; return tolower(__c); }
static inline int isdigit_l(int __c, locale_t __loc) { (void)__loc; return isdigit(__c); }
static inline int isxdigit_l(int __c, locale_t __loc) { (void)__loc; return isxdigit(__c); }

/* String collation */
static inline int strcoll_l(const char *__s1, const char *__s2, locale_t __loc) {
    (void)__loc;
    return strcoll(__s1, __s2);
}

static inline size_t strxfrm_l(char *__dest, const char *__src, size_t __n, locale_t __loc) {
    (void)__loc;
    return strxfrm(__dest, __src, __n);
}

/* Wide character functions */
static inline int iswctype_l(wint_t __c, wctype_t __type, locale_t __loc) {
    (void)__loc;
    return iswctype(__c, __type);
}

static inline int iswspace_l(wint_t __c, locale_t __loc) { (void)__loc; return iswspace(__c); }
static inline int iswprint_l(wint_t __c, locale_t __loc) { (void)__loc; return iswprint(__c); }
static inline int iswcntrl_l(wint_t __c, locale_t __loc) { (void)__loc; return iswcntrl(__c); }
static inline int iswupper_l(wint_t __c, locale_t __loc) { (void)__loc; return iswupper(__c); }
static inline int iswlower_l(wint_t __c, locale_t __loc) { (void)__loc; return iswlower(__c); }
static inline int iswalpha_l(wint_t __c, locale_t __loc) { (void)__loc; return iswalpha(__c); }
static inline int iswdigit_l(wint_t __c, locale_t __loc) { (void)__loc; return iswdigit(__c); }
static inline int iswpunct_l(wint_t __c, locale_t __loc) { (void)__loc; return iswpunct(__c); }
/* IRIX wctype.h declares iswblank but libc doesn't implement it — provide weak impl */
static inline int iswblank_l(wint_t __c, locale_t __loc) { (void)__loc; return iswblank(__c); }
static inline int iswxdigit_l(wint_t __c, locale_t __loc) { (void)__loc; return iswxdigit(__c); }
static inline wint_t towupper_l(wint_t __c, locale_t __loc) { (void)__loc; return towupper(__c); }
static inline wint_t towlower_l(wint_t __c, locale_t __loc) { (void)__loc; return towlower(__c); }

static inline int wcscoll_l(const wchar_t *__s1, const wchar_t *__s2, locale_t __loc) {
    (void)__loc;
    return wcscoll(__s1, __s2);
}

static inline size_t wcsxfrm_l(wchar_t *__dest, const wchar_t *__src, size_t __n, locale_t __loc) {
    (void)__loc;
    return wcsxfrm(__dest, __src, __n);
}

static inline size_t mbsrtowcs_l(wchar_t *__dest, const char **__src, size_t __len,
                                  mbstate_t *__ps, locale_t __loc) {
    (void)__loc;
    return mbsrtowcs(__dest, __src, __len, __ps);
}

static inline size_t mbrtowc_l(wchar_t *__pwc, const char *__s, size_t __n,
                                mbstate_t *__ps, locale_t __loc) {
    (void)__loc;
    return mbrtowc(__pwc, __s, __n, __ps);
}

static inline int mbtowc_l(wchar_t *__pwc, const char *__s, size_t __n, locale_t __loc) {
    (void)__loc;
    return mbtowc(__pwc, __s, __n);
}

static inline size_t mbrlen_l(const char *__s, size_t __n, mbstate_t *__ps, locale_t __loc) {
    (void)__loc;
    return mbrlen(__s, __n, __ps);
}

static inline size_t wcrtomb_l(char *__s, wchar_t __wc, mbstate_t *__ps, locale_t __loc) {
    (void)__loc;
    return wcrtomb(__s, __wc, __ps);
}

static inline int snprintf_l(char *__s, size_t __n, locale_t __loc, const char *__fmt, ...) {
    va_list ap;
    va_start(ap, __fmt);
    (void)__loc;
    int ret = vsnprintf(__s, __n, __fmt, ap);
    va_end(ap);
    return ret;
}

static inline int sscanf_l(const char *__s, locale_t __loc, const char *__fmt, ...) {
    va_list ap;
    va_start(ap, __fmt);
    (void)__loc;
    int ret = vsscanf(__s, __fmt, ap);
    va_end(ap);
    return ret;
}

static inline int asprintf_l(char **__strp, locale_t __loc, const char *__fmt, ...) {
    va_list ap;
    va_start(ap, __fmt);
    (void)__loc;
    /* IRIX lacks vasprintf. Implement via vsnprintf with two passes. */
    va_list ap2;
    va_copy(ap2, ap);
    int len = vsnprintf((char*)0, 0, __fmt, ap);
    va_end(ap);
    if (len < 0) { va_end(ap2); *__strp = (char*)0; return -1; }
    *__strp = (char*)malloc((__SIZE_TYPE__)(len + 1));
    if (!*__strp) { va_end(ap2); return -1; }
    int ret = vsnprintf(*__strp, (__SIZE_TYPE__)(len + 1), __fmt, ap2);
    va_end(ap2);
    return ret;
}

/* vasprintf: IRIX lacks this GNU/BSD extension. Implement via vsnprintf. */
static inline int vasprintf(char **__strp, const char *__fmt, va_list __ap) {
    va_list ap2;
    va_copy(ap2, __ap);
    int len = vsnprintf((char*)0, 0, __fmt, __ap);
    if (len < 0) { va_end(ap2); *__strp = (char*)0; return -1; }
    *__strp = (char*)malloc((__SIZE_TYPE__)(len + 1));
    if (!*__strp) { va_end(ap2); return -1; }
    int ret = vsnprintf(*__strp, (__SIZE_TYPE__)(len + 1), __fmt, ap2);
    va_end(ap2);
    return ret;
}

/* strftime_l: locale-aware strftime (ignore locale, use global) */
static inline size_t strftime_l(char *__s, size_t __max, const char *__fmt,
                                 const struct tm *__tm, locale_t __loc) {
    (void)__loc;
    return strftime(__s, __max, __fmt, __tm);
}

/* wcsrtombs_l: locale-aware wcsrtombs */
static inline size_t wcsrtombs_l(char *__dest, const wchar_t **__src,
                                  size_t __len, mbstate_t *__ps, locale_t __loc) {
    (void)__loc;
    return wcsrtombs(__dest, __src, __len, __ps);
}

/*
 * mbsnrtowcs / wcsnrtombs: non-standard BSD/GNU extensions.
 * IRIX libc lacks these. Our C implementations are in irix-libcxx-files/src/.
 * Declare them here so the locale backend can call them.
 */
extern size_t __bsd_mbsnrtowcs(wchar_t * __restrict, const char ** __restrict,
                                size_t, size_t, mbstate_t * __restrict);
extern size_t __bsd_wcsnrtombs(char * __restrict, const wchar_t ** __restrict,
                                size_t, size_t, mbstate_t * __restrict);

/* Provide standard names as wrappers around our implementations */
#ifndef mbsnrtowcs
static inline size_t mbsnrtowcs(wchar_t *__dest, const char **__src,
                                 size_t __nms, size_t __len, mbstate_t *__ps) {
    return __bsd_mbsnrtowcs(__dest, __src, __nms, __len, __ps);
}
#endif

#ifndef wcsnrtombs
static inline size_t wcsnrtombs(char *__dest, const wchar_t **__src,
                                 size_t __nwc, size_t __len, mbstate_t *__ps) {
    return __bsd_wcsnrtombs(__dest, __src, __nwc, __len, __ps);
}
#endif

#ifdef __cplusplus
}
#endif

#endif /* _MOGRIX_XLOCALE_IRIX_H */
