#ifndef _DICL_CLANG_COMPAT_STDINT_H
#define _DICL_CLANG_COMPAT_STDINT_H

/*
 * IRIX stdint.h wrapper for clang.
 *
 * IRIX's stdint.h requires __c99 (MIPSpro define) and has various
 * guarding issues with _XOPEN_SOURCE. We provide our own definitions
 * using clang's predefined macros.
 */

/* Try to include IRIX's stdint.h first */
#ifndef __c99
#define __c99 1
#define _DICL_UNDEF_C99 1
#endif

#include_next <stdint.h>

#ifdef _DICL_UNDEF_C99
#undef __c99
#undef _DICL_UNDEF_C99
#endif

/*
 * Define standard integer types if not already defined.
 * Use clang's predefined type macros.
 */

/* Exact-width types */
#ifndef _INT8_T
#define _INT8_T
typedef __INT8_TYPE__ int8_t;
#endif

#ifndef _UINT8_T
#define _UINT8_T
typedef __UINT8_TYPE__ uint8_t;
#endif

#ifndef _INT16_T
#define _INT16_T
typedef __INT16_TYPE__ int16_t;
#endif

#ifndef _UINT16_T
#define _UINT16_T
typedef __UINT16_TYPE__ uint16_t;
#endif

#ifndef _INT32_T
#define _INT32_T
typedef __INT32_TYPE__ int32_t;
#endif

#ifndef _UINT32_T
#define _UINT32_T
typedef __UINT32_TYPE__ uint32_t;
#endif

#ifndef _INT64_T
#define _INT64_T
typedef __INT64_TYPE__ int64_t;
#endif

#ifndef _UINT64_T
#define _UINT64_T
typedef __UINT64_TYPE__ uint64_t;
#endif

/* Minimum-width types */
#ifndef _INT_LEAST8_T
#define _INT_LEAST8_T
typedef __INT_LEAST8_TYPE__ int_least8_t;
#endif

#ifndef _UINT_LEAST8_T
#define _UINT_LEAST8_T
typedef __UINT_LEAST8_TYPE__ uint_least8_t;
#endif

#ifndef _INT_LEAST16_T
#define _INT_LEAST16_T
typedef __INT_LEAST16_TYPE__ int_least16_t;
#endif

#ifndef _UINT_LEAST16_T
#define _UINT_LEAST16_T
typedef __UINT_LEAST16_TYPE__ uint_least16_t;
#endif

#ifndef _INT_LEAST32_T
#define _INT_LEAST32_T
typedef __INT_LEAST32_TYPE__ int_least32_t;
#endif

#ifndef _UINT_LEAST32_T
#define _UINT_LEAST32_T
typedef __UINT_LEAST32_TYPE__ uint_least32_t;
#endif

#ifndef _INT_LEAST64_T
#define _INT_LEAST64_T
typedef __INT_LEAST64_TYPE__ int_least64_t;
#endif

#ifndef _UINT_LEAST64_T
#define _UINT_LEAST64_T
typedef __UINT_LEAST64_TYPE__ uint_least64_t;
#endif

/* Fast types */
#ifndef _INT_FAST8_T
#define _INT_FAST8_T
typedef __INT_FAST8_TYPE__ int_fast8_t;
#endif

#ifndef _UINT_FAST8_T
#define _UINT_FAST8_T
typedef __UINT_FAST8_TYPE__ uint_fast8_t;
#endif

#ifndef _INT_FAST16_T
#define _INT_FAST16_T
typedef __INT_FAST16_TYPE__ int_fast16_t;
#endif

#ifndef _UINT_FAST16_T
#define _UINT_FAST16_T
typedef __UINT_FAST16_TYPE__ uint_fast16_t;
#endif

#ifndef _INT_FAST32_T
#define _INT_FAST32_T
typedef __INT_FAST32_TYPE__ int_fast32_t;
#endif

#ifndef _UINT_FAST32_T
#define _UINT_FAST32_T
typedef __UINT_FAST32_TYPE__ uint_fast32_t;
#endif

#ifndef _INT_FAST64_T
#define _INT_FAST64_T
typedef __INT_FAST64_TYPE__ int_fast64_t;
#endif

#ifndef _UINT_FAST64_T
#define _UINT_FAST64_T
typedef __UINT_FAST64_TYPE__ uint_fast64_t;
#endif

/* Pointer types */
#ifndef _INTPTR_T
#define _INTPTR_T
typedef __INTPTR_TYPE__ intptr_t;
#endif

#ifndef _UINTPTR_T
#define _UINTPTR_T
typedef __UINTPTR_TYPE__ uintptr_t;
#endif

/* Max types */
#ifndef _INTMAX_T
#define _INTMAX_T
typedef __INTMAX_TYPE__ intmax_t;
#endif

#ifndef _UINTMAX_T
#define _UINTMAX_T
typedef __UINTMAX_TYPE__ uintmax_t;
#endif

/*
 * Limit macros
 */
#ifndef INT8_MAX
# define INT8_MAX __INT8_MAX__
#endif
#ifndef INT8_MIN
# define INT8_MIN (-INT8_MAX - 1)
#endif
#ifndef UINT8_MAX
# define UINT8_MAX __UINT8_MAX__
#endif

#ifndef INT16_MAX
# define INT16_MAX __INT16_MAX__
#endif
#ifndef INT16_MIN
# define INT16_MIN (-INT16_MAX - 1)
#endif
#ifndef UINT16_MAX
# define UINT16_MAX __UINT16_MAX__
#endif

#ifndef INT32_MAX
# define INT32_MAX __INT32_MAX__
#endif
#ifndef INT32_MIN
# define INT32_MIN (-INT32_MAX - 1)
#endif
#ifndef UINT32_MAX
# define UINT32_MAX __UINT32_MAX__
#endif

#ifndef INT64_MAX
# define INT64_MAX __INT64_MAX__
#endif
#ifndef INT64_MIN
# define INT64_MIN (-INT64_MAX - 1)
#endif
#ifndef UINT64_MAX
# define UINT64_MAX __UINT64_MAX__
#endif

#ifndef INTMAX_MAX
# define INTMAX_MAX __INTMAX_MAX__
#endif
#ifndef INTMAX_MIN
# define INTMAX_MIN (-INTMAX_MAX - 1)
#endif
#ifndef UINTMAX_MAX
# define UINTMAX_MAX __UINTMAX_MAX__
#endif

#ifndef INTPTR_MAX
# define INTPTR_MAX __INTPTR_MAX__
#endif
#ifndef INTPTR_MIN
# define INTPTR_MIN (-INTPTR_MAX - 1)
#endif
#ifndef UINTPTR_MAX
# define UINTPTR_MAX __UINTPTR_MAX__
#endif

#ifndef SIZE_MAX
# define SIZE_MAX __SIZE_MAX__
#endif

#ifndef PTRDIFF_MAX
# define PTRDIFF_MAX __PTRDIFF_MAX__
#endif
#ifndef PTRDIFF_MIN
# define PTRDIFF_MIN (-PTRDIFF_MAX - 1)
#endif

/*
 * Constant macros
 */
#ifndef INT8_C
# define INT8_C(c) c
#endif
#ifndef UINT8_C
# define UINT8_C(c) c
#endif
#ifndef INT16_C
# define INT16_C(c) c
#endif
#ifndef UINT16_C
# define UINT16_C(c) c
#endif
#ifndef INT32_C
# define INT32_C(c) c
#endif
#ifndef UINT32_C
# define UINT32_C(c) c ## U
#endif
#ifndef INT64_C
# define INT64_C(c) c ## LL
#endif
#ifndef UINT64_C
# define UINT64_C(c) c ## ULL
#endif
#ifndef INTMAX_C
# define INTMAX_C(c) c ## LL
#endif
#ifndef UINTMAX_C
# define UINTMAX_C(c) c ## ULL
#endif

#endif /* _DICL_CLANG_COMPAT_STDINT_H */
