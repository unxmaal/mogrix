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

/* Include the real IRIX stddef.h for other definitions */
#include_next <stddef.h>

/* Re-override offsetof since IRIX header might have redefined it */
#undef offsetof
#define offsetof(type, member) __builtin_offsetof(type, member)

#endif /* _DICL_CLANG_COMPAT_STDDEF_H */
