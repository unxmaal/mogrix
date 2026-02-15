/*
 * getentropy() implementation for IRIX.
 *
 * Reads from /dev/urandom. Replaces the Linux-specific
 * getentropy_linux.c in libretls.
 */

#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

int
getentropy(void *buf, size_t buflen)
{
	int fd, saved_errno;
	ssize_t n;
	char *p = buf;

	if (buflen > 256) {
		errno = EIO;
		return -1;
	}

	fd = open("/dev/urandom", O_RDONLY);
	if (fd == -1)
		return -1;

	while (buflen > 0) {
		n = read(fd, p, buflen);
		if (n < 0) {
			if (errno == EINTR)
				continue;
			saved_errno = errno;
			close(fd);
			errno = saved_errno;
			return -1;
		}
		p += n;
		buflen -= n;
	}

	close(fd);
	return 0;
}
