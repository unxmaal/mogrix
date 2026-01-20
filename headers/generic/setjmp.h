/* setjmp.h - replacement for IRIX setjmp.h when using clang
 *
 * IRIX's native setjmp.h has complex conditional compilation that
 * doesn't work well with clang. This provides a simplified version.
 */

#ifndef _SETJMP_H
#define _SETJMP_H

#include <sys/types.h>

/* Buffer size for jmp_buf - same as IRIX N32 */
#define _JBLEN 28
#define _SIGJBLEN 28

/* jmp_buf type */
typedef __int64_t jmp_buf[_JBLEN];

/* sigjmp_buf type */
typedef __int64_t sigjmp_buf[_SIGJBLEN];

#ifdef __cplusplus
extern "C" {
#endif

/* Standard setjmp/longjmp */
extern int setjmp(jmp_buf);
extern void longjmp(jmp_buf, int);

/* POSIX sigsetjmp/siglongjmp */
extern int sigsetjmp(sigjmp_buf, int);
extern void siglongjmp(sigjmp_buf, int);

/* BSD-style _setjmp/_longjmp (don't save signal mask) */
extern int _setjmp(jmp_buf);
extern void _longjmp(jmp_buf, int);

#ifdef __cplusplus
}
#endif

#endif /* _SETJMP_H */
