/*
 * DICL clang compat string.h
 * Wrapper for IRIX string.h
 *
 * Note: strerror_r is NOT declared here. IRIX doesn't have it,
 * and unconditionally declaring it conflicts with gnulib's own
 * strerror_r replacement module. Packages needing strerror_r
 * should inject the compat function via inject_compat_functions
 * and let gnulib handle the declaration.
 */
#ifndef _DICL_STRING_H
#define _DICL_STRING_H

/* Include IRIX string.h first */
#include_next <string.h>

#endif /* _DICL_STRING_H */
