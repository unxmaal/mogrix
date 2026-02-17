/*
 * irix_serial_compat.h - IRIX compatibility for serial port handling
 *
 * Provides:
 * - CRTSCTS (hardware flow control flag, not in IRIX termios)
 * - High baud rates B230400+ (IRIX maxes at B115200)
 * - timespec_get() (C11, not in IRIX libc)
 * - asm/termios.h replacement (TIOCM_* are in sys/ioctl.h on IRIX)
 */

#ifndef IRIX_SERIAL_COMPAT_H
#define IRIX_SERIAL_COMPAT_H

#include <sys/ioctl.h>
#include <time.h>

/* CRTSCTS: IRIX termios doesn't define this. Define as 0 (no-op).
 * IRIX serial hardware flow control uses termiox RTSXOFF/CTSXON instead. */
#ifndef CRTSCTS
#define CRTSCTS 0
#endif

/* High baud rates not supported on IRIX hardware.
 * Define as 0 so config parsing doesn't crash; cfsetospeed will reject them. */
#ifndef B230400
#define B230400 0
#endif
#ifndef B460800
#define B460800 0
#endif
#ifndef B576000
#define B576000 0
#endif
#ifndef B921600
#define B921600 0
#endif
#ifndef B1000000
#define B1000000 0
#endif
#ifndef B1500000
#define B1500000 0
#endif
#ifndef B2000000
#define B2000000 0
#endif
#ifndef B2500000
#define B2500000 0
#endif
#ifndef B3000000
#define B3000000 0
#endif
#ifndef B3500000
#define B3500000 0
#endif
#ifndef B4000000
#define B4000000 0
#endif

/* timespec_get (C11) - IRIX doesn't have it. Use clock_gettime instead. */
#ifndef TIME_UTC
#define TIME_UTC 1
#endif

static inline int timespec_get(struct timespec *ts, int base) {
    (void)base;
    return clock_gettime(CLOCK_REALTIME, ts) == 0 ? TIME_UTC : 0;
}

#endif /* IRIX_SERIAL_COMPAT_H */
