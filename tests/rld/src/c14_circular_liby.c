/*
 * C14: Circular NEEDED test - library Y
 */
static int y_value = 20;

int liby_get(void) { return y_value; }
int liby_set(int v) { y_value = v; return v; }
