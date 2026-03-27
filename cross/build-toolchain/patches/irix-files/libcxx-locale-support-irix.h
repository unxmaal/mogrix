// -*- C++ -*-
//===-----------------------------------------------------------------------===//
//
// IRIX locale support for libc++
//
// Maps libc++ __locale::* API to IRIX xlocale stubs from mogrix compat.
// Based on the Linux backend with modifications for IRIX's lack of
// native xlocale support.
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCPP___LOCALE_DIR_SUPPORT_IRIX_H
#define _LIBCPP___LOCALE_DIR_SUPPORT_IRIX_H

#include <__config>
#include <__cstddef/size_t.h>
#include <__std_mbstate_t.h>
#include <__utility/forward.h>
#include <clocale> // std::lconv
#include <cstdio>
#include <cstdlib>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#if _LIBCPP_HAS_WIDE_CHARACTERS
#  include <cwchar>
#  include <wctype.h>
#endif

// Include our xlocale stubs
#include <locale/xlocale_irix.h>

#if !defined(_LIBCPP_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCPP_BEGIN_NAMESPACE_STD
namespace __locale {

struct __locale_guard {
  _LIBCPP_HIDE_FROM_ABI __locale_guard(locale_t& __loc) : __old_loc_(::uselocale(__loc)) {}

  _LIBCPP_HIDE_FROM_ABI ~__locale_guard() {
    if (__old_loc_)
      ::uselocale(__old_loc_);
  }

  locale_t __old_loc_;

  __locale_guard(__locale_guard const&)            = delete;
  __locale_guard& operator=(__locale_guard const&) = delete;
};

//
// Locale management
//
#define _LIBCPP_COLLATE_MASK LC_COLLATE_MASK
#define _LIBCPP_CTYPE_MASK LC_CTYPE_MASK
#define _LIBCPP_MONETARY_MASK LC_MONETARY_MASK
#define _LIBCPP_NUMERIC_MASK LC_NUMERIC_MASK
#define _LIBCPP_TIME_MASK LC_TIME_MASK
#define _LIBCPP_MESSAGES_MASK LC_MESSAGES_MASK
#define _LIBCPP_ALL_MASK LC_ALL_MASK
#define _LIBCPP_LC_ALL LC_ALL

using __locale_t _LIBCPP_NODEBUG = ::locale_t;

#if defined(_LIBCPP_BUILDING_LIBRARY)
using __lconv_t _LIBCPP_NODEBUG = std::lconv;

inline _LIBCPP_HIDE_FROM_ABI __locale_t __newlocale(int __category_mask, const char* __locale, __locale_t __base) {
  return ::newlocale(__category_mask, __locale, __base);
}

inline _LIBCPP_HIDE_FROM_ABI void __freelocale(__locale_t __loc) { ::freelocale(__loc); }

inline _LIBCPP_HIDE_FROM_ABI char* __setlocale(int __category, char const* __locale) {
  return ::setlocale(__category, __locale);
}

inline _LIBCPP_HIDE_FROM_ABI __lconv_t* __localeconv(__locale_t& __loc) {
  __locale_guard __current(__loc);
  return std::localeconv();
}
#endif // _LIBCPP_BUILDING_LIBRARY

//
// Strtonum functions
//
inline _LIBCPP_HIDE_FROM_ABI float __strtof(const char* __nptr, char** __endptr, __locale_t __loc) {
  return ::strtof_l(__nptr, __endptr, __loc);
}

inline _LIBCPP_HIDE_FROM_ABI double __strtod(const char* __nptr, char** __endptr, __locale_t __loc) {
  return ::strtod_l(__nptr, __endptr, __loc);
}

inline _LIBCPP_HIDE_FROM_ABI long double __strtold(const char* __nptr, char** __endptr, __locale_t __loc) {
  return ::strtold_l(__nptr, __endptr, __loc);
}

//
// Character manipulation functions
//
#if defined(_LIBCPP_BUILDING_LIBRARY)
inline _LIBCPP_HIDE_FROM_ABI int __toupper(int __c, __locale_t __loc) { return toupper_l(__c, __loc); }

inline _LIBCPP_HIDE_FROM_ABI int __tolower(int __c, __locale_t __loc) { return tolower_l(__c, __loc); }

inline _LIBCPP_HIDE_FROM_ABI int __strcoll(const char* __s1, const char* __s2, __locale_t __loc) {
  return strcoll_l(__s1, __s2, __loc);
}

inline _LIBCPP_HIDE_FROM_ABI size_t __strxfrm(char* __dest, const char* __src, size_t __n, __locale_t __loc) {
  return strxfrm_l(__dest, __src, __n, __loc);
}

#  if _LIBCPP_HAS_WIDE_CHARACTERS
inline _LIBCPP_HIDE_FROM_ABI int __iswctype(wint_t __c, wctype_t __type, __locale_t __loc) {
  return iswctype_l(__c, __type, __loc);
}

inline _LIBCPP_HIDE_FROM_ABI int __iswspace(wint_t __c, __locale_t __loc) { return iswspace_l(__c, __loc); }

inline _LIBCPP_HIDE_FROM_ABI int __iswprint(wint_t __c, __locale_t __loc) { return iswprint_l(__c, __loc); }

inline _LIBCPP_HIDE_FROM_ABI int __iswcntrl(wint_t __c, __locale_t __loc) { return iswcntrl_l(__c, __loc); }

inline _LIBCPP_HIDE_FROM_ABI int __iswupper(wint_t __c, __locale_t __loc) { return iswupper_l(__c, __loc); }

inline _LIBCPP_HIDE_FROM_ABI int __iswlower(wint_t __c, __locale_t __loc) { return iswlower_l(__c, __loc); }

inline _LIBCPP_HIDE_FROM_ABI int __iswalpha(wint_t __c, __locale_t __loc) { return iswalpha_l(__c, __loc); }

inline _LIBCPP_HIDE_FROM_ABI int __iswblank(wint_t __c, __locale_t __loc) { return iswblank_l(__c, __loc); }

inline _LIBCPP_HIDE_FROM_ABI int __iswdigit(wint_t __c, __locale_t __loc) { return iswdigit_l(__c, __loc); }

inline _LIBCPP_HIDE_FROM_ABI int __iswpunct(wint_t __c, __locale_t __loc) { return iswpunct_l(__c, __loc); }

inline _LIBCPP_HIDE_FROM_ABI int __iswxdigit(wint_t __c, __locale_t __loc) { return iswxdigit_l(__c, __loc); }

inline _LIBCPP_HIDE_FROM_ABI wint_t __towupper(wint_t __c, __locale_t __loc) { return towupper_l(__c, __loc); }

inline _LIBCPP_HIDE_FROM_ABI wint_t __towlower(wint_t __c, __locale_t __loc) { return towlower_l(__c, __loc); }

inline _LIBCPP_HIDE_FROM_ABI int __wcscoll(const wchar_t* __s1, const wchar_t* __s2, __locale_t __loc) {
  return wcscoll_l(__s1, __s2, __loc);
}

inline _LIBCPP_HIDE_FROM_ABI size_t __wcsxfrm(wchar_t* __dest, const wchar_t* __src, size_t __n, __locale_t __loc) {
  return wcsxfrm_l(__dest, __src, __n, __loc);
}
#  endif // _LIBCPP_HAS_WIDE_CHARACTERS

//
// Other locale functions
//
inline _LIBCPP_HIDE_FROM_ABI size_t
__mbsrtowcs(wchar_t* __dest, const char** __src, size_t __len, mbstate_t* __ps, __locale_t __loc) {
  return mbsrtowcs_l(__dest, __src, __len, __ps, __loc);
}

inline _LIBCPP_HIDE_FROM_ABI size_t
__mbrtowc(wchar_t* __pwc, const char* __s, size_t __n, mbstate_t* __ps, __locale_t __loc) {
  return mbrtowc_l(__pwc, __s, __n, __ps, __loc);
}

inline _LIBCPP_HIDE_FROM_ABI int __mbtowc(wchar_t* __pwc, const char* __pmb, size_t __max, __locale_t __loc) {
  return mbtowc_l(__pwc, __pmb, __max, __loc);
}

inline _LIBCPP_HIDE_FROM_ABI size_t __mbrlen(const char* __s, size_t __n, mbstate_t* __ps, __locale_t __loc) {
  return mbrlen_l(__s, __n, __ps, __loc);
}

inline _LIBCPP_HIDE_FROM_ABI size_t
__wcrtomb(char* __s, wchar_t __wc, mbstate_t* __ps, __locale_t __loc) {
  return wcrtomb_l(__s, __wc, __ps, __loc);
}

inline _LIBCPP_HIDE_FROM_ABI int __snprintf(char* __s, size_t __n, __locale_t __loc, const char* __format, ...) {
  va_list __va;
  va_start(__va, __format);
  (void)__loc;
  int __res = ::vsnprintf(__s, __n, __format, __va);
  va_end(__va);
  return __res;
}

inline _LIBCPP_HIDE_FROM_ABI int __sscanf(const char* __s, __locale_t __loc, const char* __format, ...) {
  va_list __va;
  va_start(__va, __format);
  (void)__loc;
  int __res = ::vsscanf(__s, __format, __va);
  va_end(__va);
  return __res;
}
#endif // _LIBCPP_BUILDING_LIBRARY

} // namespace __locale
_LIBCPP_END_NAMESPACE_STD

#endif // _LIBCPP___LOCALE_DIR_SUPPORT_IRIX_H
