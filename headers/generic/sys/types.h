/* sys/types.h wrapper - ensures sigset_t is always defined */

#ifndef _SYS_TYPES_WRAPPER_H
#define _SYS_TYPES_WRAPPER_H

/* Include the original sys/types.h */
#include_next <sys/types.h>

/* Define sigset_t if not already defined */
#ifndef _SIGSET_T
#define _SIGSET_T
typedef struct {
    __uint32_t __sigbits[4];
} sigset_t;
#endif

#endif /* _SYS_TYPES_WRAPPER_H */
