/*
 * mogrix_crash_handler.h - Generic crash diagnostic handler for IRIX
 *
 * Self-initializing via __attribute__((constructor)).
 * Set MOGRIX_CRASH_DEBUG=1 to enable.
 * When a crash occurs, prints PC, RA, registers, and stack dump to stderr.
 *
 * Usage: Just compile and link mogrix_crash_handler.c into your program.
 *        No code changes needed — constructor handles initialization.
 *        Set MOGRIX_CRASH_DEBUG=1 at runtime to activate.
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
