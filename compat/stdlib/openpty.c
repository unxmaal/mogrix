/*
 * openpty() for IRIX
 *
 * IRIX uses _getpty() instead of the BSD/glibc openpty().
 * _getpty(int *fildes, int oflag, mode_t mode, int nofork)
 * returns the slave device name and sets *fildes to the master fd.
 */

#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <sys/ioctl.h>

int openpty(int *amaster, int *aslave, char *name,
            struct termios *termp, struct winsize *winp)
{
    char *slave_name;

    slave_name = _getpty(amaster, O_RDWR | O_NOCTTY, 0600, 0);
    if (slave_name == NULL)
        return -1;

    *aslave = open(slave_name, O_RDWR | O_NOCTTY);
    if (*aslave < 0) {
        close(*amaster);
        return -1;
    }

    if (name)
        strcpy(name, slave_name);
    if (termp)
        tcsetattr(*aslave, TCSAFLUSH, termp);
    if (winp)
        ioctl(*aslave, TIOCSWINSZ, winp);

    return 0;
}
