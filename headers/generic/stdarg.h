/* stdarg.h - replacement for IRIX stdarg.h when using clang
 *
 * IRIX's native stdarg.h uses MIPSpro-specific builtins that clang
 * doesn't support. This replacement uses clang's builtin varargs.
 */

#ifndef _STDARG_H
#define _STDARG_H

#ifndef _VA_LIST_
#define _VA_LIST_
typedef __builtin_va_list va_list;
#endif

#define va_start(ap, param) __builtin_va_start(ap, param)
#define va_end(ap)          __builtin_va_end(ap)
#define va_arg(ap, type)    __builtin_va_arg(ap, type)
#define va_copy(dest, src)  __builtin_va_copy(dest, src)

/* For compatibility with code that uses __va_list */
#ifndef __va_list
#define __va_list va_list
#endif

#endif /* _STDARG_H */
