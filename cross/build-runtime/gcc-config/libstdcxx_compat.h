/* Compatibility header for building libstdc++ with clang for IRIX n32 */
#ifndef _LIBSTDCXX_COMPAT_H
#define _LIBSTDCXX_COMPAT_H

/* Override SGI namespace macros — libstdc++ handles namespaces itself */
#define __SGI_LIBC_NAMESPACE_QUALIFIER
#define __SGI_LIBC_BEGIN_NAMESPACE_STD
#define __SGI_LIBC_END_NAMESPACE_STD
#define __SGI_LIBC_USING_FROM_STD(x)

/* Fix __restrict conflict in C++ mode:
   IRIX sgimacros.h defines __restrict as 'restrict' when __c99 is defined,
   but 'restrict' is not a C++ keyword. Use clang's __restrict__ instead. */
#define __restrict __restrict__

/* Disable wchar C99 features that IRIX libc doesn't have */
#undef _GLIBCXX_HAVE_VFWSCANF
#undef _GLIBCXX_HAVE_VSWSCANF
#undef _GLIBCXX_HAVE_VWSCANF
#undef _GLIBCXX_HAVE_WCSTOF
#define _GLIBCXX_USE_C99_WCHAR 0
#define _GLIBCXX11_USE_C99_WCHAR 0
#define _GLIBCXX98_USE_C99_WCHAR 0

#endif /* _LIBSTDCXX_COMPAT_H */
