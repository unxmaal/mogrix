/* Mogrix IRIX compatibility - getopt_long shim
 * Wraps IRIX getopt and adds getopt_long functionality
 */
#ifndef _MOGRIX_GETOPT_H
#define _MOGRIX_GETOPT_H

/* Include IRIX's native getopt first to get optarg, optind, opterr */
#include_next <getopt.h>

/* Option structure for getopt_long */
struct option {
    const char *name;    /* Long option name */
    int has_arg;         /* no_argument, required_argument, or optional_argument */
    int *flag;           /* If non-NULL, set *flag to val when option is found */
    int val;             /* Value to return or store in *flag */
};

/* Values for has_arg */
#ifndef no_argument
#define no_argument       0
#endif
#ifndef required_argument
#define required_argument 1
#endif
#ifndef optional_argument
#define optional_argument 2
#endif

/* Minimal getopt_long implementation using getopt
 * This is a simplified version that ignores long options and falls back to getopt
 */
static inline int getopt_long(int argc, char * const argv[],
                              const char *optstring,
                              const struct option *longopts,
                              int *longindex)
{
    /* Ignore longopts and just use getopt */
    (void)longopts;
    if (longindex) *longindex = -1;
    return getopt(argc, argv, optstring);
}

/* getopt_long_only - same as getopt_long in our minimal impl */
static inline int getopt_long_only(int argc, char * const argv[],
                                   const char *optstring,
                                   const struct option *longopts,
                                   int *longindex)
{
    return getopt_long(argc, argv, optstring, longopts, longindex);
}

#endif /* _MOGRIX_GETOPT_H */
