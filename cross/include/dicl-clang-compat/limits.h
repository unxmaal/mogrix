/*
 * DICL clang compat limits.h
 * Include the real IRIX limits.h and add C99 long long limits
 * that IRIX guards behind __c99 (MIPSpro flag)
 */
#ifndef _DICL_LIMITS_H
#define _DICL_LIMITS_H

#include_next <limits.h>

/* C99 long long limits — IRIX has these but guards them behind __c99 */
#ifndef LLONG_MIN
#define LLONG_MIN   (-9223372036854775807LL-1LL)
#endif

#ifndef LLONG_MAX
#define LLONG_MAX   9223372036854775807LL
#endif

#ifndef ULLONG_MAX
#define ULLONG_MAX  18446744073709551615ULL
#endif

#endif /* _DICL_LIMITS_H */
