/* sys/select.h - wrapper for IRIX sys/select.h
 *
 * IRIX's sys/select.h only defines fd_set and FD_* macros but
 * does not declare select(). The declaration is in <unistd.h>
 * and <sys/time.h> instead. This causes problems with gnulib
 * which expects sys/select.h to declare select().
 *
 * Include the real header, then declare select() if not already
 * declared (the macro guard prevents double-declaration when
 * gnulib's generated sys/select.h is also involved).
 */
#ifndef _CLANG_COMPAT_SYS_SELECT_H
#define _CLANG_COMPAT_SYS_SELECT_H

#include_next <sys/select.h>
#include <sys/time.h>

#ifndef _DICL_SELECT_DECLARED
#define _DICL_SELECT_DECLARED
#ifdef __cplusplus
extern "C" {
#endif

extern int select(int, fd_set *, fd_set *, fd_set *, struct timeval *);

#ifdef __cplusplus
}
#endif
#endif /* _DICL_SELECT_DECLARED */

#endif /* _CLANG_COMPAT_SYS_SELECT_H */
