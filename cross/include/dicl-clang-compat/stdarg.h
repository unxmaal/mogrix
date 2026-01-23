/*
 * DICL clang compat stdarg.h
 * Provide clang-compatible va_list and va_* macros
 */
#ifndef _DICL_STDARG_H
#define _DICL_STDARG_H

/* Prevent IRIX stdarg.h from being included */
#define _STDARG_H
#define __STDARG_H

/* Use clang's builtin va_list type for compatibility with clang builtins */
#ifndef _VA_LIST_
#define _VA_LIST_
typedef __builtin_va_list va_list;
#endif

/* clang builtins */
#define va_start(ap, param) __builtin_va_start(ap, param)
#define va_end(ap)          __builtin_va_end(ap)
#define va_arg(ap, type)    __builtin_va_arg(ap, type)
#define va_copy(dest, src)  __builtin_va_copy(dest, src)

#endif /* _DICL_STDARG_H */
