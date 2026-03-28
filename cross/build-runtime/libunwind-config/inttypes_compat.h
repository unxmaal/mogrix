/* IRIX inttypes.h lacks C99 format macros for pointer-sized types.
   N32 ABI: pointers are 32-bit, ptrdiff_t is int. */
#ifndef MOGRIX_INTTYPES_COMPAT_H
#define MOGRIX_INTTYPES_COMPAT_H

#ifndef PRIdPTR
#define PRIdPTR "d"
#endif
#ifndef PRIxPTR
#define PRIxPTR "x"
#endif
#ifndef PRIuPTR
#define PRIuPTR "u"
#endif

#endif
