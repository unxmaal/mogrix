/*
 * DICL clang compat stdarg.h
 * Provide clang-compatible va_list and va_* macros
 *
 * In C mode: va_list = __builtin_va_list (void* on MIPS n32)
 * In C++ mode: va_list = char* to match IRIX header declarations.
 *   The va_* macros use type-punning since both are 4-byte pointers
 *   with identical representation on MIPS n32.
 */
#ifndef _DICL_STDARG_H
#define _DICL_STDARG_H

/* Assembly files may include headers that transitively pull in stdarg.h.
 * The assembler can't parse C typedefs, so guard the C-only parts. */
#ifndef __ASSEMBLER__

/* Prevent IRIX stdarg.h from being included */
#define _STDARG_H
#define __STDARG_H

/* GCC compat: __gnuc_va_list is used by libstdc++ headers */
typedef __builtin_va_list __gnuc_va_list;

#ifndef _VA_LIST_
#define _VA_LIST_
#ifdef __cplusplus
/* C++ mode: IRIX headers declare variadic functions with char* parameters.
 * C++ does not allow implicit void* -> char* conversion, so va_list must
 * be char* to match. We use type-punning through the va_* macros since
 * char* and __builtin_va_list (void*) have identical representation on
 * MIPS n32 (both 4-byte pointers). */
typedef char *va_list;
#else
/* C mode: use clang's native type directly */
typedef __builtin_va_list va_list;
#endif
#endif

/* clang builtins */
#ifdef __cplusplus
/* Type-pun char* as __builtin_va_list for clang builtins */
#define va_start(ap, param) __builtin_va_start(*(__builtin_va_list *)&(ap), param)
#define va_end(ap)          __builtin_va_end(*(__builtin_va_list *)&(ap))
#define va_arg(ap, type)    __builtin_va_arg(*(__builtin_va_list *)&(ap), type)
#define va_copy(dest, src)  __builtin_va_copy(*(__builtin_va_list *)&(dest), *(__builtin_va_list *)&(src))
#else
#define va_start(ap, param) __builtin_va_start(ap, param)
#define va_end(ap)          __builtin_va_end(ap)
#define va_arg(ap, type)    __builtin_va_arg(ap, type)
#define va_copy(dest, src)  __builtin_va_copy(dest, src)
#endif

#endif /* !__ASSEMBLER__ */

#endif /* _DICL_STDARG_H */
