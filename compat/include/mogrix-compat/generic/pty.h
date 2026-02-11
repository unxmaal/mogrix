/*
 * pty.h — openpty() declaration for IRIX
 *
 * IRIX has no <pty.h>. This header provides the openpty() prototype
 * for packages that expect it (st, screen, etc.).
 */
#ifndef MOGRIX_COMPAT_PTY_H
#define MOGRIX_COMPAT_PTY_H

#include <termios.h>

struct winsize;

int openpty(int *amaster, int *aslave, char *name,
            struct termios *termp, struct winsize *winp);

#endif /* MOGRIX_COMPAT_PTY_H */
