/* sys/procfs.h - wrapper for IRIX procfs.h
 *
 * IRIX's procfs.h needs gregset_t/fpregset_t from sys/ucontext.h.
 * Include sys/signal.h first (which pulls in ucontext.h) so the
 * types are properly defined before procfs.h uses them.
 *
 * Previous approach of pre-defining __fpregset caused redefinition
 * errors when procfs.h transitively included ucontext.h again.
 */
#ifndef _CLANG_COMPAT_SYS_PROCFS_H
#define _CLANG_COMPAT_SYS_PROCFS_H

#include <sys/types.h>
/* Pull in gregset_t/fpregset_t via the standard include chain */
#include <sys/signal.h>

/* Now include the real IRIX procfs.h */
#include_next <sys/procfs.h>

#endif /* _CLANG_COMPAT_SYS_PROCFS_H */
