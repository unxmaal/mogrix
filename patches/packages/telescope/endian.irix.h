/* IRIX endian.h - MIPS is always big-endian */
#ifndef _COMPAT_ENDIAN_H
#define _COMPAT_ENDIAN_H

#define __LITTLE_ENDIAN 1234
#define __BIG_ENDIAN    4321
#define __PDP_ENDIAN    3412
#define __BYTE_ORDER    __BIG_ENDIAN

#define LITTLE_ENDIAN __LITTLE_ENDIAN
#define BIG_ENDIAN    __BIG_ENDIAN
#define PDP_ENDIAN    __PDP_ENDIAN
#define BYTE_ORDER    __BYTE_ORDER

/* htobe/betoh macros - no-op on big-endian */
#define htobe16(x) (x)
#define htobe32(x) (x)
#define htobe64(x) (x)
#define be16toh(x) (x)
#define be32toh(x) (x)
#define be64toh(x) (x)

/* Little-endian conversions need byte swapping */
#define __bswap16(x) ((uint16_t)((((x) >> 8) & 0xff) | (((x) & 0xff) << 8)))
#define __bswap32(x) ((uint32_t)((((x) >> 24) & 0xff) | (((x) >> 8) & 0xff00) | (((x) & 0xff00) << 8) | (((x) & 0xff) << 24)))
#define htole16(x) __bswap16(x)
#define htole32(x) __bswap32(x)
#define le16toh(x) __bswap16(x)
#define le32toh(x) __bswap32(x)

#endif /* _COMPAT_ENDIAN_H */
