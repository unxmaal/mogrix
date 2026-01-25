/*
 * mogrix-compat/generic/stdint.h
 *
 * Wrapper that includes the real stdint.h and adds missing C99 macros
 * for IRIX compatibility.
 */

#ifndef _MOGRIX_COMPAT_STDINT_H
#define _MOGRIX_COMPAT_STDINT_H

/* Ensure constant macros are defined */
#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS 1
#endif

/* Include the real stdint.h */
#include_next <stdint.h>

/*
 * C99 integer constant macros
 *
 * These macros create integer constants with the proper type suffix.
 * IRIX 6.5 stdint.h may not have these.
 */

/* Signed integer constant macros */
#ifndef INT8_C
#define INT8_C(c)   c
#endif

#ifndef INT16_C
#define INT16_C(c)  c
#endif

#ifndef INT32_C
#define INT32_C(c)  c
#endif

#ifndef INT64_C
#define INT64_C(c)  c ## LL
#endif

/* Unsigned integer constant macros */
#ifndef UINT8_C
#define UINT8_C(c)  c
#endif

#ifndef UINT16_C
#define UINT16_C(c) c
#endif

#ifndef UINT32_C
#define UINT32_C(c) c ## U
#endif

#ifndef UINT64_C
#define UINT64_C(c) c ## ULL
#endif

/* Maximum-width integer constant macros */
#ifndef INTMAX_C
#define INTMAX_C(c)  c ## LL
#endif

#ifndef UINTMAX_C
#define UINTMAX_C(c) c ## ULL
#endif

#endif /* _MOGRIX_COMPAT_STDINT_H */
