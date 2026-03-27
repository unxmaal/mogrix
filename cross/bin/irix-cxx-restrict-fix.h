/*
 * Fix IRIX __restrict → restrict macro for C++ mode.
 *
 * IRIX sgimacros.h defines __restrict as 'restrict' for C++ mode.
 * Clang C++ treats 'restrict' as a C99 keyword, causing
 * "redefinition of parameter" errors on any IRIX header function
 * that uses __restrict twice (memcpy, strcpy, fprintf, etc.).
 *
 * Fix: redefine __restrict to __restrict__ (clang's C++ qualifier).
 * Must be force-included FIRST before any other header.
 */
#ifndef MOGRIX_IRIX_RESTRICT_FIX_H
#define MOGRIX_IRIX_RESTRICT_FIX_H

/* Pre-include sgimacros.h so we can fix its __restrict macro */
#include <internal/sgimacros.h>

#ifdef __cplusplus
#  ifdef __restrict
#    undef __restrict
#  endif
#  define __restrict __restrict__
#endif

/* Pre-define timespec_t before IRIX headers use it.
 * IRIX time.h defines timespec_t, but libc++ uses struct timespec.
 * This ensures they're the same type. */
#ifndef _SYS_TIMESPEC_H
#define _SYS_TIMESPEC_H
typedef struct timespec {
    long tv_sec;
    long tv_nsec;
} timespec_t;
#endif

/* C99 math classification: libc++ 22 with _LIBCPP_PROVIDES_DEFAULT_RUNE_TABLE
 * handles these via __builtin_* in <cmath>. No stubs needed here. */

#endif /* MOGRIX_IRIX_RESTRICT_FIX_H */
