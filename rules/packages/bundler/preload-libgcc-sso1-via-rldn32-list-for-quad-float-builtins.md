# Preload libgcc_s.so.1 via _RLDN32_LIST for quad-float builtins

**Keywords:** libgcc_s, _RLDN32_LIST, __getf2, __multf3, __lttf2, quad-float, 128-bit, long double, libunistring, libintl, preload, rld, DT_NEEDED
**Category:** bundler

# Preload libgcc_s.so.1 via _RLDN32_LIST

## Problem
libunistring.so.5 and libintl.so.8 use 128-bit quad-float builtins (__getf2, __multf3, __lttf2, __addtf3, __subtf3, etc.) from libgcc_s.so.1, but neither declares DT_NEEDED libgcc_s.so.1. IRIX rld only resolves symbols through declared NEEDED chains — having libgcc_s.so.1 present in the same directory is NOT enough.

## Symptoms
- Binary exits immediately with no output (exit 1)
- stderr shows: `rld: <lib>: unresolved symbol __getf2` (and similar quad-float symbols)
- Affects any bundle containing libunistring or libintl (curl, git, gettext, etc.)

## Fix
Add libgcc_s.so.1 to the _RLDN32_LIST preload list in bundle wrapper scripts. This makes all libgcc_s symbols globally available to every shared library in the process, regardless of NEEDED chains.

In mogrix/bundle.py, the preload loop includes libgcc_s.so.1 alongside libmogrix_compat.so and irix_rld_stubs.so.

## Root Cause
MIPS N32 ABI uses 128-bit long double. Many libraries use long double operations that compile to libgcc_s builtins, but gcc doesn't always add -lgcc_s to the link, so the .so files lack DT_NEEDED libgcc_s.so.1. On Linux this works because the dynamic linker searches all loaded objects; on IRIX rld it fails.
