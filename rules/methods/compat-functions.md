# Compat Functions Method

> **For AI agents**: READ THIS when a build fails due to missing functions.
> Check `compat/catalog.yaml` first - the function may already exist.

---

## Quick Check

Before implementing a new compat function:

```bash
# Check if function already exists
grep -r "function_name" compat/catalog.yaml
grep -r "function_name" compat/
```

---

## Adding a Compat Function

### Step 1: Check catalog.yaml

Read `compat/catalog.yaml` to see:
- If the function already exists
- Which category it belongs to (string, stdio, stdlib, posix_at, etc.)
- Which packages already use it

### Step 2: Implement the Function

Create implementation in appropriate directory:

| Category | Directory | Examples |
|----------|-----------|----------|
| String functions | `compat/string/` | strdup, strndup, strcasestr, strsep |
| I/O functions | `compat/stdio/` | getline, asprintf, funopen |
| Standard library | `compat/stdlib/` | reallocarray, mkdtemp, qsort_r |
| POSIX "at" functions | `compat/dicl/` | openat, fstatat, futimens (17 total) |
| Runtime | `compat/runtime/` | stpcpy, getprogname |

**Implementation template:**

```c
/* funcname.c - provide funcname() for IRIX
 *
 * IRIX 6.5 lacks funcname(). This implementation...
 */

#include <...>

return_type funcname(args) {
    // Implementation
}
```

### Step 3: Add Header Declaration

Add to `compat/include/mogrix-compat/generic/<header>.h`:

```c
#ifndef HAVE_FUNCNAME
return_type funcname(args);
#endif
```

### Step 4: Register in catalog.yaml

```yaml
functions:
  funcname:
    category: string  # or stdio, stdlib, etc.
    header: string.h
    source: compat/string/funcname.c
    description: "Brief description of what it does"
    packages:
      - package1
      - package2
```

### Step 5: Add to Package Rule

In `rules/packages/<pkg>.yaml`:

```yaml
inject_compat_functions:
  - funcname
```

---

## IRIX-Specific Quirks

### vsnprintf(NULL, 0) Returns -1

IRIX vsnprintf does NOT support C99 behavior:
- C99: `vsnprintf(NULL, 0, fmt, ap)` returns required buffer size
- IRIX: Returns **-1**

**Solution**: Use iterative `vasprintf` from `compat/stdio/asprintf.c`

### fopencookie CRASHES

The `compat/stdio/fopencookie.c` implementation crashes on IRIX.

**Solution**: Use `funopen` instead (BSD cookie I/O)

### No lutimes()

IRIX has no way to set timestamps on symlinks without following them.

**Solution**: `utimensat()` with `AT_SYMLINK_NOFOLLOW` returns success immediately for symlinks.

---

## Common Functions by Package Type

### String-heavy packages (text processing)

```yaml
inject_compat_functions:
  - strdup
  - strndup
  - strcasestr
  - strsep
```

### I/O-heavy packages (file processing)

```yaml
inject_compat_functions:
  - getline
  - asprintf
```

### Modern POSIX packages (rpm, libsolv)

```yaml
inject_compat_functions:
  - openat
  - fstatat
  - futimens
  - posix_spawn
```

---

## Testing Compat Functions

After adding a function:

1. **Rebuild the package**:
   ```bash
   MOGRIX_ROOT=/path/to/mogrix mogrix convert <srpm> -o /tmp/test
   MOGRIX_ROOT=/path/to/mogrix rpmbuild --rebuild /tmp/test/*.src.rpm --nodeps
   ```

2. **Verify function is linked**:
   ```bash
   nm ~/rpmbuild/RPMS/mips/*.rpm 2>/dev/null | grep funcname
   ```

3. **Test on IRIX** (see `methods/irix-testing.md`)

---

## Files Reference

| File | Purpose |
|------|---------|
| `compat/catalog.yaml` | Registry of all compat functions |
| `compat/string/*.c` | String function implementations |
| `compat/stdio/*.c` | I/O function implementations |
| `compat/stdlib/*.c` | Standard library implementations |
| `compat/dicl/*.c` | POSIX "at" functions (openat family) |
| `compat/include/mogrix-compat/generic/*.h` | Header declarations |

---

## Syncing Headers to Staging

After editing compat headers in the repo, sync them to staging:

```bash
mogrix sync-headers
```

This copies headers from `mogrix/compat/include/` to `/opt/sgug-staging/usr/sgug/include/`.

**Important**: Never edit headers directly in `/opt/sgug-staging/`. Always edit in the repo and sync.
