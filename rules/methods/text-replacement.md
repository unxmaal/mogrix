# Text Replacement Methods

> **For AI agents**: READ THIS before using sed or creating patches.
> These methods ensure changes are reliable and reproducible.

---

## Decision Tree

```
Need to modify source code?
├── YES: Is it a C/header/Makefile change?
│   ├── YES → Create .patch file in patches/packages/<pkg>/
│   └── NO (spec macro/build script) → Use safepatch or spec_replacements
└── NO: Is it a build-time fix (libtool, config)?
    ├── YES: Will silent failure cause problems?
    │   ├── YES → Use safepatch (fails loudly)
    │   └── NO → sed is acceptable (truly trivial only)
    └── NO → Probably a spec_replacement in YAML
```

---

## Method 1: Patch Files (Source Code Changes)

**When**: C code, headers, Makefiles, any file in the source tarball

**Location**: `patches/packages/<pkg>/<descriptive-name>.patch`

**How to create**:

```bash
# 1. Extract source tarball
tar xzf package-1.0.tar.gz -C /tmp
cd /tmp/package-1.0

# 2. Backup original file
cp src/file.c src/file.c.orig

# 3. Make your change (edit manually or with sed for one-time use)
# Example: comment out a function
sed -i 's/^int broken_func/\/\/ int broken_func/' src/file.c

# 4. Generate patch with proper headers for -p1
diff -u src/file.c.orig src/file.c > /tmp/my-fix.patch

# 5. Edit patch header to use a/ b/ prefixes:
#    --- src/file.c.orig  →  --- a/src/file.c
#    +++ src/file.c       →  +++ b/src/file.c

# 6. Copy to mogrix
cp /tmp/my-fix.patch patches/packages/<pkg>/
```

**Add to YAML**:
```yaml
add_patch:
  - my-fix.patch
```

Mogrix will:
- Copy patch to SOURCES as Patch200+
- Add `%patch -P200 -p1` to %prep (or use %autosetup)

---

## Method 2: safepatch (Build-Time Reliable Replacement)

**When**: Libtool fixes, config file changes, anything where silent failure is dangerous

**Location**: Called from `spec_replacements` or helper scripts like `fix-libtool-irix.sh`

**Tool**: `tools/safepatch` (Perl)

**Key features**:
- **FAILS** if pattern not found (sed silently does nothing)
- **FAILS** if wrong number of matches
- Uses **exact strings** (no regex surprises)
- Creates backups by default

**Usage**:

```bash
# Basic - expects exactly 1 match
tools/safepatch libtool \
    --old 'build_libtool_libs=no' \
    --new 'build_libtool_libs=yes'

# Expect specific count
tools/safepatch source.c --old 'TODO' --new 'DONE' --count 3

# Allow any count (0 = unlimited)
tools/safepatch Makefile --old 'gcc' --new 'irix-cc' --count 0

# Preview without changing
tools/safepatch --dry-run config.h --old '#define X 0' --new '#define X 1'

# Suppress backup
tools/safepatch file.txt --old 'foo' --new 'bar' --no-backup
```

**Exit codes** (for scripting):
- 0: Success
- 1: Pattern not found
- 2: Wrong number of matches
- 3: File error
- 4: Invalid arguments
- 5: Backup failed
- 6: Write failed

**Example in spec_replacements**:
```yaml
spec_replacements:
  - pattern: "%configure"
    replacement: |
      %configure
      # Fix libtool using safepatch (fails if pattern missing)
      $MOGRIX_ROOT/tools/safepatch libtool \
          --old 'build_libtool_libs=no' \
          --new 'build_libtool_libs=yes' \
          --no-backup --quiet
```

---

## Method 3: fix-libtool-irix.sh (Standard Libtool Fix)

**When**: Any autoconf package that builds shared libraries

**Tool**: `tools/fix-libtool-irix.sh`

This script uses safepatch internally to fix common libtool issues:
- `build_libtool_libs=no` → `yes`
- `deplibs_check_method="unknown"` → `"pass_all"`
- `version_type=none` → `linux`
- Empty `soname_spec` and `library_names_spec`

**Usage in YAML**:
```yaml
spec_replacements:
  - pattern: "%configure"
    replacement: |
      %configure
      $MOGRIX_ROOT/tools/fix-libtool-irix.sh libtool || exit 1
```

---

## Method 4: sed (Trivial Changes Only)

**When**: Truly trivial changes where failure doesn't matter

**WARNING**: sed silently does nothing if pattern doesn't match. Only use when:
- The change is cosmetic/optional
- Build will fail obviously for other reasons if source changed
- You've verified the pattern matches

**Example** (acceptable):
```yaml
# Cosmetic version string - if it fails, no harm done
prep_commands:
  - "sed -i 's/UNRELEASED/IRIX/' version.txt"
```

**Example** (NOT acceptable - use safepatch instead):
```yaml
# DANGEROUS - if this silently fails, broken libraries get built
prep_commands:
  - "sed -i 's/build_libtool_libs=no/build_libtool_libs=yes/' libtool"
```

---

## Summary Table

| Change Type | Method | Fails Loudly? |
|-------------|--------|---------------|
| Source code (C, headers, Makefile) | `.patch` file | Yes (patch fails if context wrong) |
| Libtool fixes | `fix-libtool-irix.sh` | Yes (uses safepatch) |
| Build-time config changes | `safepatch` | Yes |
| Spec macro changes | `spec_replacements` in YAML | N/A (mogrix handles) |
| Trivial/cosmetic | `sed` | **NO** - use only when safe |

---

## Common Mistakes

### 1. Creating patches with wrong context
**Problem**: Patch created manually with guessed line numbers
**Solution**: Always generate from actual extracted source tarball

### 2. Using sed for critical changes
**Problem**: `sed -i 's/foo/bar/' file` - silently does nothing if "foo" not found
**Solution**: Use `safepatch --old 'foo' --new 'bar' file`

### 3. Forgetting MOGRIX_ROOT for helper scripts
**Problem**: `fix-libtool-irix.sh` not found during rpmbuild
**Solution**: Use `$MOGRIX_ROOT/tools/fix-libtool-irix.sh` and set MOGRIX_ROOT when running rpmbuild

### 4. Regex surprises in sed
**Problem**: `sed 's/func()/func_new()/'` - parentheses are regex metacharacters
**Solution**: Use safepatch which treats patterns as literal strings
