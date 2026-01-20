/* stddef.h - replacement for IRIX stddef.h when using clang
 *
 * IRIX's offsetof macro uses __INTADDR__ which doesn't work as a
 * compile-time constant in clang. This provides a standard replacement.
 */

#ifndef _STDDEF_H
#define _STDDEF_H

/* Pull in the original IRIX stddef.h for other definitions */
#include_next <stddef.h>

/* Override the IRIX offsetof with standard version */
#undef offsetof
#ifdef __cplusplus
#define offsetof(type, member) __builtin_offsetof(type, member)
#else
#define offsetof(type, member) ((size_t)&((type *)0)->member)
#endif

#endif /* _STDDEF_H */
