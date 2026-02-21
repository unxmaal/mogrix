/*
 * A06: GOTSYM threshold test - library
 *
 * Exports both data and function symbols. The GOTSYM value in .dynamic
 * partitions dynsym into local (< GOTSYM) and global (>= GOTSYM) portions.
 * Global symbols get GOT entries; local ones don't.
 */

int gotsym_data_a = 100;
int gotsym_data_b = 200;

int gotsym_func_a(void) { return gotsym_data_a; }
int gotsym_func_b(void) { return gotsym_data_b; }
int gotsym_func_sum(void) { return gotsym_data_a + gotsym_data_b; }
