/*
 * DICL clang compat endian.h
 * Provide Linux-style endian macros for IRIX
 * IRIX doesn't have <endian.h>; byte order comes from MIPSEB/MIPSEL macros
 */
#ifndef _DICL_ENDIAN_H
#define _DICL_ENDIAN_H

#define __LITTLE_ENDIAN 1234
#define __BIG_ENDIAN    4321
#define __PDP_ENDIAN    3412

#ifdef __MIPSEB__
#define __BYTE_ORDER __BIG_ENDIAN
#else
#define __BYTE_ORDER __LITTLE_ENDIAN
#endif

/* BSD-style aliases */
#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN __LITTLE_ENDIAN
#endif
#ifndef BIG_ENDIAN
#define BIG_ENDIAN __BIG_ENDIAN
#endif
#ifndef BYTE_ORDER
#define BYTE_ORDER __BYTE_ORDER
#endif

#endif /* _DICL_ENDIAN_H */
