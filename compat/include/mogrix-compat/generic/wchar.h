/*
 * mogrix-compat/generic/wchar.h
 *
 * Wrapper that includes the real wchar.h and adds C99 multibyte function
 * declarations hidden behind IRIX's __c99 guard.
 *
 * IRIX wchar_core.h gates different functions behind different macros:
 *   mbrlen/mbrtowc/etc: defined(__c99) || defined(__cplusplus) || _XOPEN_SOURCE>=500
 *   wcstoll/wcstoull:   defined(__c99) || _SGIAPI
 *   vfwscanf/etc:       defined(__c99)
 *
 * We force _SGIAPI=1 to expose _SGIAPI-gated declarations and keep
 * explicit re-declarations for __c99-only functions as fallback.
 *
 * IMPORTANT: Do NOT force __c99 here — IRIX wchar.h includes stdio.h
 * and ctype.h transitively, and __c99=1 causes conflicting declarations
 * (char* vs va_list for vsscanf, static vs extern isblank).
 */

#ifndef _MOGRIX_COMPAT_WCHAR_H
#define _MOGRIX_COMPAT_WCHAR_H

/*
 * Pre-include standards.h so its include guard prevents it from
 * re-defining _SGIAPI inside #include_next.
 */
#include <standards.h>

#pragma push_macro("_SGIAPI")
#pragma push_macro("_NO_ANSIMODE")
#undef _SGIAPI
#define _SGIAPI 1
#undef _NO_ANSIMODE
#define _NO_ANSIMODE 1

#include_next <wchar.h>

#pragma pop_macro("_NO_ANSIMODE")
#pragma pop_macro("_SGIAPI")

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C99 7.24.6 restartable multibyte/wide character conversion functions.
 * Gated by __c99 || __cplusplus || _XOPEN_SOURCE>=500 in IRIX.
 * In C++ mode, __cplusplus makes them visible from IRIX. In C mode,
 * they may not be visible, so we re-declare them (harmless if duplicate).
 */
extern size_t   mbrlen(const char * __restrict, size_t, mbstate_t * __restrict);
extern size_t   mbrtowc(wchar_t * __restrict, const char * __restrict, size_t, mbstate_t * __restrict);
extern size_t   wcrtomb(char * __restrict, wchar_t, mbstate_t * __restrict);
extern size_t   mbsrtowcs(wchar_t * __restrict, const char ** __restrict, size_t, mbstate_t * __restrict);
extern size_t   wcsrtombs(char * __restrict, const wchar_t ** __restrict, size_t, mbstate_t * __restrict);
extern int      mbsinit(const mbstate_t *);
extern wint_t   btowc(int);
extern int      wctob(wint_t);

/* C99 7.24.4 wide memory functions (also hidden by _WCHAR_CORE_EXTENSIONS_1) */
extern wchar_t  *wmemchr(const wchar_t *, wchar_t, size_t);
extern wchar_t  *wmemcpy(wchar_t * __restrict, const wchar_t * __restrict, size_t);
extern wchar_t  *wmemmove(wchar_t *, const wchar_t *, size_t);
extern int       wmemcmp(const wchar_t *, const wchar_t *, size_t);
extern wchar_t  *wmemset(wchar_t *, wchar_t, size_t);

/* C99 7.24.2.7-9 wide character formatted I/O (gated by __c99 on IRIX) */
extern int      vfwscanf(FILE * __restrict, const wchar_t * __restrict, va_list);
extern int      vswscanf(const wchar_t * __restrict, const wchar_t * __restrict, va_list);
extern int      vwscanf(const wchar_t * __restrict, va_list);

/*
 * C99 7.24.4.1 wide string to number conversion.
 * wcstoll/wcstoull are gated by __c99 || _SGIAPI — with our forced
 * _SGIAPI=1, IRIX provides them. Re-declare anyway for safety.
 */
extern float              wcstof(const wchar_t * __restrict, wchar_t ** __restrict);
extern long double        wcstold(const wchar_t * __restrict, wchar_t ** __restrict);
extern long long          wcstoll(const wchar_t * __restrict, wchar_t ** __restrict, int);
extern unsigned long long wcstoull(const wchar_t * __restrict, wchar_t ** __restrict, int);

#ifdef __cplusplus
}
#endif

#endif /* _MOGRIX_COMPAT_WCHAR_H */
