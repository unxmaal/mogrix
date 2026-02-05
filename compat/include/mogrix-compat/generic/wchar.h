/*
 * mogrix-compat/generic/wchar.h
 *
 * Wrapper that includes the real wchar.h and adds C99 multibyte function
 * declarations hidden behind IRIX's __c99 guard.
 *
 * IRIX wchar_core.h gates mbrlen/mbrtowc/wcrtomb/etc. behind:
 *   #if defined(__c99) || defined(__cplusplus) || (_XOPEN_SOURCE+0 >= 500)
 * Clang doesn't define __c99 (that's SGI MIPSpro), so these are invisible.
 */

#ifndef _MOGRIX_COMPAT_WCHAR_H
#define _MOGRIX_COMPAT_WCHAR_H

/* Include the real IRIX wchar.h */
#include_next <wchar.h>

#ifdef __cplusplus
extern "C" {
#endif

/* C99 7.24.6 restartable multibyte/wide character conversion functions */
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

#ifdef __cplusplus
}
#endif

#endif /* _MOGRIX_COMPAT_WCHAR_H */
