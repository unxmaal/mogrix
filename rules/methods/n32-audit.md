# N32 Audit: MIPS N32 Silent Corruption Across ALL Built Packages

> **Date**: Feb 24, 2026
> **Scope**: All mogrix-built packages, ir8 bundle
> **Status**: Root cause found and fixed. Preventive rebuild recommended.

---

## Root Cause: libmogrix_compat.so with GLib Dependencies

### Symptom
64 of 111 binaries in the ir8 bundle failed with "exit 1 with no output". SYSLOG showed:
```
rld: Error: unresolvable symbol in libmogrix_compat.so: g_log_set_always_fatal
rld: Error: unresolvable symbol in libmogrix_compat.so: g_log_set_handler
rld: Fatal Error: this executable has unresolvable symbols
```

### Root Cause
`glib_critical_trap.c` was manually compiled into `libmogrix_compat.so` outside the rules system. This introduced two GLib-dependent functions (`g_signal_connect_data`, `g_signal_handlers_disconnect_by_data`) that referenced `g_log_set_always_fatal` and `g_log_set_handler` as UNDEF symbols.

Since `libmogrix_compat.so` is preloaded via `_RLDN32_LIST` into **ALL** binaries (including non-GLib ones like bzip2, sqlite3, zstd), rld tried to resolve the GLib symbols, failed, and killed the process.

### Fix
Rebuilt `libmogrix_compat.so` using ONLY the documented sources from `rules/methods/compat-functions.md`:
```bash
irix-cc -shared -fPIC -o /tmp/libmogrix_compat.so \
    compat/stdlib/bsearch.c \
    compat/sys/socketpair.c \
    compat/sys/shm_open.c \
    compat/sys/mincore.c \
    compat/runtime/muloti4.c \
    compat/runtime/divti3.c \
    compat/stdlib/mkdtemp.c \
    compat/error/strerror_r.c \
    compat/string/memmem.c \
    patches/shared/mogrix_crash_handler.c \
    -I compat/include -I patches/shared
```

### Result
- **Before fix**: 64/111 binaries failed (rld symbol resolution)
- **After fix**: 112/112 ELF binaries load correctly
- **Functional tests**: 11/11 pass (compression round-trips, sqlite3, xmllint, pcre2grep, fribidi, fontconfig, image tools)

### Rule
**NEVER compile GLib/GTK-dependent code into libmogrix_compat.so.** It's preloaded into ALL binaries. Any UNDEF symbols that can't be resolved by non-GLib binaries will kill them silently.

---

## GOT Size Audit

### Overview
Audited all 237 .so files in `/opt/sgug-staging/usr/sgug/lib32/`:
- **217** mogrix-built (Feb 7-19, before `-Bsymbolic` fix on Feb 20) — missing SYMBOLIC
- **5** post-Feb-20 (have `-Bsymbolic`): libicudata, libjavascriptcoregtk, libmogrix_compat, libpipeline, libwebkit2gtk
- **~15** SGUG pre-existing (Aug 2019, can't change): libXm, libXaw, libXmu, etc.

### IRIX rld GOT Limit
IRIX rld has a ~4370 global GOT entry limit per library (from Ghidra decompilation). Libraries exceeding this crash with "unresolvable symbol" errors.

### High-Risk Libraries (Global GOT > 1000, no SYMBOLIC)

| Library | Global GOT | Risk Level |
|---------|-----------|------------|
| libepoxy.so | 3417 | HIGH (78% of limit) |
| libQt5Widgets.so | 3207 | HIGH (73%) |
| libstdc++.so | 2430 | MEDIUM (56%) |
| libXm.so.2 | 1959 | MEDIUM (SGUG, can't fix) |
| libQt5Gui.so | 1840 | MEDIUM |
| libtcl8.6.so | 1779 | MEDIUM |
| libicui18n.so | 1771 | MEDIUM |
| libXm.so.1 | 1575 | LOW (SGUG) |
| libQt5XcbQpa.so | 1217 | LOW |
| libperl.so | 1171 | LOW |
| libQt5Network.so | 1155 | LOW |

### Current Status
None exceed the ~4370 limit. All binaries load correctly. This is a **preventive** concern — as bundles grow and more libraries are loaded simultaneously, GOT pressure increases.

### `-Bsymbolic` Impact
Adding `-Bsymbolic` reduces global GOT by making internal function calls resolve locally (within the library) rather than through the GOT. The Feb 20 fix to `irix-ld` adds this automatically for all new builds.

### Rebuild Recommendation
Rebuilding the 217 pre-Feb-20 libs with `-Bsymbolic` would reduce GOT pressure. Priority order:
1. **libepoxy** (highest risk, 3417 global GOT)
2. **Qt5 libraries** (3207, 1840, 1217, 1155)
3. **libstdc++** (2430)
4. **ICU libraries** (1771)
5. Everything else (low risk, < 1000 entries)

This is not urgent — all binaries work today. But should be done before adding more large library dependencies.

---

## Hypothesis Testing Summary

| Hypothesis | Result |
|-----------|--------|
| rld failures from missing `-Bsymbolic` / `--mips-got-size` | **DISPROVED** — root cause was GLib UNDEFs in libmogrix_compat.so |
| GOT overflow causing failures | **NOT YET** — max is 3417/4370, but preventive rebuild recommended |
| LLVM codegen corruption (%zu, endian) | **NOT DETECTED** — all functional tests pass |
| GLib 2.80 n32 fix not generic | **DEFERRED** — ir8 works, can be addressed when more packages need it |

---

## Files Modified

| File | Change |
|------|--------|
| `/opt/sgug-staging/usr/sgug/lib32/libmogrix_compat.so` | Rebuilt without glib_critical_trap.c |
| `rules/INDEX.md` | Added anti-pattern for GLib in libmogrix_compat.so |
| `rules/methods/compat-functions.md` | Added CRITICAL warning about GLib deps |
| `rules/methods/n32-audit.md` | This document |

---

## Diagnostic Methodology

For future audits:

1. **Check SYSLOG first**: `tail -200 /var/adm/SYSLOG` — rld's `error()`/`fatal()` log here
2. **par_trace failing binaries**: Shows exactly where rld fails (which .so, which symbol)
3. **elfdump -s on IRIX**: Shows .dynsym including UNDEF symbols
4. **readelf -d for GOT info**: MIPS_LOCAL_GOTNO, MIPS_GOTSYM, MIPS_SYMTABNO
5. **readelf -d for SYMBOLIC**: Presence of DT_SYMBOLIC flag
6. **File dates**: `ls -la` to identify mogrix-built vs SGUG pre-existing

### Key rld Error Messages
- `"unresolvable symbol in <lib>: <sym>"` — UNDEF symbol can't be found in NEEDED chain
- `"Cannot Successfully map soname '<lib>'"` — .so file not found in library search path
- `"this executable has unresolvable symbols"` — fatal, process killed after above errors
