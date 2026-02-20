/*
 * Stub implementations for symbols that IRIX rld can't resolve.
 *
 * IRIX rld only searches .dynsym entries at indices >= DT_MIPS_GOTSYM
 * for external resolution. Symbols below this threshold are invisible
 * even though they're GLOBAL DEFAULT. This provides stub implementations
 * for such symbols.
 *
 * Built as a plain .so (no CRT objects, no DT_INIT) and preloaded via
 * _RLDN32_LIST. Must NOT have constructors — IRIX rld doesn't correctly
 * relocate DT_INIT for _RLDN32_LIST libraries loaded at non-preferred
 * addresses, causing SIGILL.
 *
 * See rules/INDEX.md: "IRIX rld MIPS_GOTSYM threshold"
 */

#define NULL ((void*)0)

/*
 * GLib power management stubs.
 * These exist in libgio-2.0.so.0's .dynsym at indices 126 and 1023,
 * but GOTSYM is 2113. Power management has no meaning on IRIX.
 */
void *g_power_profile_monitor_dup_default(void) { return NULL; }
int g_power_profile_monitor_get_power_saver_enabled(void *monitor) {
    (void)monitor;
    return 0;
}
