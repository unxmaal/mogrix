/*
 * mincore() compat for IRIX 6.5
 *
 * IRIX doesn't have mincore(). This stub pretends all pages are
 * resident in memory (sets all vec bytes to 1). This satisfies
 * WebKit's BlockDirectory which uses mincore to check if GC heap
 * pages are resident before accessing them.
 *
 * Uses typedef instead of #include <sys/types.h> to avoid
 * IRIX sysroot header issues when compiling standalone.
 */

typedef char *caddr_t;
typedef unsigned int size_t;

extern long sysconf(int);
extern void *memset(void *, int, size_t);

#define _SC_PAGESIZE 1

int mincore(caddr_t addr, size_t length, char *vec)
{
    /* Report all pages as resident */
    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size <= 0)
        page_size = 16384; /* IRIX default */
    size_t pages = (length + page_size - 1) / page_size;
    memset(vec, 1, pages);
    return 0;
}
