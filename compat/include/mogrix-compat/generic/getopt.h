/*
 * mogrix-compat/generic/getopt.h
 *
 * Wrapper that includes the real getopt.h and adds GNU getopt_long
 * for IRIX compatibility.
 *
 * Skipped when gnulib provides its own getopt (detected via __GETOPT_PREFIX).
 * gnulib's getopt replacement uses rpl_ prefixed functions that conflict
 * with our declarations.
 */

#ifndef _MOGRIX_COMPAT_GETOPT_H
#define _MOGRIX_COMPAT_GETOPT_H

/* If gnulib is providing its own getopt, don't interfere */
#ifdef __GETOPT_PREFIX
#include_next <getopt.h>
#else

/* Include unistd.h for basic getopt */
#include_next <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

/* External variables used by getopt - ensure they're declared */
extern char *optarg;
extern int optind, opterr, optopt;

/* Basic getopt function - should be in libc but may need explicit declaration */
int getopt(int argc, char * const argv[], const char *optstring);

/* Option structure for getopt_long */
struct option {
    const char *name;     /* Long option name */
    int has_arg;          /* no_argument, required_argument, optional_argument */
    int *flag;            /* If not NULL, set *flag to val and return 0 */
    int val;              /* Value to return or assign to *flag */
};

/* Argument requirement values */
#define no_argument       0
#define required_argument 1
#define optional_argument 2

/*
 * getopt_long - parse long options (GNU extension)
 *
 * IRIX doesn't have getopt_long. We provide our own implementation.
 */
int getopt_long(int argc, char * const argv[],
                const char *optstring,
                const struct option *longopts,
                int *longindex);

/*
 * getopt_long_only - parse long options with single dash (GNU extension)
 */
int getopt_long_only(int argc, char * const argv[],
                     const char *optstring,
                     const struct option *longopts,
                     int *longindex);

#ifdef __cplusplus
}
#endif

#endif /* __GETOPT_PREFIX */

#endif /* _MOGRIX_COMPAT_GETOPT_H */
