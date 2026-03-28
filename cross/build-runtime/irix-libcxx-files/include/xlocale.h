//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCPP_SUPPORT_IRIX_XLOCALE_H
#define _LIBCPP_SUPPORT_IRIX_XLOCALE_H

#include <cstdlib>
#include <clocale>
#include <cwctype>
#include <ctype.h>
#include <cstdarg>

#include <__support/xlocale/__nop_locale_mgmt.h>

extern "C" {
size_t __bsd_wcsnrtombs(char *__dest, const wchar_t **__src, size_t __nwc,
    size_t __len, mbstate_t *__ps);

size_t __bsd_mbsnrtowcs(wchar_t * __dest, const char **__src, size_t __nms,
    size_t __len, mbstate_t *__ps);
}

static inline 
size_t wcsnrtombs(char *__dest, const wchar_t **__src, size_t __nwc,
    size_t __len, mbstate_t *__ps)
{
  return __bsd_wcsnrtombs(__dest, __src, __nwc, __len, __ps);
}

static inline
size_t mbsnrtowcs(wchar_t * __dest, const char **__src, size_t __nms,
    size_t __len, mbstate_t *__ps)
{
  return __bsd_mbsnrtowcs(__dest, __src, __nms, __len, __ps);
}

// posix_l_fallback wants wcsnrtombs / mbsnrtowcs
//
#include <__support/xlocale/__posix_l_fallback.h>
#include <__support/xlocale/__strtonum_fallback.h>

// Use __attribute__((weak)) so compat/stdlib/vasprintf.c can override
__attribute__((weak)) inline
int vasprintf(char **strp, const char *fmt, va_list ap)
{
  const int buf_size = 256;
  int str_size;
  if ((*strp = (char *)malloc(buf_size)) == NULL)
  {
    return -1;
  }
  if ((str_size = vsnprintf(*strp, buf_size, fmt,  ap)) >= buf_size)
  {
    if ((*strp = (char *)realloc(*strp, str_size + 1)) == NULL)
    {
      return -1;
    }
    str_size = vsnprintf(*strp, str_size + 1, fmt,  ap);
  }
  return str_size;
}

#endif // _LIBCPP_SUPPORT_IRIX_XLOCALE_H
