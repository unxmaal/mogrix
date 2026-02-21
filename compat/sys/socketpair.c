/*
 * socketpair() compat wrapper for IRIX.
 *
 * IRIX AF_UNIX does not support SOCK_SEQPACKET (errno 120).
 * WebKit uses socketpair(AF_UNIX, SOCK_SEQPACKET, 0, fds) for IPC.
 * When it fails, WebKit calls abort().
 *
 * This provides a complete AF_UNIX socketpair implementation using
 * socket()/bind()/listen()/connect()/accept(). For SOCK_SEQPACKET
 * requests, it transparently uses SOCK_STREAM instead.
 *
 * Must be in libmogrix_compat.so, preloaded via _RLDN32_LIST.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

/* IRIX: SOCK_SEQPACKET = 6, SOCK_STREAM = 2 */

/* Atomic counter for unique socket paths */
static int sp_counter = 0;

int
socketpair(int domain, int type, int protocol, int sv[2])
{
    struct sockaddr_un addr;
    int listener, conn, acc;
    int save_errno;
    int real_type;

    /* Only handle AF_UNIX — socketpair is almost exclusively AF_UNIX */
    if (domain != AF_UNIX) {
        errno = EAFNOSUPPORT;
        return -1;
    }

    /* Map SOCK_SEQPACKET to SOCK_STREAM — WebKit uses length-prefixed
     * framing so stream semantics work identically. */
    real_type = (type == SOCK_SEQPACKET) ? SOCK_STREAM : type;

    listener = socket(AF_UNIX, real_type, 0);
    if (listener < 0)
        return -1;

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    /* Use pid + counter for unique path */
    {
        int pid = (int)getpid();
        int cnt = sp_counter++;
        int i = 0;
        char *p = addr.sun_path;
        /* Build "/tmp/.msp_<pid>_<cnt>" by hand (avoid sprintf deps) */
        memcpy(p, "/tmp/.msp_", 10);
        i = 10;
        /* pid digits */
        {
            char buf[12];
            int n = 0, v = pid;
            if (v < 0) v = -v;
            do { buf[n++] = '0' + (v % 10); v /= 10; } while (v > 0);
            while (n > 0) p[i++] = buf[--n];
        }
        p[i++] = '_';
        /* counter digits */
        {
            char buf[12];
            int n = 0, v = cnt;
            if (v < 0) v = -v;
            do { buf[n++] = '0' + (v % 10); v /= 10; } while (v > 0);
            while (n > 0) p[i++] = buf[--n];
        }
        p[i] = '\0';
    }

    unlink(addr.sun_path);

    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        goto fail_listener;

    if (listen(listener, 1) < 0)
        goto fail_bound;

    conn = socket(AF_UNIX, real_type, 0);
    if (conn < 0)
        goto fail_bound;

    if (connect(conn, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        goto fail_conn;

    acc = accept(listener, (struct sockaddr *)0, (socklen_t *)0);
    if (acc < 0)
        goto fail_conn;

    close(listener);
    unlink(addr.sun_path);

    sv[0] = conn;
    sv[1] = acc;
    return 0;

fail_conn:
    save_errno = errno;
    close(conn);
    errno = save_errno;
fail_bound:
    save_errno = errno;
    unlink(addr.sun_path);
    errno = save_errno;
fail_listener:
    save_errno = errno;
    close(listener);
    errno = save_errno;
    return -1;
}
