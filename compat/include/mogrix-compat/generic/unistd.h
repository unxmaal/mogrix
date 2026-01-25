/*
 * mogrix-compat/generic/unistd.h
 *
 * Wrapper that includes the real unistd.h and adds compatibility
 * definitions for sysconf constants that may be missing from IRIX.
 */

#ifndef _MOGRIX_COMPAT_UNISTD_H
#define _MOGRIX_COMPAT_UNISTD_H

/* Include the real IRIX unistd.h */
#include_next <unistd.h>

/*
 * IRIX uses _SC_NPROC_ONLN instead of the POSIX _SC_NPROCESSORS_ONLN.
 * Map the POSIX name to the IRIX name for compatibility.
 */
#ifndef _SC_NPROCESSORS_ONLN
#ifdef _SC_NPROC_ONLN
#define _SC_NPROCESSORS_ONLN _SC_NPROC_ONLN
#else
/* Fallback - define a value that sysconf will return -1 for */
#define _SC_NPROCESSORS_ONLN 9999
#endif
#endif

/*
 * _SC_PHYS_PAGES - number of physical memory pages
 * IRIX doesn't have this. Define a stub value that sysconf will
 * return -1 for, and the caller should handle gracefully.
 */
#ifndef _SC_PHYS_PAGES
#define _SC_PHYS_PAGES 9998
#endif

/*
 * _SC_PAGESIZE / _SC_PAGE_SIZE - size of a page in bytes
 * IRIX should have this but ensure it's defined.
 */
#ifndef _SC_PAGESIZE
#ifdef _SC_PAGE_SIZE
#define _SC_PAGESIZE _SC_PAGE_SIZE
#endif
#endif

#endif /* _MOGRIX_COMPAT_UNISTD_H */
