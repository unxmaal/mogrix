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

/*
 * max_align_t - C11/C++11 type with maximum fundamental alignment.
 * IRIX predates C11 and doesn't define this. Required by libstdc++ <cstddef>.
 * On MIPS n32: max fundamental alignment is 8 (double, long long).
 */
#ifndef __max_align_t_defined
#define __max_align_t_defined
typedef union {
    long long __ll;
    double __d;
} max_align_t;
#endif

#endif /* _DICL_CLANG_COMPAT_STDDEF_H */
