/* Force-included BEFORE any IRIX headers to fix C++ compilation issues.
   Must be included via -include BEFORE -isystem paths take effect. */
#ifndef MOGRIX_IRIX_CXX_COMPAT_H
#define MOGRIX_IRIX_CXX_COMPAT_H

/* IRIX sgimacros.h (line 68-70) does:
     #undef __restrict
     #define __restrict restrict

   In C++ mode, 'restrict' is treated as a qualifier by clang. Two 'restrict'
   qualifiers on the same parameter is a "redefinition of parameter" error.
   e.g.: extern void *memcpy(void * __restrict, const void * __restrict, size_t);
   expands to: extern void *memcpy(void * restrict, const void * restrict, size_t);
   which clang C++ rejects.

   Fix: define 'restrict' to __restrict__ BEFORE sgimacros.h is included.
   __restrict__ is clang's built-in C++ extension qualifier that doesn't
   cause duplicate-qualifier errors. So the expansion becomes:
     __restrict → restrict → __restrict__
   and clang handles __restrict__ correctly in C++ mode. */
#ifdef __cplusplus
#define restrict __restrict__
#endif

/* IRIX inttypes.h lacks C99 format macros.
   N32 ABI: pointers are 32-bit, long long is 64-bit. */
#ifndef PRIdPTR
#define PRIdPTR "d"
#endif
#ifndef PRIxPTR
#define PRIxPTR "x"
#endif
#ifndef PRIuPTR
#define PRIuPTR "u"
#endif
#ifndef PRIx64
#define PRIx64 "llx"
#endif
#ifndef PRIu64
#define PRIu64 "llu"
#endif
#ifndef PRId64
#define PRId64 "lld"
#endif
#ifndef PRIx32
#define PRIx32 "x"
#endif
#ifndef PRIu32
#define PRIu32 "u"
#endif
#ifndef PRId32
#define PRId32 "d"
#endif

/* IRIX lacks CLOCK_MONOTONIC. Map to CLOCK_SGI_CYCLE (free-running HW counter,
   closest to monotonic available on IRIX). Falls back to CLOCK_REALTIME=1
   if CLOCK_SGI_CYCLE is unavailable. */
#ifndef CLOCK_MONOTONIC
#define CLOCK_MONOTONIC 2  /* == CLOCK_SGI_CYCLE */
#endif

/* Guard C constructs from the assembler — .S files include this via -include */
#ifndef __ASSEMBLER__

/* IRIX header ordering: timespec_t must be defined before sys/types.h
   triggers the chain: sys/types.h → bsd_types.h → select.h → time.h →
   time_core.h → needs timespec_t. Define it minimally here, with the
   correct guard so sys/timespec.h becomes a no-op. */
#ifndef _SYS_TIMESPEC_H
#define _SYS_TIMESPEC_H
/* Can't include sys/types.h here (circular). Use long for time_t. */
#define __timespec timespec
typedef struct __timespec {
    long tv_sec;
    long tv_nsec;
} timespec_t;
#endif

/* IRIX math.h lacks C99 float-specific math functions and classification
   macros. libc++ math.h wrappers call ::frexpf etc. in the global namespace.
   We must declare these BEFORE libc++ math.h is included. Include IRIX's
   math.h directly (via #include_next to skip libc++ wrapper) to get the
   double versions, then provide float wrappers. */
#ifdef __cplusplus
extern "C" {
/* Forward-declare what we need from IRIX math.h */
extern double frexp(double, int*);
extern double ldexp(double, int);
extern double log(double);
extern double log10(double);
extern double modf(double, double*);
extern int finite(double);
/* C99 float math — delegate to double versions */
#ifndef frexpf
inline float frexpf(float x, int* e) { return (float)frexp((double)x, e); }
#endif
#ifndef ldexpf
inline float ldexpf(float x, int e) { return (float)ldexp((double)x, e); }
#endif
#ifndef logf
inline float logf(float x) { return (float)log((double)x); }
#endif
#ifndef log10f
inline float log10f(float x) { return (float)log10((double)x); }
#endif
#ifndef modff
inline float modff(float x, float* i) { double di; float r = (float)modf((double)x, &di); *i = (float)di; return r; }
#endif
} /* extern "C" */
/* C99 classification functions are in dicl-clang-compat/math.h —
   do NOT define as macros here (conflicts with function definitions). */
#endif /* __cplusplus */

/* IRIX libc lacks strerror_r(). Provide POSIX-compatible wrapper around strerror(). */
#ifndef _IRIX_STRERROR_R_DEFINED
#define _IRIX_STRERROR_R_DEFINED
#ifdef __cplusplus
extern "C" {
#endif
extern char *strerror(int);
static inline int strerror_r(int __errnum, char *__buf, __SIZE_TYPE__ __buflen) {
    const char *__msg = strerror(__errnum);
    if (!__msg) return -1;
    __SIZE_TYPE__ __len = 0;
    while (__msg[__len]) ++__len;
    if (__len >= __buflen) { if (__buflen > 0) { __SIZE_TYPE__ __i; for (__i = 0; __i < __buflen - 1; ++__i) __buf[__i] = __msg[__i]; __buf[__buflen - 1] = '\0'; } return 34; /* ERANGE */ }
    { __SIZE_TYPE__ __i; for (__i = 0; __i <= __len; ++__i) __buf[__i] = __msg[__i]; }
    return 0;
}
#ifdef __cplusplus
}
#endif
#endif /* _IRIX_STRERROR_R_DEFINED */

/* IRIX libc lacks C11 aligned_alloc(). Provide via memalign().
   Use __SIZE_TYPE__ (clang builtin) to avoid #include ordering issues. */
#ifndef _IRIX_ALIGNED_ALLOC_DEFINED
#define _IRIX_ALIGNED_ALLOC_DEFINED
#ifdef __cplusplus
extern "C" {
#endif
extern void *memalign(__SIZE_TYPE__, __SIZE_TYPE__);
static inline void *aligned_alloc(__SIZE_TYPE__ __alignment, __SIZE_TYPE__ __size) {
    return memalign(__alignment, __size);
}
#ifdef __cplusplus
}
#endif
#endif /* _IRIX_ALIGNED_ALLOC_DEFINED */

/* IRIX struct lconv lacks C99/POSIX.1-2001 int_* monetary fields.
   The IRIX header has them #if 0'd out. Map to the non-international
   equivalents (which is what they default to per C99 7.11.2.1). */
#ifndef _IRIX_LCONV_INT_FIELDS
#define _IRIX_LCONV_INT_FIELDS
#define int_p_cs_precedes   p_cs_precedes
#define int_n_cs_precedes   n_cs_precedes
#define int_p_sep_by_space  p_sep_by_space
#define int_n_sep_by_space  n_sep_by_space
#define int_p_sign_posn     p_sign_posn
#define int_n_sign_posn     n_sign_posn
#endif

/* IRIX lacks POSIX.1-2008 *at() functions and related constants.
   These are used by libc++ filesystem support. Provide stubs/defines. */
#ifndef AT_FDCWD
#define AT_FDCWD (-2)
#endif
#ifndef AT_REMOVEDIR
#define AT_REMOVEDIR 1
#endif
#ifndef O_CLOEXEC
#define O_CLOEXEC 0  /* IRIX lacks O_CLOEXEC — no-op, harmless */
#endif
#ifndef O_DIRECTORY
#define O_DIRECTORY 0  /* IRIX lacks O_DIRECTORY — best-effort */
#endif
#ifndef O_NOFOLLOW
#define O_NOFOLLOW 0  /* IRIX lacks O_NOFOLLOW — best-effort */
#endif
#ifndef UTIME_NOW
#define UTIME_NOW  ((1l << 30) - 1l)
#define UTIME_OMIT ((1l << 30) - 2l)
#endif

#endif /* !__ASSEMBLER__ */

#endif /* MOGRIX_IRIX_CXX_COMPAT_H */
