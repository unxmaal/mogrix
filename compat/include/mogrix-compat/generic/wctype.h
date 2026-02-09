/*
 * mogrix-compat/generic/wctype.h
 *
 * Wrapper that includes the real wctype.h and adds C99 functions
 * hidden behind IRIX's __c99 guard.
 */

#ifndef _MOGRIX_COMPAT_WCTYPE_H
#define _MOGRIX_COMPAT_WCTYPE_H

/* Include the real IRIX wctype.h */
#include_next <wctype.h>

#ifdef __cplusplus
extern "C" {
#endif

/* C99 7.25.2.1.3 iswblank - gated behind __c99 in IRIX wchar_core.h */
#ifndef iswblank
extern int iswblank(wint_t);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _MOGRIX_COMPAT_WCTYPE_H */
