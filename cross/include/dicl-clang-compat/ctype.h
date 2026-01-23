/*
 * DICL clang compat ctype.h
 * Add C99 ctype functions not available in IRIX ctype.h
 */
#ifndef _DICL_CTYPE_H
#define _DICL_CTYPE_H

/* Include IRIX ctype.h first */
#include_next <ctype.h>

#ifdef __cplusplus
extern "C" {
#endif

/* C99 isblank() - test for blank character (space or tab) */
#ifndef isblank
static inline int isblank(int c)
{
    return (c == ' ' || c == '\t');
}
#endif

#ifdef __cplusplus
}
#endif

#endif /* _DICL_CTYPE_H */
