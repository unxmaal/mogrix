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

/* Filesystem support: IRIX has POSIX filesystem headers but no d_type */
#define _GLIBCXX_HAVE_DIRENT_H 1
#define _GLIBCXX_HAVE_FCNTL_H 1
#define _GLIBCXX_HAVE_SYS_STAT_H 1
#define _GLIBCXX_HAVE_SYS_STATVFS_H 1
#define _GLIBCXX_HAVE_UTIME_H 1
/* IRIX struct dirent has no d_type field */
/* _GLIBCXX_HAVE_STRUCT_DIRENT_D_TYPE is intentionally NOT defined */

/* IRIX has no aligned_alloc (C11) — must undef, not set to 0, because
   <cstdlib> uses #ifdef. IRIX has memalign and posix_memalign instead. */
#undef _GLIBCXX_HAVE_ALIGNED_ALLOC
#define _GLIBCXX_HAVE_POSIX_MEMALIGN 1
#define _GLIBCXX_HAVE_MEMALIGN 1

#endif /* _LIBSTDCXX_COMPAT_H */
