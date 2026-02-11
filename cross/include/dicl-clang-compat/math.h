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

/* FP_* classification constants — needed by both C and C++ */
#ifndef FP_NAN
#define FP_NAN       0
#define FP_INFINITE  1
#define FP_ZERO      2
#define FP_SUBNORMAL 3
#define FP_NORMAL    4
#endif

/* C99 classification — two modes depending on language:
 *
 * C mode: define as function-like macros (standard C99 approach).
 *
 * C++ mode: Only provide isinf() as global inline function.
 * - isinf: _GLIBCXX_HAVE_OBSOLETE_ISINF=1 makes cmath do `using ::isinf;`
 *   so we must provide a global-scope declaration.
 * - isnan: IRIX math.h already declares `int isnan(double)`.
 * - isfinite/isnormal/signbit: cmath defines its own constexpr versions.
 *   We must NOT provide global-scope functions for these because GCC 9's
 *   C++ math.h wrapper does `using std::isfinite;` which conflicts with
 *   any pre-existing global-scope declaration (different entity). */
#ifdef __cplusplus

/* Only isinf needed: _GLIBCXX_HAVE_OBSOLETE_ISINF=1 makes cmath do
 * `using ::isinf;` — needs a global-scope function to pull into std::.
 * isnan: IRIX math.h already declares int isnan(double).
 * isfinite/isnormal/signbit: cmath defines its own constexpr versions
 * in std:: — providing our own at global scope conflicts with
 * `using std::isfinite;` in GCC 9's C++ math.h wrapper. */
static inline int isinf(double x) { return __builtin_isinf(x); }

#else /* C mode */

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

#endif /* __cplusplus / C mode */

/* C99 type aliases */
#ifndef _MATH_FLOAT_T
#define _MATH_FLOAT_T
typedef float  float_t;
typedef double double_t;
#endif

/* ================================================================
 * C99 math functions missing from IRIX libm.
 * All implemented via compiler builtins (no library dependency).
 * IRIX has: hypot, cbrt, lgamma, erf, erfc, asinh, acosh, atanh,
 *           log1p, expm1, nextafter, remainder, copysign, trunc,
 *           rint, logb, ilogb, scalb
 * IRIX lacks everything below.
 * ================================================================ */

/* --- exp2 / log2 --- */
static __inline double exp2(double x) { return __builtin_exp2(x); }
static __inline float exp2f(float x) { return __builtin_exp2f(x); }
static __inline long double exp2l(long double x) { return __builtin_exp2l(x); }

static __inline double log2(double x) { return __builtin_log2(x); }
static __inline float log2f(float x) { return __builtin_log2f(x); }
static __inline long double log2l(long double x) { return __builtin_log2l(x); }

/* --- rounding --- */
static __inline double round(double x) { return __builtin_round(x); }
static __inline float roundf(float x) { return __builtin_roundf(x); }
static __inline long double roundl(long double x) { return __builtin_roundl(x); }

static __inline long lround(double x) { return __builtin_lround(x); }
static __inline long lroundf(float x) { return __builtin_lroundf(x); }
static __inline long lroundl(long double x) { return __builtin_lroundl(x); }

static __inline long long llround(double x) { return __builtin_llround(x); }
static __inline long long llroundf(float x) { return __builtin_llroundf(x); }
static __inline long long llroundl(long double x) { return __builtin_llroundl(x); }

static __inline long lrint(double x) { return __builtin_lrint(x); }
static __inline long lrintf(float x) { return __builtin_lrintf(x); }
static __inline long lrintl(long double x) { return __builtin_lrintl(x); }

static __inline long long llrint(double x) { return __builtin_llrint(x); }
static __inline long long llrintf(float x) { return __builtin_llrintf(x); }
static __inline long long llrintl(long double x) { return __builtin_llrintl(x); }

static __inline double nearbyint(double x) { return __builtin_nearbyint(x); }
static __inline float nearbyintf(float x) { return __builtin_nearbyintf(x); }
static __inline long double nearbyintl(long double x) { return __builtin_nearbyintl(x); }

/* --- fdim / fmax / fmin --- */
static __inline double fdim(double x, double y) { return __builtin_fdim(x, y); }
static __inline float fdimf(float x, float y) { return __builtin_fdimf(x, y); }
static __inline long double fdiml(long double x, long double y) { return __builtin_fdiml(x, y); }

static __inline double fmax(double x, double y) { return __builtin_fmax(x, y); }
static __inline float fmaxf(float x, float y) { return __builtin_fmaxf(x, y); }
static __inline long double fmaxl(long double x, long double y) { return __builtin_fmaxl(x, y); }

static __inline double fmin(double x, double y) { return __builtin_fmin(x, y); }
static __inline float fminf(float x, float y) { return __builtin_fminf(x, y); }
static __inline long double fminl(long double x, long double y) { return __builtin_fminl(x, y); }

/* --- fma --- */
static __inline double fma(double x, double y, double z) { return __builtin_fma(x, y, z); }
static __inline float fmaf(float x, float y, float z) { return __builtin_fmaf(x, y, z); }
static __inline long double fmal(long double x, long double y, long double z) { return __builtin_fmal(x, y, z); }

/* --- tgamma --- */
static __inline double tgamma(double x) { return __builtin_tgamma(x); }
static __inline float tgammaf(float x) { return __builtin_tgammaf(x); }
static __inline long double tgammal(long double x) { return __builtin_tgammal(x); }

/* --- nan --- */
static __inline double nan(const char *s) { return __builtin_nan(s); }
static __inline float nanf(const char *s) { return __builtin_nanf(s); }
static __inline long double nanl(const char *s) { return __builtin_nanl(s); }

/* --- nexttoward --- */
static __inline double nexttoward(double x, long double y) { return __builtin_nexttoward(x, y); }
static __inline float nexttowardf(float x, long double y) { return __builtin_nexttowardf(x, y); }
static __inline long double nexttowardl(long double x, long double y) { return __builtin_nexttowardl(x, y); }

/* --- remquo --- */
static __inline double remquo(double x, double y, int *q) { return __builtin_remquo(x, y, q); }
static __inline float remquof(float x, float y, int *q) { return __builtin_remquof(x, y, q); }
static __inline long double remquol(long double x, long double y, int *q) { return __builtin_remquol(x, y, q); }

/* --- scalbn / scalbln --- */
static __inline double scalbn(double x, int n) { return __builtin_scalbn(x, n); }
static __inline float scalbnf(float x, int n) { return __builtin_scalbnf(x, n); }
static __inline long double scalbnl(long double x, int n) { return __builtin_scalbnl(x, n); }

static __inline double scalbln(double x, long n) { return __builtin_scalbln(x, n); }
static __inline float scalblnf(float x, long n) { return __builtin_scalblnf(x, n); }
static __inline long double scalblnl(long double x, long n) { return __builtin_scalblnl(x, n); }

/* --- float/long double variants missing from IRIX ---
 * IRIX has old-style SGI names (facosh, fatanh, etc.) but NOT
 * the C99 standard names (acoshf, atanhf, etc.).
 * IRIX DOES have: expm1f, log1pf, truncf, hypotf — do NOT redefine those.
 * For functions IRIX declares as `extern`, we can't use static inline
 * (linkage conflict), so only add functions IRIX completely lacks. */
static __inline float rintf(float x) { return __builtin_rintf(x); }
static __inline float logbf(float x) { return __builtin_logbf(x); }
static __inline int ilogbf(float x) { return __builtin_ilogbf(x); }
static __inline int ilogbl(long double x) { return __builtin_ilogbl(x); }
static __inline float remainderf(float x, float y) { return __builtin_remainderf(x, y); }
static __inline long double remainderl(long double x, long double y) { return __builtin_remainderl(x, y); }
static __inline float erff(float x) { return __builtin_erff(x); }
static __inline float erfcf(float x) { return __builtin_erfcf(x); }
static __inline float lgammaf(float x) { return __builtin_lgammaf(x); }
static __inline float asinhf(float x) { return __builtin_asinhf(x); }
static __inline float acoshf(float x) { return __builtin_acoshf(x); }
static __inline float atanhf(float x) { return __builtin_atanhf(x); }
static __inline float cbrtf(float x) { return __builtin_cbrtf(x); }
static __inline float nextafterf(float x, float y) { return __builtin_nextafterf(x, y); }
static __inline float copysignf(float x, float y) { return __builtin_copysignf(x, y); }
/* truncl: IRIX has it (extern long double truncl) — do NOT redefine */

/* --- long double variants missing from IRIX ---
 * IRIX has erfl, erfcl, lgammal, log1pl, copysignl, hypotl, nextafterl,
 * logbl, rintl, truncl — but lacks the following. */
static __inline long double acoshl(long double x) { return __builtin_acoshl(x); }
static __inline long double asinhl(long double x) { return __builtin_asinhl(x); }
static __inline long double atanhl(long double x) { return __builtin_atanhl(x); }
static __inline long double cbrtl(long double x) { return __builtin_cbrtl(x); }
static __inline long double expm1l(long double x) { return __builtin_expm1l(x); }

#endif /* _DICL_MATH_H */
