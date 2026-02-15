/*
 * forkpty() implementation for IRIX using _getpty().
 *
 * tmux detects IRIX as "unknown" platform for forkpty, so this file
 * is installed as compat/forkpty-unknown.c.
 *
 * CRITICAL: After setsid(), must re-open slave WITHOUT O_NOCTTY so it
 * becomes the controlling terminal. On SVR4/IRIX, only the first
 * terminal open after setsid() (without O_NOCTTY) sets the controlling
 * terminal. Without this, the spawned shell has no controlling
 * terminal -> no job control -> exits immediately.
 */

#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "compat.h"

void fatal(const char *, ...);

pid_t
forkpty(int *master, char *name, struct termios *tio, struct winsize *ws)
{
	int slave = -1;
	char *sname;
	char slavename[TTY_NAME_MAX];
	pid_t pid;

	sname = _getpty(master, O_RDWR | O_NOCTTY, 0620, 0);
	if (sname == NULL)
		return (-1);

	strlcpy(slavename, sname, sizeof(slavename));
	if (name != NULL)
		strlcpy(name, slavename, TTY_NAME_MAX);

	if ((slave = open(slavename, O_RDWR | O_NOCTTY)) == -1)
		goto out;

	switch (pid = fork()) {
	case -1:
		goto out;
	case 0:
		close(*master);
		close(slave);
		setsid();
		/* Re-open slave WITHOUT O_NOCTTY -- first terminal open after
		 * setsid() becomes the controlling terminal on SVR4/IRIX. */
		slave = open(slavename, O_RDWR);
		if (slave == -1)
			_exit(1);
		if (tio != NULL && tcsetattr(slave, TCSAFLUSH, tio) == -1)
			fatal("tcsetattr failed");
		if (ws != NULL && ioctl(slave, TIOCSWINSZ, ws) == -1)
			fatal("ioctl failed");
		dup2(slave, 0);
		dup2(slave, 1);
		dup2(slave, 2);
		if (slave > 2)
			close(slave);
		return (0);
	}

	close(slave);
	return (pid);
out:
	if (*master != -1)
		close(*master);
	if (slave != -1)
		close(slave);
	return (-1);
}
