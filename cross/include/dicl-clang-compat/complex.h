/*
 * DICL clang compat complex.h
 * Provide C99 complex types for clang without relying on IRIX complex.h
 * which requires __c99 and causes va_list conflicts.
 */
#ifndef _DICL_COMPLEX_H
#define _DICL_COMPLEX_H

/* C99 complex types - clang supports these natively */
#define complex _Complex
#define _Complex_I (__extension__ 1.0iF)
#define I _Complex_I

/* Complex math functions - declare prototypes */
#ifdef __cplusplus
extern "C" {
#endif

/* Basic complex operations - from C99 7.3 */
double cabs(double _Complex);
double carg(double _Complex);
double cimag(double _Complex);
double creal(double _Complex);
double _Complex conj(double _Complex);
double _Complex cproj(double _Complex);

float cabsf(float _Complex);
float cargf(float _Complex);
float cimagf(float _Complex);
float crealf(float _Complex);
float _Complex conjf(float _Complex);
float _Complex cprojf(float _Complex);

/* Trigonometric functions */
double _Complex cacos(double _Complex);
double _Complex casin(double _Complex);
double _Complex catan(double _Complex);
double _Complex ccos(double _Complex);
double _Complex csin(double _Complex);
double _Complex ctan(double _Complex);

float _Complex cacosf(float _Complex);
float _Complex casinf(float _Complex);
float _Complex catanf(float _Complex);
float _Complex ccosf(float _Complex);
float _Complex csinf(float _Complex);
float _Complex ctanf(float _Complex);

/* Hyperbolic functions */
double _Complex cacosh(double _Complex);
double _Complex casinh(double _Complex);
double _Complex catanh(double _Complex);
double _Complex ccosh(double _Complex);
double _Complex csinh(double _Complex);
double _Complex ctanh(double _Complex);

float _Complex cacoshf(float _Complex);
float _Complex casinhf(float _Complex);
float _Complex catanhf(float _Complex);
float _Complex ccoshf(float _Complex);
float _Complex csinhf(float _Complex);
float _Complex ctanhf(float _Complex);

/* Exponential and logarithmic */
double _Complex cexp(double _Complex);
double _Complex clog(double _Complex);

float _Complex cexpf(float _Complex);
float _Complex clogf(float _Complex);

/* Power functions */
double _Complex cpow(double _Complex, double _Complex);
double _Complex csqrt(double _Complex);

float _Complex cpowf(float _Complex, float _Complex);
float _Complex csqrtf(float _Complex);

#ifdef __cplusplus
}
#endif

#endif /* _DICL_COMPLEX_H */
