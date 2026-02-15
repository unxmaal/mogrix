/*
 * mogrix-compat/generic/sys/select.h
 *
 * Wrapper that includes the real sys/select.h and adds pselect()
 * declaration for IRIX compatibility.
 *
 * IRIX has select() but lacks pselect() (POSIX.1-2001).
 * The implementation (compat/stdlib/pselect.c) wraps select()
 * with sigprocmask().
 */

#ifndef _MOGRIX_COMPAT_SYS_SELECT_H
#define _MOGRIX_COMPAT_SYS_SELECT_H

#include_next <sys/select.h>
#include <signal.h>
#include <sys/time.h>

#ifdef __cplusplus
extern "C" {
#endif

int pselect(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
            const struct timespec *timeout, const sigset_t *sigmask);

#ifdef __cplusplus
}
#endif

#endif /* _MOGRIX_COMPAT_SYS_SELECT_H */
