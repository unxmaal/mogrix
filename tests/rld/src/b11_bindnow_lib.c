/*
 * B11: DT_BIND_NOW test - library
 * Tests whether rld honors DT_BIND_NOW on MIPS (resolves all GOT at load).
 */
int bindnow_func(int x) { return x + 11; }
int bindnow_func2(int x) { return x * 2; }
