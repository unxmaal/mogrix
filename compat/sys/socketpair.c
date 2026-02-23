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
 * IMPORTANT: Both sockets are explicitly bind()ed to temp paths.
 * Without this, getsockname() on the unnamed connecting socket returns
 * addrlen=0 on IRIX. GLib's g_socket_new_from_fd() then tries SO_DOMAIN
 * (which IRIX lacks) and fails, returning NULL. This broke WebKit IPC:
 * subprocesses got NULL GSockets, couldn't set up event monitors, and died.
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

/* Build a unique socket path into buf: /tmp/.msp_<pid>_<cnt><suffix>
 * Returns length written (excluding NUL). */
static int
sp_build_path(char *buf, int pid, int cnt, char suffix)
{
    int i = 0;
    memcpy(buf, "/tmp/.msp_", 10);
    i = 10;
    /* pid digits */
    {
        char tmp[12];
        int n = 0, v = pid;
        if (v < 0) v = -v;
        do { tmp[n++] = '0' + (v % 10); v /= 10; } while (v > 0);
        while (n > 0) buf[i++] = tmp[--n];
    }
    buf[i++] = '_';
    /* counter digits */
    {
        char tmp[12];
        int n = 0, v = cnt;
        if (v < 0) v = -v;
        do { tmp[n++] = '0' + (v % 10); v /= 10; } while (v > 0);
        while (n > 0) buf[i++] = tmp[--n];
    }
    if (suffix) buf[i++] = suffix;
    buf[i] = '\0';
    return i;
}

int
socketpair(int domain, int type, int protocol, int sv[2])
{
    struct sockaddr_un addr, addr_conn;
    int listener, conn, acc;
    int save_errno;
    int real_type;
    int pid, cnt;

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

    pid = (int)getpid();
    cnt = sp_counter++;

    /* Listener address: /tmp/.msp_<pid>_<cnt>s */
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    sp_build_path(addr.sun_path, pid, cnt, 's');

    unlink(addr.sun_path);

    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        goto fail_listener;

    if (listen(listener, 1) < 0)
        goto fail_bound;

    conn = socket(AF_UNIX, real_type, 0);
    if (conn < 0)
        goto fail_bound;

    /* Bind the connecting socket to its own name so that getsockname()
     * returns a valid AF_UNIX address on IRIX. Without this, the unnamed
     * socket causes GLib's g_socket_new_from_fd() to fail (no SO_DOMAIN
     * on IRIX to detect the address family of unnamed sockets). */
    memset(&addr_conn, 0, sizeof(addr_conn));
    addr_conn.sun_family = AF_UNIX;
    sp_build_path(addr_conn.sun_path, pid, cnt, 'c');

    unlink(addr_conn.sun_path);

    if (bind(conn, (struct sockaddr *)&addr_conn, sizeof(addr_conn)) < 0)
        goto fail_conn;

    if (connect(conn, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        goto fail_conn;

    acc = accept(listener, (struct sockaddr *)0, (socklen_t *)0);
    if (acc < 0)
        goto fail_conn;

    close(listener);
    unlink(addr.sun_path);
    unlink(addr_conn.sun_path);

    sv[0] = conn;
    sv[1] = acc;
    return 0;

fail_conn:
    save_errno = errno;
    close(conn);
    unlink(addr_conn.sun_path);
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
