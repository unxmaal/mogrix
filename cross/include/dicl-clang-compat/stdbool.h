/*
 * DICL clang compat stdbool.h
 * Override IRIX's stdbool.h which requires MIPSpro's __c99 mode
 */
#ifndef _DICL_STDBOOL_H
#define _DICL_STDBOOL_H

/* Standard C99 stdbool.h */
#ifndef __cplusplus
#define bool _Bool
#define true 1
#define false 0
#endif

#define __bool_true_false_are_defined 1

#endif /* _DICL_STDBOOL_H */
