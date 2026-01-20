/* stdio.h - clang compatibility wrapper for IRIX stdio.h
 *
 * The IRIX stdio_core.h defines va_list as "char *" which conflicts with
 * clang's __builtin_va_list. We pre-define va_list here so the IRIX
 * header skips its definition.
 */

#ifndef _DICL_CLANG_COMPAT_STDIO_H
#define _DICL_CLANG_COMPAT_STDIO_H

/* Pre-define va_list to prevent IRIX from defining it as char* */
#ifndef _VA_LIST_
#define _VA_LIST_
typedef __builtin_va_list va_list;
#endif

/* Now include the real IRIX stdio.h */
#include_next <stdio.h>

#endif /* _DICL_CLANG_COMPAT_STDIO_H */
