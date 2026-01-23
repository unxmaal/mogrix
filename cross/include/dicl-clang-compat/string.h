/*
 * DICL clang compat string.h
 * Add GNU/POSIX functions not available in IRIX string.h
 */
#ifndef _DICL_STRING_H
#define _DICL_STRING_H

/* Include IRIX string.h first */
#include_next <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* GNU-style strerror_r (provided by compat library) */
#ifndef _DICL_STRERROR_R
#define _DICL_STRERROR_R
extern char *strerror_r(int errnum, char *buf, size_t buflen);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _DICL_STRING_H */
