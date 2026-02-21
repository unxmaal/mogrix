/*
 * A01: 2-segment layout test - library
 * Simple .so to verify LOAD segment count (must be exactly 2: RE+RW).
 * -z separate-code produces 3 segments which crashes rld.
 */
int segment_func(int x) { return x + 1; }
int segment_data = 42;
