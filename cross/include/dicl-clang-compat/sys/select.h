/* sys/select.h - wrapper for IRIX sys/select.h
 *
 * IRIX's sys/select.h only defines fd_set and FD_* macros but
 * does not declare select(). The declaration is in <unistd.h>
 * and <sys/time.h> instead. This causes problems with gnulib
 * which expects sys/select.h to declare select().
 *
 * When _XOPEN_SOURCE is set, IRIX sys/time.h defines a static
 * select() wrapper around __xpg4_select(). We must NOT also declare
 * extern select() in that case — the conflicting linkage causes
 * duplicate symbols at link time.
 *
 * Strategy: Only provide the extern declaration if sys/time.h hasn't
 * been included yet. Code that includes sys/time.h gets the static
 * wrapper directly.
 */
#ifndef _CLANG_COMPAT_SYS_SELECT_H
#define _CLANG_COMPAT_SYS_SELECT_H

#include_next <sys/select.h>

/* If sys/time.h hasn't been included yet, include it now.
 * sys/time.h provides struct timeval (full definition, not just
 * forward declaration) and declares select().
 * On Linux, sys/select.h provides these directly; on IRIX they
 * live in sys/time.h. Without this, code that uses struct timeval
 * as a value (not just pointer) after including sys/select.h
 * gets an incomplete-type error from the forward declaration. */
#ifndef _SYS_TIME_H
#include <sys/time.h>
#endif /* _SYS_TIME_H */

#endif /* _CLANG_COMPAT_SYS_SELECT_H */
