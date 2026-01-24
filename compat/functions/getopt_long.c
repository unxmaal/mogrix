/*
 * getopt_long.c - GNU-style long option parsing for IRIX
 *
 * This is a minimal implementation of getopt_long() for systems that
 * don't have it (like IRIX).
 *
 * Based on public domain implementations.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Include the header with struct option definition */
#include <getopt.h>

/* External variables used by getopt - provided by libc */
extern char *optarg;
extern int optind, opterr, optopt;

int getopt_long(int argc, char * const argv[],
                const char *optstring,
                const struct option *longopts,
                int *longindex)
{
    static int current_pos = 1;  /* Position within current argument */
    const char *oli;             /* Option letter index */

    if (optind >= argc || argv[optind] == NULL) {
        return -1;
    }

    if (argv[optind][0] != '-') {
        /* Not an option */
        return -1;
    }

    if (argv[optind][1] == '\0') {
        /* Just "-" */
        return -1;
    }

    /* Check for "--" (end of options) */
    if (argv[optind][1] == '-' && argv[optind][2] == '\0') {
        optind++;
        return -1;
    }

    /* Check for long option (starts with "--") */
    if (argv[optind][1] == '-') {
        const char *arg = argv[optind] + 2;
        const struct option *opt;
        int i;
        size_t namelen;
        const char *eq;

        /* Find '=' in the option */
        eq = strchr(arg, '=');
        namelen = eq ? (size_t)(eq - arg) : strlen(arg);

        /* Search for matching long option */
        for (i = 0, opt = longopts; opt->name; i++, opt++) {
            if (strncmp(arg, opt->name, namelen) == 0 &&
                strlen(opt->name) == namelen) {
                /* Found a match */
                if (longindex)
                    *longindex = i;

                optind++;

                if (opt->has_arg == required_argument) {
                    if (eq) {
                        optarg = (char *)(eq + 1);
                    } else if (optind < argc) {
                        optarg = argv[optind++];
                    } else {
                        if (opterr)
                            fprintf(stderr, "%s: option '--%s' requires an argument\n",
                                    argv[0], opt->name);
                        return '?';
                    }
                } else if (opt->has_arg == optional_argument) {
                    if (eq) {
                        optarg = (char *)(eq + 1);
                    } else {
                        optarg = NULL;
                    }
                } else {
                    optarg = NULL;
                }

                if (opt->flag) {
                    *opt->flag = opt->val;
                    return 0;
                }
                return opt->val;
            }
        }

        /* Unknown long option */
        if (opterr)
            fprintf(stderr, "%s: unrecognized option '--%.*s'\n",
                    argv[0], (int)namelen, arg);
        optopt = 0;
        optind++;
        return '?';
    }

    /* Short option */
    if (current_pos == 1)
        current_pos = 1;  /* Start of new argument */

    optopt = argv[optind][current_pos];

    if (optopt == ':' || (oli = strchr(optstring, optopt)) == NULL) {
        /* Unknown option */
        if (opterr && *optstring != ':')
            fprintf(stderr, "%s: invalid option -- '%c'\n", argv[0], optopt);
        current_pos++;
        if (argv[optind][current_pos] == '\0') {
            optind++;
            current_pos = 1;
        }
        return '?';
    }

    if (oli[1] == ':') {
        /* Option requires an argument */
        if (argv[optind][current_pos + 1] != '\0') {
            /* Argument is part of current argv element */
            optarg = &argv[optind][current_pos + 1];
        } else if (optind + 1 < argc) {
            /* Argument is next argv element */
            optarg = argv[optind + 1];
            optind++;
        } else {
            /* Missing argument */
            if (opterr && *optstring != ':')
                fprintf(stderr, "%s: option requires an argument -- '%c'\n",
                        argv[0], optopt);
            current_pos = 1;
            optind++;
            return (*optstring == ':') ? ':' : '?';
        }
        optind++;
        current_pos = 1;
    } else {
        /* No argument needed */
        optarg = NULL;
        current_pos++;
        if (argv[optind][current_pos] == '\0') {
            optind++;
            current_pos = 1;
        }
    }

    return optopt;
}

/* getopt_long_only is the same but allows single dash for long options */
int getopt_long_only(int argc, char * const argv[],
                     const char *optstring,
                     const struct option *longopts,
                     int *longindex)
{
    /* For simplicity, just call getopt_long */
    /* A full implementation would try long options with single dash too */
    return getopt_long(argc, argv, optstring, longopts, longindex);
}
