/*
 * Mogrix libdicl-compatible stdlib.h replacement header
 *
 * This header is compatible with libdicl's stdlib.h, providing
 * the ld_* function declarations and macros that redirect
 * standard functions to our implementations.
 */

#ifndef MOGRIX_DICL_STDLIB_H
#define MOGRIX_DICL_STDLIB_H

/* Include the real stdlib.h first */
#if defined(__GNUC__)
#include_next <stdlib.h>
#else
#include "/usr/include/stdlib.h"
#endif

#if defined(__cplusplus)
extern "C" {
#endif

/* Redirect standard functions to our ld_* implementations */
#undef strtod
#define strtod ld_strtod
extern double ld_strtod(const char *, char **);

#undef strtold
#define strtold ld_strtold
extern long double ld_strtold(const char *, char **);

#undef strtol
#define strtol ld_strtol
extern long int ld_strtol(const char *, char **, int);

#undef strtoll
#define strtoll ld_strtoll
extern long long int ld_strtoll(const char *, char **, int);

#undef strtoul
#define strtoul ld_strtoul
extern unsigned long int ld_strtoul(const char *, char **, int);

#undef strtoull
#define strtoull ld_strtoull
extern unsigned long long int ld_strtoull(const char *, char **, int);

/* Program name functions - not in IRIX libc */
extern const char *getprogname(void);
extern void setprogname(const char *progname);

/* Environment functions */
extern int setenv(const char *name, const char *value, int overwrite);
extern int unsetenv(const char *name);

/* Temporary directory creation */
extern char *mkdtemp(char *template);

/* Pseudo-terminal */
extern int posix_openpt(int flags);

#if defined(__cplusplus)
}
namespace std {
  inline double ld_strtod(const char *nptr, char **endptr) {
    return ::ld_strtod(nptr, endptr);
  };
  inline long double ld_strtold(const char *nptr, char **endptr) {
    return ::ld_strtold(nptr, endptr);
  };
  inline long int ld_strtol(const char *nptr, char **endptr, int base) {
    return ::ld_strtol(nptr, endptr, base);
  };
  inline long long int ld_strtoll(const char *nptr, char **endptr, int base) {
    return ::ld_strtoll(nptr, endptr, base);
  };
  inline unsigned long int ld_strtoul(const char *nptr, char **endptr, int base) {
    return ::ld_strtoul(nptr, endptr, base);
  };
  inline unsigned long long int ld_strtoull(const char *nptr, char **endptr, int base) {
    return ::ld_strtoull(nptr, endptr, base);
  };
}
#endif

#endif /* MOGRIX_DICL_STDLIB_H */
