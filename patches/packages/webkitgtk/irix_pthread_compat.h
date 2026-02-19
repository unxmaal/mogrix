/*
 * IRIX pthread compatibility for WebKitGTK.
 *
 * IRIX 6.5 lacks:
 *   - pthread_getattr_np() -- GNU extension to get attributes of a running thread
 *   - pthread_attr_getstack() -- POSIX.1-2001 combined stack addr+size query
 *
 * IRIX provides the older separate functions:
 *   - pthread_attr_getstackaddr() / pthread_attr_setstackaddr()
 *   - pthread_attr_getstacksize() / pthread_attr_setstacksize()
 *
 * This header provides inline compatibility wrappers.  Guarded by __sgi so
 * it is a no-op when compiled on other platforms.
 */
#ifdef __sgi

#include <sys/resource.h>
#include <unistd.h>

/*
 * pthread_getattr_np: get attributes of a running thread.
 *
 * IRIX cannot query a running thread's actual stack region, so we
 * approximate using getrlimit(RLIMIT_STACK) for size and a local
 * variable's address to estimate the stack base.  This is sufficient
 * for WebKit's stack overflow detection (StackBounds).
 */
static inline int
pthread_getattr_np(pthread_t thread, pthread_attr_t *attr)
{
    (void)thread;
    int rc = pthread_attr_init(attr);
    if (rc != 0)
        return rc;

    /* Stack size from resource limits */
    struct rlimit lim;
    getrlimit(RLIMIT_STACK, &lim);
    size_t stackSize = (lim.rlim_cur == RLIM_INFINITY)
                           ? (size_t)(8 * 1024 * 1024)
                           : (size_t)lim.rlim_cur;
    pthread_attr_setstacksize(attr, stackSize);

    /*
     * Approximate stack base from a local variable.
     * Stack grows downward on MIPS.  Round up SP to the next page
     * boundary as an estimate of the stack origin (top), then the
     * base (lowest address) is origin - stackSize.
     */
    char localVar;
    uintptr_t sp = (uintptr_t)&localVar;
    long pageSize = sysconf(_SC_PAGESIZE);
    uintptr_t origin = (sp + (uintptr_t)pageSize) & ~((uintptr_t)pageSize - 1);
    void *base = (void *)(origin - stackSize);
    pthread_attr_setstackaddr(attr, base);

    return 0;
}

/*
 * pthread_attr_getstack: combined query of stack address and size.
 * Wraps the separate IRIX functions.
 */
static inline int
pthread_attr_getstack(const pthread_attr_t *attr,
                      void **stackaddr, size_t *stacksize)
{
    int rc = pthread_attr_getstackaddr(attr, stackaddr);
    if (rc != 0)
        return rc;
    return pthread_attr_getstacksize(attr, stacksize);
}

#endif /* __sgi */
