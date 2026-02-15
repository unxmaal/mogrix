/*
 * arc4random platform hooks for IRIX.
 *
 * Provides mutex locking, fork detection, and /dev/zero-based mmap
 * (IRIX has no MAP_ANON). Included by compat/arc4random.h when
 * __sgi is defined.
 */

#include <sys/mman.h>
#include <pthread.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>

static pthread_mutex_t arc4random_mtx = PTHREAD_MUTEX_INITIALIZER;
#define _ARC4_LOCK()   pthread_mutex_lock(&arc4random_mtx)
#define _ARC4_UNLOCK() pthread_mutex_unlock(&arc4random_mtx)
#define _ARC4_ATFORK(f) pthread_atfork(NULL, NULL, (f))

static inline void
_getentropy_fail(void)
{
	raise(SIGKILL);
}

static volatile sig_atomic_t _rs_forked;

static inline void
_rs_forkhandler(void)
{
	_rs_forked = 1;
}

static inline void
_rs_forkdetect(void)
{
	static pid_t _rs_pid = 0;
	pid_t pid = getpid();
	if (_rs_pid == 0 || _rs_pid == 1 || _rs_pid != pid || _rs_forked) {
		_rs_pid = pid;
		_rs_forked = 0;
		if (rs)
			memset(rs, 0, sizeof(*rs));
	}
}

/* IRIX has no MAP_ANON -- use /dev/zero */
static inline void *
_arc4_mmap_anon(size_t len)
{
	int fd;
	void *p;

	fd = open("/dev/zero", O_RDWR);
	if (fd == -1)
		return MAP_FAILED;
	p = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
	close(fd);
	return p;
}

static inline int
_rs_allocate(struct _rs **rsp, struct _rsx **rsxp)
{
	if ((*rsp = _arc4_mmap_anon(sizeof(**rsp))) == MAP_FAILED)
		return -1;
	if ((*rsxp = _arc4_mmap_anon(sizeof(**rsxp))) == MAP_FAILED) {
		munmap(*rsp, sizeof(**rsp));
		*rsp = NULL;
		return -1;
	}
	_ARC4_ATFORK(_rs_forkhandler);
	return 0;
}
