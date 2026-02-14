/* bits/wordsize.h — glibc multilib header, IRIX n32 compat.
 * IRIX doesn't have glibc's bits/ headers. For n32 ABI, word size is
 * always 32 (sizeof(long) == 4, sizeof(void*) == 4).
 * This unblocks packages whose generated headers use the glibc multilib
 * pattern (#include <bits/wordsize.h> / #if __WORDSIZE == 32).
 */
#ifndef _MOGRIX_BITS_WORDSIZE_H
#define _MOGRIX_BITS_WORDSIZE_H

#define __WORDSIZE 32

#endif /* _MOGRIX_BITS_WORDSIZE_H */
