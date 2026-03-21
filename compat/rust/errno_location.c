/* errno_location — provide errno access for Rust std.
 * Rust's libc crate calls __errno_location() on some platforms.
 * IRIX uses __oserror(). Provide an alias. */
extern int *__oserror(void);
int *errno_location(void) { return __oserror(); }
int *__errno_location(void) { return __oserror(); }
