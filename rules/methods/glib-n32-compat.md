# GLib 2.80 n32 Compatibility

## Problem: G_DEFINE_TYPE crashes at compile time

On MIPS n32, `sizeof(gsize) == 8` but `sizeof(gpointer) == 4`. GLib 2.80 introduced
`G_STATIC_ASSERT(sizeof *(location) == sizeof(gpointer))` in the `g_once_init_enter`
and `g_once_init_leave` macros. This assertion fires for ANY code using `G_DEFINE_TYPE`,
`G_DEFINE_TYPE_WITH_CODE`, etc., because the internal `_g_type_once_init_type` resolves
to `gsize` (8 bytes) which doesn't match `gpointer` (4 bytes).

**Symptom:**
```
gthread.h:277:5: error: static assertion failed: "Expression evaluates to false"
  277 |     G_STATIC_ASSERT (sizeof *(location) == sizeof (gpointer));
```

This affects ALL GTK/GLib applications compiled against GLib 2.80 on n32.

## Fix: Two parts required

### Part 1: Version cap (insufficient alone)

Add to CFLAGS:
```
-DGLIB_VERSION_MIN_REQUIRED=G_ENCODE_VERSION(2,78)
-DGLIB_VERSION_MAX_ALLOWED=G_ENCODE_VERSION(2,78)
```

This forces `gtype.h` to use the old `gsize`-based `g_once_init_enter` path instead of
the new `GType`/pointer-based path (gated by `#if GLIB_VERSION_MAX_ALLOWED >= GLIB_VERSION_2_80`).

**Problem:** This alone is NOT sufficient. GLib 2.80 also added the `G_STATIC_ASSERT` to
the OLD macro variants in `gthread.h`, so even the gsize path hits the assertion.

### Part 2: Fixup header (required)

Create a force-include header that removes the macro wrappers:

```c
/* glib-n32-fixup.h */
#include <glib.h>

#ifdef g_once_init_enter
#undef g_once_init_enter
#endif
#ifdef g_once_init_leave
#undef g_once_init_leave
#endif
```

Add to CFLAGS: `-include glib-n32-fixup.h`

**How it works:** The `-include` directive processes the header before any source file.
It includes `<glib.h>` (which defines the broken macros), then `#undef`s them. When
subsequent source files include `<glib.h>` again, the include guard prevents
re-definition. All calls to `g_once_init_enter()` / `g_once_init_leave()` now resolve
to the underlying C functions (which accept `volatile gsize *` — correct for n32).

### Makefile example (ir8)

```makefile
CFLAGS += -Wall -O2 \
    -DGLIB_VERSION_MIN_REQUIRED=G_ENCODE_VERSION\(2,78\) \
    -DGLIB_VERSION_MAX_ALLOWED=G_ENCODE_VERSION\(2,78\) \
    -include glib-n32-fixup.h \
    $(CFLAGS_PKG)
```

## Current state

- Fix implemented for ir8 only (`patches/packages/ir8/glib-n32-fixup.h`)
- Should be made generic for all GLib/GTK apps — consider adding to `compat/include/`
  and `mogrix sync-headers`, or as a generic rule

## Files

- `patches/packages/ir8/glib-n32-fixup.h` — the fixup header
- `patches/packages/ir8/Makefile` — example CFLAGS usage
- `rules/packages/ir8.yaml` — `add_source` includes the header
