/* Shim: define va_list as char* to match IRIX stdio_core.h expectations.
   GCC unwind code doesn't use variadic functions, so this is safe. */
#ifndef _STDARG_H
#define _STDARG_H
#define __STDARG_H
typedef char *va_list;
typedef char *__gnuc_va_list;
#define va_start(ap, param) __builtin_va_start(ap, param)
#define va_end(ap)          __builtin_va_end(ap)
#define va_arg(ap, type)    __builtin_va_arg(ap, type)
#define va_copy(dest, src)  __builtin_va_copy(dest, src)
#ifndef _VA_LIST_
#define _VA_LIST_
#endif
#endif
