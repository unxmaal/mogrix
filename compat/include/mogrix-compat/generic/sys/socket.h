/*
 * mogrix-compat/generic/sys/socket.h
 *
 * Wrapper that includes the real sys/socket.h and defines
 * Linux socket constants missing on IRIX.
 *
 * MSG_NOSIGNAL: Prevents SIGPIPE on send()/sendmsg(). IRIX doesn't have it.
 *   Define as 0 — callers should also SIG_IGN SIGPIPE for safety.
 *   Discovered via WebKit (session 115), but general: Qt5, GnuTLS, libsoup,
 *   and any modern networking code may use it.
 *
 * SOCK_CLOEXEC / SOCK_NONBLOCK: Linux 2.6.27+ socket creation flags.
 *   Define as 0 — callers get a working socket but must manually set
 *   FD_CLOEXEC via fcntl() and O_NONBLOCK via fcntl() if needed.
 *   Qt5 qnet_unix_p.h uses both.
 */

#ifndef _MOGRIX_COMPAT_SYS_SOCKET_H
#define _MOGRIX_COMPAT_SYS_SOCKET_H

#include_next <sys/socket.h>

/* MSG_NOSIGNAL: suppress SIGPIPE on send/sendmsg.
 * Safe to define as 0 — the flag simply isn't passed, which means
 * SIGPIPE can fire. Callers should also signal(SIGPIPE, SIG_IGN). */
#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

/* SOCK_CLOEXEC: atomically set FD_CLOEXEC on new socket.
 * Defining as 0 means the flag is a no-op — socket() still works,
 * but won't auto-set close-on-exec. Apps must use fcntl(fd, F_SETFD, FD_CLOEXEC). */
#ifndef SOCK_CLOEXEC
#define SOCK_CLOEXEC 0
#endif

/* SOCK_NONBLOCK: atomically set O_NONBLOCK on new socket.
 * Defining as 0 means the flag is a no-op — socket is blocking by default.
 * Apps must use fcntl(fd, F_SETFL, O_NONBLOCK) if needed. */
#ifndef SOCK_NONBLOCK
#define SOCK_NONBLOCK 0
#endif

#endif /* _MOGRIX_COMPAT_SYS_SOCKET_H */
