/*
 * mogrix-compat/generic/math.h
 *
 * Wrapper that includes the real math.h and adds C99 math functions
 * that may be missing from IRIX headers.
 */

#ifndef _MOGRIX_COMPAT_MATH_H
#define _MOGRIX_COMPAT_MATH_H

/* Include the real IRIX math.h */
#include_next <math.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C99 math functions that may be missing from IRIX headers.
 * These are typically available in libm but not declared in headers.
 */

/* log2 - base-2 logarithm (C99) */
#ifndef log2
double log2(double x);
#endif

/* log2f - float version of log2 (C99) */
#ifndef log2f
float log2f(float x);
#endif

/* exp2 - base-2 exponential (C99) */
#ifndef exp2
double exp2(double x);
#endif

/* exp2f - float version of exp2 (C99) */
#ifndef exp2f
float exp2f(float x);
#endif

/* round - round to nearest integer (C99) */
#ifndef round
double round(double x);
#endif

/* roundf - float version of round (C99) */
#ifndef roundf
float roundf(float x);
#endif

/* trunc - truncate toward zero (C99) */
#ifndef trunc
double trunc(double x);
#endif

/* truncf - float version of trunc (C99) */
#ifndef truncf
float truncf(float x);
#endif

/*
 * C99 comparison macros for floating point (may be missing from IRIX)
 */

#ifndef isgreater
#define isgreater(x, y) __builtin_isgreater(x, y)
#endif

#ifndef isgreaterequal
#define isgreaterequal(x, y) __builtin_isgreaterequal(x, y)
#endif

#ifndef isless
#define isless(x, y) __builtin_isless(x, y)
#endif

#ifndef islessequal
#define islessequal(x, y) __builtin_islessequal(x, y)
#endif

#ifndef islessgreater
#define islessgreater(x, y) __builtin_islessgreater(x, y)
#endif

#ifndef isunordered
#define isunordered(x, y) __builtin_isunordered(x, y)
#endif

#ifdef __cplusplus
}
#endif

#endif /* _MOGRIX_COMPAT_MATH_H */
