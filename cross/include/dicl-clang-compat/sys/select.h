/* sys/select.h - wrapper for IRIX sys/select.h
 *
 * IRIX's sys/select.h only defines fd_set and FD_* macros but
 * does not declare select(). The declaration is in <unistd.h>
 * and <sys/time.h> instead. This causes problems with gnulib
 * which expects sys/select.h to declare select().
 *
 * When _XOPEN_SOURCE is set, IRIX sys/time.h defines a static
 * select() wrapper around __xpg4_select(). We must NOT also declare
 * extern select() in that case — the conflicting linkage causes
 * duplicate symbols at link time.
 *
 * Strategy: Only provide the extern declaration if sys/time.h hasn't
 * been included yet. Code that includes sys/time.h gets the static
 * wrapper directly.
 */
#ifndef _CLANG_COMPAT_SYS_SELECT_H
#define _CLANG_COMPAT_SYS_SELECT_H

#include_next <sys/select.h>

/* Only declare select() if sys/time.h hasn't been included.
 * sys/time.h provides its own select() (static wrapper or extern)
 * depending on _XOPEN_SOURCE. Adding our extern on top of that
 * static definition causes duplicate symbols. */
#ifndef _SYS_TIME_H
#ifndef _DICL_SELECT_DECLARED
#define _DICL_SELECT_DECLARED
struct timeval;
#ifdef __cplusplus
extern "C" {
#endif

extern int select(int, fd_set *, fd_set *, fd_set *, struct timeval *);

#ifdef __cplusplus
}
#endif
#endif /* _DICL_SELECT_DECLARED */
#endif /* _SYS_TIME_H */

#endif /* _CLANG_COMPAT_SYS_SELECT_H */
