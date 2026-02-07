/*
 * dicl-clang-compat/sgidefs.h
 *
 * Overlay for IRIX sgidefs.h that adds __ASSEMBLER__ guards.
 * IRIX's sgidefs.h uses _LANGUAGE_C to guard typedefs, but clang
 * defines _LANGUAGE_C even in assembler-with-cpp mode.
 * This overlay undefines _LANGUAGE_C when __ASSEMBLER__ is set,
 * then includes the real header, restoring the define afterward.
 */
#ifndef _DICL_SGIDEFS_H
#define _DICL_SGIDEFS_H

#if defined(__ASSEMBLER__) && defined(_LANGUAGE_C)
# undef _LANGUAGE_C
# define _DICL_RESTORE_LANGUAGE_C 1
#endif

#include_next <sgidefs.h>

#ifdef _DICL_RESTORE_LANGUAGE_C
# define _LANGUAGE_C
# undef _DICL_RESTORE_LANGUAGE_C
#endif

#endif /* _DICL_SGIDEFS_H */
