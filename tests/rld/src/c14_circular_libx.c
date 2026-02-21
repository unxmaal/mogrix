/*
 * C14: Circular NEEDED test - library X
 *
 * libx calls a function from liby, and liby calls one from libx.
 * This creates a circular NEEDED dependency. We want to know if
 * rld handles this or crashes.
 *
 * Note: we can't actually create circular NEEDED at link time easily,
 * so we use dlopen to simulate it. Each lib only calls its own functions.
 */
static int x_value = 10;

int libx_get(void) { return x_value; }
int libx_set(int v) { x_value = v; return v; }
