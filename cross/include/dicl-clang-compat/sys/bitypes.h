/*
 * DICL clang compat sys/bitypes.h
 * IRIX headers reference this but it doesn't exist in the sysroot.
 * The types it would provide (u_int8_t, u_int16_t, etc.) are already
 * available from <sys/types.h> on IRIX.
 */
#ifndef _DICL_SYS_BITYPES_H
#define _DICL_SYS_BITYPES_H

#include_next <sys/types.h>

#endif /* _DICL_SYS_BITYPES_H */
