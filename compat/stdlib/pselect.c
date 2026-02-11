/*
 * pselect() for IRIX
 *
 * IRIX has select() but lacks pselect() (POSIX.1-2001).
 * This wraps select() with sigprocmask() to atomically set
 * a signal mask during the wait.
 *
 * Note: This is NOT fully atomic (race window between sigprocmask
 * and select), but it's sufficient for typical terminal emulator use.
 */

#include <sys/select.h>
#include <sys/time.h>
#include <signal.h>

int pselect(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
            const struct timespec *timeout, const sigset_t *sigmask)
{
    struct timeval tv, *tvp = NULL;
    sigset_t origmask;
    int ret;

    if (timeout) {
        tv.tv_sec = timeout->tv_sec;
        tv.tv_usec = timeout->tv_nsec / 1000;
        tvp = &tv;
    }

    if (sigmask)
        sigprocmask(SIG_SETMASK, sigmask, &origmask);

    ret = select(nfds, readfds, writefds, exceptfds, tvp);

    if (sigmask)
        sigprocmask(SIG_SETMASK, &origmask, NULL);

    return ret;
}
