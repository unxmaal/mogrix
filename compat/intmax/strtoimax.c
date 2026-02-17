/* wcstoimax.c / wcstoumax.c for IRIX
 *
 * IRIX 6.5 has strtoimax/strtoumax in libc but lacks the wide-char variants.
 * These are thin wrappers around wcstoll/wcstoull.
 */

#if defined(__sgi)

#include <wchar.h>

/* intmax_t = long long on MIPS n32 */
typedef long long intmax_t;
typedef unsigned long long uintmax_t;

intmax_t wcstoimax(const wchar_t *nptr, wchar_t **endptr, int base)
{
    return wcstoll(nptr, endptr, base);
}

uintmax_t wcstoumax(const wchar_t *nptr, wchar_t **endptr, int base)
{
    return wcstoull(nptr, endptr, base);
}

#endif /* __sgi */
