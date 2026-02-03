# IRIX Platform Quirks

> **For AI agents**: These are IRIX-specific issues that may affect package porting.
> Check this list when encountering unexpected build or runtime failures.

---

## Header Namespace Collisions

IRIX system headers define some symbols that conflict with common application code.

### `struct comment` in pwd.h

**Location**: `/usr/include/pwd.h` (line 66)

**Problem**: IRIX's pwd.h defines `struct comment` for an obscure password file feature. Applications that define their own `struct comment` will fail to compile.

**Symptom**:
```
error: redefinition of 'comment'
./app.h:XX: struct comment {
/opt/irix-sysroot/usr/include/pwd.h:66: note: previous definition is here
```

**Solution**: Rename the application's struct. Example patch:
```bash
sed -i 's/struct comment/struct app_comment/g' *.h *.c
```

**Affected packages**: tree-pkg (renamed to `struct tree_comment`)

---

## Platform Detection Macros

Different packages use different macros to detect Linux/Unix platforms.

| Macro | Common In | IRIX Equivalent |
|-------|-----------|-----------------|
| `LINUX` | Older code | `__sgi` |
| `__linux__` | Modern code | `__sgi` |
| `linux` | Legacy | `__sgi` or `sgi` |
| `__unix__` | Portable code | Already defined on IRIX |

**Note**: IRIX defines `__sgi`, `__sgi__`, `sgi`, `__unix__`, `__mips__`, and `_MIPS_SIM`.

When porting, check what platform macro the source uses and add appropriate `#elif defined(__sgi)` blocks or sed replacements.

---

## Missing/Different Functions

### vsnprintf C99 behavior

**Problem**: IRIX vsnprintf does NOT return required buffer size when passed NULL/0.

```c
// C99 (Linux): returns 5
int n = vsnprintf(NULL, 0, "hello");

// IRIX: returns -1
int n = vsnprintf(NULL, 0, "hello");
```

**Solution**: Use iterative buffer doubling or inject mogrix's vasprintf.

**Affected packages**: popt, rpm (both inject vasprintf compat function)

### O_NOFOLLOW

**Problem**: IRIX doesn't support O_NOFOLLOW flag - returns EINVAL.

**Solution**: Strip the flag in openat-compat.c (already done in mogrix compat layer).

### Thread-local storage (__thread)

**Problem**: IRIX rld doesn't support `__tls_get_addr`.

**Symptom**: Undefined symbol `__tls_get_addr` at runtime.

**Solution**: Patch source to use static variables or pthreads instead.

**Affected packages**: rpm (patched to remove `static __thread`)

---

## Filesystem Differences

### /dev/fd behavior

**Problem**: IRIX has /dev/fd but `utimes("/dev/fd/N", ...)` doesn't work.

**Solution**: Use filename-based utimes instead of fd-based.

**Affected packages**: rpm (futimens disabled, uses utimensat with pathname)

### Long filenames in tar

**Problem**: IRIX tar corrupts GNU-style long filenames (>100 chars).

**Solution**: Use `createrepo_c --simple-md-filenames` for repositories.

---

## Type Size Differences

### mode_t

**Problem**: IRIX mode_t is `unsigned long` (4 bytes), Linux is `unsigned int`.

**Symptom**: Format warnings like:
```
warning: format specifies type 'unsigned int' but argument has type 'mode_t' (aka 'unsigned long')
```

**Solution**: Use `%lo` format specifier or cast to `unsigned int`.

---

## Adding New Quirks

When you encounter a new IRIX-specific issue:

1. Document it here with:
   - Location/cause
   - Symptom (error message)
   - Solution
   - Affected packages

2. If it affects multiple packages, consider:
   - Adding a compat header in `cross/include/dicl-clang-compat/`
   - Adding a compat function in `compat/runtime/`
   - Creating a generic rule pattern

3. If it's package-specific:
   - Handle in the package's YAML rule file
   - Add a comment explaining why
