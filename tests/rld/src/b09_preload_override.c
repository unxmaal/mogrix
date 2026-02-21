/*
 * B09: _RLDN32_LIST preload test - override library
 * Overrides preload_func() when loaded via _RLDN32_LIST.
 */
int preload_func(void) {
    return 42;  /* override: returns 42 */
}
