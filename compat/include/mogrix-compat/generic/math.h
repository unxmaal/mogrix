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

/*
 * C99 float/long double infinity constants (IRIX only has HUGE_VAL for double)
 * These are needed by code that checks for float/long double math function availability
 */
#ifndef HUGE_VALF
#define HUGE_VALF __builtin_huge_valf()
#endif

#ifndef HUGE_VALL
#define HUGE_VALL __builtin_huge_vall()
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C99 math functions that may be missing from IRIX headers.
 * These are typically available in libm but not declared in headers.
 */

/*
 * Float math functions that IRIX has in headers but commented out with #if 0.
 * ldexpf and frexpf are disabled in IRIX math.h - we provide inline wrappers.
 * (modff is available in IRIX - don't redefine)
 */
#ifndef ldexpf
static __inline__ float ldexpf(float x, int exp) { return (float)ldexp((double)x, exp); }
#endif

#ifndef frexpf
static __inline__ float frexpf(float x, int *exp) { return (float)frexp((double)x, exp); }
#endif

/* C99 math functions missing from IRIX headers.
 * These are NOT builtins — IRIX libm lacks log2/exp2/round/trunc entirely.
 * We provide them in compat (log2.c etc.) so they must be declared in
 * both C and C++ mode. */

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

/* round/roundf/trunc/truncf are declared in dicl-clang-compat/math.h —
 * do NOT redeclare here (causes C++ language linkage conflict) */

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
