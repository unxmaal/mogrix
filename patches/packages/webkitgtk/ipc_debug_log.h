/*
 * IPC debug logging for IRIX - unbuffered write() to per-process log files.
 * Each process writes to /usr/people/edodd/ipc_<pid>.log.
 * Temporary: remove once IPC connection drop root cause is found.
 */
#ifndef IPC_DEBUG_LOG_H
#define IPC_DEBUG_LOG_H

#ifdef __sgi
#include <stdarg.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>

static void _ipc_log(const char *fmt, ...) __attribute__((format(printf,1,2)));
static void _ipc_log(const char *fmt, ...) {
    char path[128];
    snprintf(path, sizeof(path), "/usr/people/edodd/ipc_%d.log", (int)getpid());
    int logfd = open(path, O_WRONLY|O_CREAT|O_APPEND, 0666);
    if (logfd >= 0) {
        char buf[1024];
        va_list ap;
        va_start(ap, fmt);
        int n = vsnprintf(buf, sizeof(buf), fmt, ap);
        va_end(ap);
        if (n > 0) { int wr = write(logfd, buf, n); (void)wr; }
        close(logfd);
    }
}

/* Macros for each log site - format strings here, not in YAML */
#define IPC_LOG_OPEN(sock, pid) \
    _ipc_log("OPEN sock=%d pid=%d\n", (int)(sock), (int)(pid))
#define IPC_LOG_INVALIDATE(sock, pid) \
    _ipc_log("INVALIDATE sock=%d pid=%d\n", (int)(sock), (int)(pid))
#define IPC_LOG_MSG(count, pid) \
    _ipc_log("MSG count=%d pid=%d\n", (int)(count), (int)(pid))
#define IPC_LOG_RECV_ERR(err, sock) \
    _ipc_log("RECV_ERR errno=%d sock=%d\n", (int)(err), (int)(sock))
#define IPC_LOG_CTRUNC(sock) \
    _ipc_log("MSG_CTRUNC sock=%d\n", (int)(sock))
#define IPC_LOG_CLOSE_CONNRESET(sock, pid) \
    _ipc_log("CLOSE:ECONNRESET sock=%d pid=%d\n", (int)(sock), (int)(pid))
#define IPC_LOG_CLOSE_RECV_ERR(err, sock, pid) \
    _ipc_log("CLOSE:RECV_ERR errno=%d sock=%d pid=%d\n", (int)(err), (int)(sock), (int)(pid))
#define IPC_LOG_CLOSE_EOF(sock, pid) \
    _ipc_log("CLOSE:EOF sock=%d pid=%d\n", (int)(sock), (int)(pid))
#define IPC_LOG_CLOSE_GIO(cond, pid) \
    _ipc_log("CLOSE:GIO_HUP cond=0x%x pid=%d\n", (unsigned)(cond), (int)(pid))
#define IPC_LOG_CLOSE_SEND(err, sock, pid) \
    _ipc_log("CLOSE:SEND_CONNRESET errno=%d sock=%d pid=%d\n", (int)(err), (int)(sock), (int)(pid))
#define IPC_LOG_SEND_ERR(err, sock, pid) \
    _ipc_log("SEND_ERR errno=%d sock=%d pid=%d\n", (int)(err), (int)(sock), (int)(pid))

#else
/* No-op on non-IRIX */
#define IPC_LOG_OPEN(sock, pid) ((void)0)
#define IPC_LOG_INVALIDATE(sock, pid) ((void)0)
#define IPC_LOG_MSG(count, pid) ((void)0)
#define IPC_LOG_RECV_ERR(err, sock) ((void)0)
#define IPC_LOG_CTRUNC(sock) ((void)0)
#define IPC_LOG_CLOSE_CONNRESET(sock, pid) ((void)0)
#define IPC_LOG_CLOSE_RECV_ERR(err, sock, pid) ((void)0)
#define IPC_LOG_CLOSE_EOF(sock, pid) ((void)0)
#define IPC_LOG_CLOSE_GIO(cond, pid) ((void)0)
#define IPC_LOG_CLOSE_SEND(err, sock, pid) ((void)0)
#define IPC_LOG_SEND_ERR(err, sock, pid) ((void)0)
#endif

#endif /* IPC_DEBUG_LOG_H */
