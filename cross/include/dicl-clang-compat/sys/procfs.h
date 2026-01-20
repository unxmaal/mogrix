/* sys/procfs.h - wrapper for IRIX procfs.h
 *
 * IRIX's procfs.h uses gregset_t which is defined in sys/ucontext.h
 * but the complex standards.h macros using defined() inside macro
 * expansions have undefined behavior in clang.
 *
 * We directly provide the needed types before including procfs.h.
 */
#ifndef _CLANG_COMPAT_SYS_PROCFS_H
#define _CLANG_COMPAT_SYS_PROCFS_H

#include <sys/types.h>

/* Types needed by procfs.h that come from ucontext.h */
#ifndef _GREGSET_T_DEFINED
#define _GREGSET_T_DEFINED
typedef __uint64_t greg_t;
typedef greg_t gregset_t[37];
#endif

#ifndef _FPREGSET_T_DEFINED
#define _FPREGSET_T_DEFINED
typedef struct __fpregset {
    union {
        double __fp_dregs[32];
        struct {
            unsigned int __fp_regs[32];
            unsigned int __fp_csr;
        } __fp_fregs;
    } __fp_r;
} fpregset_t;
#endif

/* Now include the real IRIX procfs.h */
#include_next <sys/procfs.h>

#endif /* _CLANG_COMPAT_SYS_PROCFS_H */
