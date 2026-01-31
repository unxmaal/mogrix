# Autoconf Cross-Compilation Method

> **For AI agents**: READ THIS for packages using ./configure (autoconf).
> Most packages in the tdnf chain use autoconf.

---

## Identifying Autoconf Packages

Look for:
- `./configure` script in source
- `configure.ac` or `configure.in` files
- `%configure` macro in spec file

---

## Common Patterns

### 1. GNU ld for Shared Libraries

IRIX requires GNU ld (not LLD) for shared libraries:

```yaml
export_vars:
  LD: "/opt/cross/bin/mips-sgi-irix6.5-ld.bfd"
```

**Why**: GNU ld produces correct 2-LOAD segment layout. LLD produces 3-LOAD segments which crashes IRIX rld.

### 2. Libtool Fixes

Most autoconf packages need libtool fixes after `%configure`:

```yaml
spec_replacements:
  - pattern: "%configure"
    replacement: |
      %configure
      $MOGRIX_ROOT/tools/fix-libtool-irix.sh libtool || exit 1
```

See `methods/text-replacement.md` for why we use safepatch, not sed.

### 3. Autoconf Cache Overrides

When configure's runtime tests can't work during cross-compilation:

```yaml
ac_cv_overrides:
  ac_cv_func_malloc_0_nonnull: "yes"
  ac_cv_func_realloc_0_nonnull: "yes"
  ac_cv_func_strerror_r_char_p: "no"
```

**Common overrides:**

| Variable | Value | When |
|----------|-------|------|
| `ac_cv_func_malloc_0_nonnull` | `"yes"` | Almost always |
| `ac_cv_func_realloc_0_nonnull` | `"yes"` | Almost always |
| `ac_cv_func_strerror_r_char_p` | `"no"` | IRIX uses POSIX strerror_r |
| `ac_cv_func_<func>` | `"yes"` | When compat provides function |

### 4. Configure Options

Disable features that don't work on IRIX:

```yaml
configure_opts:
  - --disable-nls
  - --disable-shared  # Only if static-only needed
  - --without-python
```

---

## Minimal Autoconf Package Rule

```yaml
package: mypackage

rules:
  # Compat functions needed
  inject_compat_functions:
    - strdup
    - getline

  # Use GNU ld for shared libraries
  export_vars:
    LD: "/opt/cross/bin/mips-sgi-irix6.5-ld.bfd"

  # Autoconf cache overrides
  ac_cv_overrides:
    ac_cv_func_malloc_0_nonnull: "yes"

  # Fix libtool after configure
  spec_replacements:
    - pattern: "%configure"
      replacement: |
        %configure
        $MOGRIX_ROOT/tools/fix-libtool-irix.sh libtool || exit 1
```

---

## Example: popt.yaml (Simple)

```yaml
package: popt

add_patch:
  - popt-disable-stpcpy.patch
  - popt-disable-mbsrtowcs.patch

rules:
  inject_compat_functions:
    - strdup
    - strndup
    - vasprintf

  configure_opts:
    - --disable-nls

  export_vars:
    LD: "/opt/cross/bin/mips-sgi-irix6.5-ld.bfd"

  ac_cv_overrides:
    ac_cv_func_strerror_r_char_p: "no"
    ac_cv_func_mbsnrtowcs: "no"
    ac_cv_func_vasprintf: "yes"

  spec_replacements:
    - pattern: "%configure"
      replacement: |
        %configure
        $MOGRIX_ROOT/tools/fix-libtool-irix.sh libtool || exit 1

  skip_find_lang: true
  files_no_lang: true
```

---

## Example: curl.yaml (Complex)

```yaml
package: curl

rules:
  inject_compat_functions:
    - strdup
    - strndup
    - getline

  # Out-of-tree build
  spec_replacements:
    - pattern: "%configure"
      replacement: |
        mkdir build && cd build
        ../configure --host=mips-sgi-irix6.5 \
          --with-ssl=/usr/sgug \
          --without-libpsl \
          ...

  export_vars:
    LD: "/opt/cross/bin/mips-sgi-irix6.5-ld.bfd"
    CFLAGS: "-D__mips64=1"  # Fix architecture detection

  ac_cv_overrides:
    # Many overrides for SSL, socket functions, etc.
```

---

## Troubleshooting

### "libtool: link: cannot find -lfoo"

Libtool can't find a dependency. Check:
1. Is the dependency staged? `ls /opt/sgug-staging/usr/sgug/lib32/libfoo*`
2. Add `-L/opt/sgug-staging/usr/sgug/lib32` to LDFLAGS

### "configure: error: cannot run test program"

Cross-compilation can't run target binaries. Add to `ac_cv_overrides`:
```yaml
ac_cv_<test_name>: "yes"  # or appropriate value
```

### Shared library not built

Libtool defaulted to static-only. Ensure:
1. `fix-libtool-irix.sh` runs after configure
2. GNU ld is used (export LD)

---

## Files Reference

| Example | Complexity | Key Features |
|---------|------------|--------------|
| `rules/packages/popt.yaml` | Simple | Basic autoconf with patches |
| `rules/packages/libxml2.yaml` | Medium | Libtool fixes, python disable |
| `rules/packages/curl.yaml` | Complex | Out-of-tree, many options |
