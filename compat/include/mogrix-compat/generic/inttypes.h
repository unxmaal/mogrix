/*
 * mogrix-compat/generic/inttypes.h
 *
 * Wrapper that includes the real inttypes.h and ensures the PRIx64 etc
 * format macros are defined. Some older systems only define these for C++.
 */

#ifndef _MOGRIX_COMPAT_INTTYPES_H
#define _MOGRIX_COMPAT_INTTYPES_H

/* Ensure format macros are defined for C (not just C++) */
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS 1
#endif

/* Include the real inttypes.h */
#include_next <inttypes.h>

/* C99 requires <inttypes.h> to include <stdint.h>, but IRIX's doesn't.
 * Pull it in explicitly to get SIZE_MAX, UINT64_MAX, etc. */
#include <stdint.h>

/* Provide fallback definitions if the format macros are still missing */

#ifndef PRId8
#define PRId8 "d"
#endif

#ifndef PRId16
#define PRId16 "d"
#endif

#ifndef PRId32
#define PRId32 "d"
#endif

#ifndef PRId64
#define PRId64 "lld"
#endif

#ifndef PRIi8
#define PRIi8 "i"
#endif

#ifndef PRIi16
#define PRIi16 "i"
#endif

#ifndef PRIi32
#define PRIi32 "i"
#endif

#ifndef PRIi64
#define PRIi64 "lli"
#endif

#ifndef PRIu8
#define PRIu8 "u"
#endif

#ifndef PRIu16
#define PRIu16 "u"
#endif

#ifndef PRIu32
#define PRIu32 "u"
#endif

#ifndef PRIu64
#define PRIu64 "llu"
#endif

#ifndef PRIx8
#define PRIx8 "x"
#endif

#ifndef PRIx16
#define PRIx16 "x"
#endif

#ifndef PRIx32
#define PRIx32 "x"
#endif

#ifndef PRIx64
#define PRIx64 "llx"
#endif

#ifndef PRIX8
#define PRIX8 "X"
#endif

#ifndef PRIX16
#define PRIX16 "X"
#endif

#ifndef PRIX32
#define PRIX32 "X"
#endif

#ifndef PRIX64
#define PRIX64 "llX"
#endif

/* C99 intmax types/functions required by C++ <cinttypes>.
 * IRIX inttypes.h has strtoimax/strtoumax but lacks imaxdiv_t/imaxabs/imaxdiv
 * and the wide-char variants wcstoimax/wcstoumax.
 *
 * NOTE: Do NOT re-declare strtoimax/strtoumax — IRIX's inttypes.h declares
 * them without extern "C", so in C++ mode they have C++ linkage. Re-declaring
 * them in extern "C" causes "different language linkage" errors. */
#include <wchar.h>  /* for wchar_t */

typedef struct {
    intmax_t quot;
    intmax_t rem;
} imaxdiv_t;

#ifdef __cplusplus
extern "C" {
#endif

static inline intmax_t imaxabs(intmax_t j)
{
    return j < 0 ? -j : j;
}

static inline imaxdiv_t imaxdiv(intmax_t numer, intmax_t denom)
{
    imaxdiv_t result;
    result.quot = numer / denom;
    result.rem  = numer % denom;
    return result;
}

intmax_t  wcstoimax(const wchar_t *nptr, wchar_t **endptr, int base);
uintmax_t wcstoumax(const wchar_t *nptr, wchar_t **endptr, int base);

#ifdef __cplusplus
}
#endif

/* MAX-width format specifiers (intmax_t = long long on MIPS n32) */
#ifndef PRIdMAX
#define PRIdMAX "lld"
#endif

#ifndef PRIiMAX
#define PRIiMAX "lli"
#endif

#ifndef PRIuMAX
#define PRIuMAX "llu"
#endif

#ifndef PRIxMAX
#define PRIxMAX "llx"
#endif

#ifndef PRIXMAX
#define PRIXMAX "llX"
#endif

#ifndef PRIoMAX
#define PRIoMAX "llo"
#endif

/* PTR-width format specifiers (intptr_t = int on MIPS n32, 32-bit pointers) */
#ifndef PRIdPTR
#define PRIdPTR "d"
#endif

#ifndef PRIiPTR
#define PRIiPTR "i"
#endif

#ifndef PRIuPTR
#define PRIuPTR "u"
#endif

#ifndef PRIxPTR
#define PRIxPTR "x"
#endif

#ifndef PRIXPTR
#define PRIXPTR "X"
#endif

/* SCN (scanf) format specifiers */
#ifndef SCNd64
#define SCNd64 "lld"
#endif

#ifndef SCNu64
#define SCNu64 "llu"
#endif

#ifndef SCNx64
#define SCNx64 "llx"
#endif

#ifndef SCNdMAX
#define SCNdMAX "lld"
#endif

#ifndef SCNuMAX
#define SCNuMAX "llu"
#endif

#ifndef SCNxMAX
#define SCNxMAX "llx"
#endif

#endif /* _MOGRIX_COMPAT_INTTYPES_H */
