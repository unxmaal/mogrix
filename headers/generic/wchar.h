#ifndef _DICL_CLANG_COMPAT_WCHAR_H
#define _DICL_CLANG_COMPAT_WCHAR_H

/*
 * IRIX wchar.h guards C99 functions like mbrlen, mbrtowc, wcrtomb, etc.
 * behind __c99 (MIPSpro) or _XOPEN_SOURCE >= 500. Force exposure for clang.
 */
#ifndef _WCHAR_CORE_EXTENSIONS_1
#define _WCHAR_CORE_EXTENSIONS_1 1
#endif

#include_next <wchar.h>

#endif /* _DICL_CLANG_COMPAT_WCHAR_H */
