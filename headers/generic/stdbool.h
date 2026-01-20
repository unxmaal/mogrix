/* stdbool.h - replacement for IRIX stdbool.h when using clang
 *
 * IRIX's native stdbool.h throws an error for non-C99 mode.
 * This replacement works with both C89 and C99.
 */

#ifndef _STDBOOL_H
#define _STDBOOL_H

#ifndef __cplusplus

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
/* C99 or later - use _Bool */
#define bool _Bool
#else
/* C89 - use int */
typedef int _Bool;
#define bool _Bool
#endif

#define true 1
#define false 0
#define __bool_true_false_are_defined 1

#endif /* __cplusplus */

#endif /* _STDBOOL_H */
