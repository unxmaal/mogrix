/*
 * mogrix-compat/generic/fnmatch.h
 *
 * Wrapper that includes the real fnmatch.h and adds GNU extensions
 * for IRIX compatibility.
 */

#ifndef _MOGRIX_COMPAT_FNMATCH_H
#define _MOGRIX_COMPAT_FNMATCH_H

/* Include the real IRIX fnmatch.h */
#include_next <fnmatch.h>

/*
 * FNM_CASEFOLD - case-insensitive matching (GNU extension)
 *
 * IRIX fnmatch doesn't support this flag. We define it to 0
 * so it has no effect - matching will be case-sensitive.
 * This allows code that uses FNM_CASEFOLD to compile and run,
 * though without case-insensitive matching behavior.
 */
#ifndef FNM_CASEFOLD
#define FNM_CASEFOLD 0
#endif

#endif /* _MOGRIX_COMPAT_FNMATCH_H */
