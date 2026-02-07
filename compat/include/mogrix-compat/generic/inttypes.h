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

#endif /* _MOGRIX_COMPAT_INTTYPES_H */
