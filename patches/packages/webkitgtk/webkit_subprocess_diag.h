/*
 * webkit_subprocess_diag.h - Diagnostic logging for WebKit subprocess init
 *
 * TEMPORARY: Remove after identifying WebProcess/NetworkProcess death cause.
 *
 * Writes stage markers to $MOGRIX_CRASH_DIR/mogrix_diag_<pid>.log
 * Uses raw open/write/close per call — no buffering, survives _exit().
 *
 * Usage: MOGRIX_DIAG("stage_name")
 * Each call opens, appends, syncs, closes. Safe even if process dies
 * immediately after.
 */
#ifndef WEBKIT_SUBPROCESS_DIAG_H
#define WEBKIT_SUBPROCESS_DIAG_H

#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

static inline void _mogrix_diag(const char *stage) {
    const char *dir = getenv("MOGRIX_CRASH_DIR");
    if (!dir || !dir[0]) dir = "/tmp";

    /* Build path: <dir>/mogrix_diag_<pid>.log */
    char path[512];
    char pidbuf[12];
    int pidlen = 0;
    int pval = getpid();
    int i = 0;
    const char *p;

    if (pval <= 0) { pidbuf[pidlen++] = '0'; pval = 0; }
    else {
        while (pval > 0) { pidbuf[pidlen++] = '0' + (pval % 10); pval /= 10; }
        /* Reverse */
        for (int j = 0; j < pidlen / 2; j++) {
            char t = pidbuf[j];
            pidbuf[j] = pidbuf[pidlen - 1 - j];
            pidbuf[pidlen - 1 - j] = t;
        }
    }

    for (p = dir; *p && i < 490; ) path[i++] = *p++;
    if (i > 0 && path[i-1] != '/') path[i++] = '/';
    for (p = "mogrix_diag_"; *p; ) path[i++] = *p++;
    for (int j = 0; j < pidlen; j++) path[i++] = pidbuf[j];
    for (p = ".log"; *p; ) path[i++] = *p++;
    path[i] = '\0';

    int fd = open(path, O_WRONLY | O_CREAT | O_APPEND, 0666);
    if (fd >= 0) {
        write(fd, "[DIAG] ", 7);
        write(fd, stage, strlen(stage));
        write(fd, "\n", 1);
        fsync(fd);
        close(fd);
    }
    /* Also stderr (may be captured by parent) */
    write(STDERR_FILENO, "[DIAG] ", 7);
    write(STDERR_FILENO, stage, strlen(stage));
    write(STDERR_FILENO, "\n", 1);
}

/* Log with an integer value */
static inline void _mogrix_diag_int(const char *stage, int val) {
    char buf[256];
    int i = 0;
    const char *p;
    char numbuf[12];
    int numlen = 0;
    unsigned int uval;

    for (p = stage; *p && i < 200; ) buf[i++] = *p++;
    buf[i++] = '=';

    if (val < 0) { buf[i++] = '-'; uval = (unsigned int)(-val); }
    else uval = (unsigned int)val;
    if (uval == 0) { numbuf[numlen++] = '0'; }
    else { while (uval > 0) { numbuf[numlen++] = '0' + (uval % 10); uval /= 10; } }
    for (int j = numlen - 1; j >= 0; j--) buf[i++] = numbuf[j];
    buf[i] = '\0';

    _mogrix_diag(buf);
}

/* Log with a string value */
static inline void _mogrix_diag_str(const char *stage, const char *val) {
    char buf[512];
    int i = 0;
    const char *p;

    for (p = stage; *p && i < 200; ) buf[i++] = *p++;
    buf[i++] = '=';
    if (val) {
        for (p = val; *p && i < 500; ) buf[i++] = *p++;
    } else {
        for (p = "(null)"; *p; ) buf[i++] = *p++;
    }
    buf[i] = '\0';

    _mogrix_diag(buf);
}

#define MOGRIX_DIAG(msg) _mogrix_diag(msg)
#define MOGRIX_DIAG_INT(msg, val) _mogrix_diag_int(msg, val)
#define MOGRIX_DIAG_STR(msg, val) _mogrix_diag_str(msg, val)

#endif /* WEBKIT_SUBPROCESS_DIAG_H */
