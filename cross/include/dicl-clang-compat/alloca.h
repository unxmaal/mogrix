/* alloca.h - Compatibility header for IRIX cross-compilation with clang
 *
 * Clang supports __builtin_alloca, but IRIX headers may not include alloca.h
 * when __GNUC__ is defined (since gcc has a different builtin mechanism).
 * This header ensures alloca() is always declared for clang.
 */
#ifndef _MOGRIX_ALLOCA_H
#define _MOGRIX_ALLOCA_H

/* Include the system alloca.h first if available */
#include_next <alloca.h>

/* If alloca wasn't defined as a builtin, define it now */
#ifndef alloca
#define alloca __builtin_alloca
#endif

#endif /* _MOGRIX_ALLOCA_H */
