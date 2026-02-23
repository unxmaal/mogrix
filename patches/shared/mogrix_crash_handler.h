/*
 * mogrix_crash_handler.h - Generic crash diagnostic handler for IRIX
 *
 * Self-initializing via __attribute__((constructor)).
 * Set MOGRIX_CRASH_DEBUG=1 to enable.
 * Optionally set MOGRIX_CRASH_DIR=/path to control log output directory.
 *
 * Catches: SIGSEGV, SIGBUS, SIGABRT, SIGFPE, SIGPIPE, SIGILL, SIGTRAP
 *
 * When a crash occurs:
 *   - Prints PC, RA, registers, and resolved stack backtrace to stderr
 *   - Writes same info to $MOGRIX_CRASH_DIR/mogrix_crash_<pid>.log
 *   - Re-raises signal for core dump
 *
 * Also installs atexit() handler — if the process exits cleanly (no signal),
 * writes $MOGRIX_CRASH_DIR/mogrix_exit_<pid>.log to confirm it wasn't killed.
 *
 * On init, writes $MOGRIX_CRASH_DIR/mogrix_init_<pid>.log to confirm loading.
 *
 * Usage (preload via libmogrix_compat.so — all bundled binaries get it):
 *   export MOGRIX_CRASH_DEBUG=1
 *   export MOGRIX_CRASH_DIR=/usr/people/edodd  # optional
 *   <run bundle normally>
 *
 * Usage (compile into specific package):
 *   add_source: [mogrix_crash_handler.c, mogrix_crash_handler.h]
 *   cp %{_sourcedir}/mogrix_crash_handler.c src/
 *   # add to build system
 */
#ifndef MOGRIX_CRASH_HANDLER_H
#define MOGRIX_CRASH_HANDLER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Call explicitly if you need to control init order. Otherwise the
 * constructor attribute handles it automatically. */
void mogrix_crash_init(void);

#ifdef __cplusplus
}
#endif

#endif /* MOGRIX_CRASH_HANDLER_H */
