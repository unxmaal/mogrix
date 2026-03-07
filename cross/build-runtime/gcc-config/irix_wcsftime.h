/* Fix IRIX wcsftime for libstdc++.
 *
 * IRIX wcsftime(3) takes const char* for the format string (XPG4).
 * C++ standard (and libstdc++) expects const wchar_t* (XPG5).
 * IRIX provides _xpg5_wcsftime() for the wchar_t version.
 *
 * We provide a C++ inline overload that calls _xpg5_wcsftime
 * when the format is const wchar_t*, without #define tricks that
 * would conflict with IRIX header declarations.
 */
#ifndef _IRIX_WCSFTIME_H
#define _IRIX_WCSFTIME_H

#ifdef __cplusplus
extern "C" size_t _xpg5_wcsftime(wchar_t *, size_t,
                                  const wchar_t *, const struct tm *);

namespace __irix_compat {
  inline size_t wcsftime_w(wchar_t *s, size_t maxsize,
                           const wchar_t *format, const struct tm *timeptr) {
    return _xpg5_wcsftime(s, maxsize, format, timeptr);
  }
}
#endif

#endif /* _IRIX_WCSFTIME_H */
