/*
 * DICL clang compat endian.h
 * Provide Linux-style endian macros for IRIX
 * IRIX doesn't have <endian.h>; byte order comes from MIPSEB/MIPSEL macros
 */
#ifndef _DICL_ENDIAN_H
#define _DICL_ENDIAN_H

#include <stdint.h>

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
#ifndef PDP_ENDIAN
#define PDP_ENDIAN __PDP_ENDIAN
#endif
#ifndef BYTE_ORDER
#define BYTE_ORDER __BYTE_ORDER
#endif

/* Byte-swap helpers */
#ifndef __bswap16
#define __bswap16(x) ((uint16_t)((((x) >> 8) & 0xff) | (((x) & 0xff) << 8)))
#endif
#ifndef __bswap32
#define __bswap32(x) ((uint32_t)((((x) >> 24) & 0xff) | (((x) >> 8) & 0xff00) | \
                      (((x) & 0xff00) << 8) | (((x) & 0xff) << 24)))
#endif

/* Big-endian host-to-network and back (no-op on MIPS big-endian) */
#if __BYTE_ORDER == __BIG_ENDIAN
#define htobe16(x) (x)
#define htobe32(x) (x)
#define htobe64(x) (x)
#define be16toh(x) (x)
#define be32toh(x) (x)
#define be64toh(x) (x)
#define htole16(x) __bswap16(x)
#define htole32(x) __bswap32(x)
#define le16toh(x) __bswap16(x)
#define le32toh(x) __bswap32(x)
#else
#define htobe16(x) __bswap16(x)
#define htobe32(x) __bswap32(x)
#define be16toh(x) __bswap16(x)
#define be32toh(x) __bswap32(x)
#define htole16(x) (x)
#define htole32(x) (x)
#define le16toh(x) (x)
#define le32toh(x) (x)
#endif

#endif /* _DICL_ENDIAN_H */
