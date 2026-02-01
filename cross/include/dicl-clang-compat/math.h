/*
 * DICL clang compat math.h
 * Provide C99 math constants and classification macros missing from IRIX math.h
 */
#ifndef _DICL_MATH_H
#define _DICL_MATH_H

/* Include IRIX math.h first */
#include_next <math.h>

/* C99 special values - use compiler builtins */
#ifndef INFINITY
#define INFINITY (__builtin_inff())
#endif

#ifndef NAN
#define NAN (__builtin_nanf(""))
#endif

#ifndef HUGE_VALF
#define HUGE_VALF (__builtin_huge_valf())
#endif

#ifndef HUGE_VALL
#define HUGE_VALL (__builtin_huge_vall())
#endif

/* C99 classification macros - use compiler builtins */
#ifndef isnan
#define isnan(x) __builtin_isnan(x)
#endif

#ifndef isinf
#define isinf(x) __builtin_isinf(x)
#endif

#ifndef isfinite
#define isfinite(x) __builtin_isfinite(x)
#endif

#ifndef isnormal
#define isnormal(x) __builtin_isnormal(x)
#endif

#ifndef signbit
#define signbit(x) __builtin_signbit(x)
#endif

#ifndef fpclassify
#define FP_NAN       0
#define FP_INFINITE  1
#define FP_ZERO      2
#define FP_SUBNORMAL 3
#define FP_NORMAL    4
#define fpclassify(x) __builtin_fpclassify(FP_NAN, FP_INFINITE, FP_NORMAL, FP_SUBNORMAL, FP_ZERO, x)
#endif

/* C99 comparison macros */
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

/* C99 rounding functions - provide stub declarations if IRIX doesn't have them */
#ifndef round
extern double round(double x);
#endif

#ifndef roundf
extern float roundf(float x);
#endif

#ifndef trunc
extern double trunc(double x);
#endif

#ifndef truncf
extern float truncf(float x);
#endif

/* Scalbn functions - use ldexp which IRIX has */
#ifndef scalbn
#define scalbn(x, n) ldexp(x, n)
#endif

#ifndef scalbnf
#define scalbnf(x, n) ldexp(x, n)
#endif

#ifndef scalbln
#define scalbln(x, n) ldexp(x, (int)(n))
#endif

#ifndef scalblnf
#define scalblnf(x, n) ldexp(x, (int)(n))
#endif

#endif /* _DICL_MATH_H */
