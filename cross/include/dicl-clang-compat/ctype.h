/*
 * DICL clang compat ctype.h
 * Wrapper that includes the real IRIX ctype.h and provides isblank() for C mode.
 *
 * IRIX internal/ctype_core.h declares isblank() only when __c99 is defined.
 * irix-cxx-libcxx defines __c99, so C++ code sees it from IRIX headers.
 * irix-cc does NOT define __c99 (it causes vsscanf conflicts in stdio_core.h),
 * so C code needs our inline fallback.
 */
#ifndef _DICL_CTYPE_H
#define _DICL_CTYPE_H

/* Include IRIX ctype.h first */
#include_next <ctype.h>

/* Provide isblank() for C mode where __c99 is not defined.
 * When __c99 IS defined (C++ via irix-cxx-libcxx), IRIX provides it natively
 * and our inline would conflict with the extern declaration. */
#if !defined(__c99) && !defined(isblank)
#ifdef __cplusplus
extern "C" {
#endif
static __inline__ int isblank(int c)
{
    return (c == ' ' || c == '\t');
}
#ifdef __cplusplus
}
#endif
#endif

#endif /* _DICL_CTYPE_H */
