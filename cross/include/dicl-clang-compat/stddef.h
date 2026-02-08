/*
 * dicl-clang-compat/stddef.h
 *
 * IRIX stddef.h uses __INTADDR__ (MIPSpro builtin) for offsetof.
 * Clang doesn't have __INTADDR__, so we provide a clang-compatible version.
 */

#ifndef _DICL_CLANG_COMPAT_STDDEF_H
#define _DICL_CLANG_COMPAT_STDDEF_H

/* Use clang's builtin offsetof */
#undef offsetof
#define offsetof(type, member) __builtin_offsetof(type, member)

#ifdef __cplusplus
/* In C++, wchar_t is a built-in type. Prevent IRIX stddef_core.h from
   trying to typedef it (causes 'long wchar_t is invalid' error). */
#define _WCHAR_T
#endif

/* Include the real IRIX stddef.h for other definitions */
#include_next <stddef.h>

/* Re-override offsetof since IRIX header might have redefined it */
#undef offsetof
#define offsetof(type, member) __builtin_offsetof(type, member)

#endif /* _DICL_CLANG_COMPAT_STDDEF_H */
