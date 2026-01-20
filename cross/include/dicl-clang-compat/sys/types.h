/* sys/types.h - wrapper for IRIX sys/types.h
 *
 * Ensures sigset_t is always defined and prevents wchar_t typedef
 * conflict in C++ mode.
 */
#ifndef _CLANG_COMPAT_SYS_TYPES_H
#define _CLANG_COMPAT_SYS_TYPES_H

/* In C++, wchar_t is a built-in type. IRIX sys/types.h tries to typedef it
 * which causes an error. Define _WCHAR_T to prevent the typedef. */
#ifdef __cplusplus
#ifndef _WCHAR_T
#define _WCHAR_T
#endif
#endif

/* Include the original sys/types.h */
#include_next <sys/types.h>

/* Define sigset_t if not already defined */
#ifndef _SIGSET_T
#define _SIGSET_T
typedef struct {
    __uint32_t __sigbits[4];
} sigset_t;
#endif

#endif /* _CLANG_COMPAT_SYS_TYPES_H */
