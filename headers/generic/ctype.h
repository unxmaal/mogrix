#ifndef _DICL_CLANG_COMPAT_CTYPE_H
#define _DICL_CLANG_COMPAT_CTYPE_H

/*
 * IRIX ctype.h guards C99 functions like isblank behind __c99 (MIPSpro).
 * Provide isblank for clang.
 */

#include_next <ctype.h>

#ifndef isblank
/* isblank: returns non-zero for space and horizontal tab */
static __inline int isblank(int c) {
    return (c == ' ' || c == '\t');
}
#endif

#endif /* _DICL_CLANG_COMPAT_CTYPE_H */
