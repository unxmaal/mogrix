/*
 * DICL clang compat byteswap.h
 * Linux-specific header providing byte swap macros.
 * IRIX doesn't have this header — provide equivalent using builtins.
 */
#ifndef _DICL_BYTESWAP_H
#define _DICL_BYTESWAP_H

#define bswap_16(x) __builtin_bswap16(x)
#define bswap_32(x) __builtin_bswap32(x)
#define bswap_64(x) __builtin_bswap64(x)

#endif /* _DICL_BYTESWAP_H */
