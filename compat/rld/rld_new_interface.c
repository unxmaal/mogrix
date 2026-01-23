/*
 * _rld_new_interface stub for cross-compilation
 *
 * On IRIX, _rld_new_interface is provided by rld (the runtime linker) and
 * accessed via an undefined symbol in libc. For cross-compilation where we
 * statically link, we need a stub that returns NULL to indicate the
 * functionality is not available.
 */

#if defined(__sgi)

#include <stddef.h>
#include <stdarg.h>

/* Stub for _rld_new_interface - returns NULL to indicate not available */
void *_rld_new_interface(unsigned int operation, ...)
{
    (void)operation;
    return NULL;
}

#endif /* __sgi */
