# Gnulib Force-Include Quirks

**Keywords:** gnulib, timezone_t, _GNU_SOURCE, _GL_CONFIG_H_INCLUDED, time.h, force-include, irix-cc, config.h
**Category:** build-system

Gnulib's replacement headers interact badly with irix-cc's `-include time.h` force-include. Three systemic fixes are needed in `cross/bin/irix-cc`.

## timezone_t Undefined (_GNU_SOURCE Ordering)

**Error:** `timezone_t` undeclared in gnulib-using packages.

**Root cause:** irix-cc force-includes `/opt/irix-sysroot/usr/include/time.h`, which triggers gnulib's generated `time.h` through the `-isystem` chain. Gnulib's `time.h` sets the `_GL_GGL_TIME_H` guard during force-include processing. At this point `_GNU_SOURCE` is not yet defined (config.h hasn't been included). The `timezone_t` typedef is guarded by `_GNU_SOURCE`, so it's skipped. Later, when source code includes `<time.h>`, the guard prevents re-processing, leaving `timezone_t` undefined.

**Fix:** Add `-D_GNU_SOURCE=1` to irix-cc `CLANG_FLAGS` BEFORE the `-include time.h` force-include line.

## "Please include config.h first" Error

**Error:** `#error "Please include config.h first"` from gnulib replacement headers.

**Root cause:** The force-include of `time.h` triggers gnulib's replacement headers (`sys/types.h`, `string.h`, etc.) before `config.h` is included. These gnulib headers check `_GL_CONFIG_H_INCLUDED` and error if not set.

**Fix:** Add `-D_GL_CONFIG_H_INCLUDED=1` to irix-cc `CLANG_FLAGS` before the `-include time.h` line. Safe because `_GL_CONFIG_H_INCLUDED` is gnulib-internal and non-gnulib code never checks it.

**Affects:** libidn2, man-db, wget2, and all gnulib-using packages.

## Time Function ac_cv Overrides

When gnulib detects missing time functions, it tries to replace them, conflicting with compat headers. Required `ac_cv` overrides for gnulib packages:

```
gl_cv_func_working_mktime=yes    # Prevents REPLACE_MKTIME=1 (rpl_mktime conflict)
ac_cv_func_timegm=yes            # Prevents HAVE_TIMEGM=0 (gnulib timegm vs compat)
ac_cv_func_gettimeofday=yes
ac_cv_func_localtime=yes
ac_cv_func_localtime_r=yes
ac_cv_func_nanosleep=yes
ac_cv_func_setitimer=yes
```

When `ac_cv_func_timegm=yes` is set, gnulib skips compiling its own `timegm.c`, so `timegm` must be provided via `inject_compat_functions`.
