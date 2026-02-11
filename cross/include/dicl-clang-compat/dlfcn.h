/*
 * DICL clang compat dlfcn.h
 * IRIX dlfcn.h doesn't define RTLD_DEFAULT (POSIX.1-2001 extension).
 * On IRIX, dlsym(NULL, name) searches the default scope, which is the
 * same behavior as RTLD_DEFAULT on other platforms.
 */
#ifndef _DICL_DLFCN_H
#define _DICL_DLFCN_H

#include_next <dlfcn.h>

#ifndef RTLD_DEFAULT
#define RTLD_DEFAULT    ((void *)0)
#endif

#endif /* _DICL_DLFCN_H */
