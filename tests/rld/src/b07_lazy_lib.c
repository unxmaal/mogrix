/*
 * B07: Lazy binding test - library
 * Basic cross-library call. If lazy binding is broken, this crashes.
 */
#include <string.h>

int lazy_add(int a, int b) { return a + b; }
int lazy_mul(int a, int b) { return a * b; }

/* Use strlen to pull in libc lazy binding too */
int lazy_len(const char *s) { return (int)strlen(s); }
