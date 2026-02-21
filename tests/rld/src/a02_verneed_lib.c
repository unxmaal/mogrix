/*
 * A02: No VERNEED test - library
 * Simple .so; verify_elf checks that VERNEED/VERSYM tags were stripped.
 */
int verneed_func(int x) { return x * 2; }
