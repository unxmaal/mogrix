# Mogrix Workflow

> **For AI agents**: READ THIS FIRST before running any build commands.
> This documents the correct way to invoke mogrix.

---

## CRITICAL: Correct Invocation

**ALWAYS use the venv binary:**
```bash
.venv/bin/mogrix <command>
```

**NEVER use:**
- `python -m mogrix` - wrong
- `python3 -m mogrix` - wrong
- `python3 -m mogrix.cli` - wrong
- `mogrix` (without venv path) - may not find correct Python

---

## Standard Workflow

### 1. Convert a spec or SRPM
```bash
# From an SRPM (preferred - extracts, converts, repackages)
.venv/bin/mogrix convert popt-1.19-6.fc40.src.rpm -o /tmp/popt-converted/

# From a spec file (just transforms the spec)
.venv/bin/mogrix convert path/to/package.spec -o /tmp/output-dir/
```

### 2. Build for IRIX (cross-compile)
```bash
# Use --cross flag - handles rpmbuild invocation correctly
.venv/bin/mogrix build /tmp/popt-converted/popt-1.19-6.src.rpm --cross
```

The `--cross` flag automatically:
- Uses `/opt/sgug-staging/rpmmacros.irix`
- Sets `--target=mips-sgi-irix`
- Passes `--nodeps`

### 3. Stage for dependent builds
```bash
.venv/bin/mogrix stage ~/rpmbuild/RPMS/mips/popt*.rpm
```

---

## Check Package Dependencies BEFORE Building

Before building a new package, check what it will require at install time:

```bash
# After converting, check the spec for Requires:
grep -E "^Requires:" /tmp/converted/package.spec

# Or check the built RPM:
rpm -qp --requires ~/rpmbuild/RPMS/mips/package-*.rpm
```

**Important considerations:**

1. **Subpackages have dependencies** - e.g., `pkgconf-pkg-config` requires `pkgconf-m4`
2. **noarch packages** - Some subpackages (like `-m4`, `-doc`) are `noarch`, not `mips`
3. **Build order matters** - If package B requires package A, build and stage A first

When copying to a repo, include BOTH architectures:
```bash
cp ~/rpmbuild/RPMS/mips/*.rpm /tmp/mogrix-repo/
cp ~/rpmbuild/RPMS/noarch/*.rpm /tmp/mogrix-repo/
createrepo_c --simple-md-filenames /tmp/mogrix-repo/
```

---

## DO NOT manually run rpmbuild

**Wrong approach** (what I kept doing):
```bash
# DON'T DO THIS - error prone, missing flags
rpmbuild --macros="..." --nodeps --target=mips-sgi-irix -ba spec.spec
```

**Correct approach:**
```bash
# DO THIS - mogrix handles all the flags
.venv/bin/mogrix build converted.src.rpm --cross
```

---

## Common Mistakes to Avoid

| Mistake | Correct |
|---------|---------|
| `python -m mogrix` | `.venv/bin/mogrix` |
| `python3 -m mogrix.cli` | `.venv/bin/mogrix` |
| Manual rpmbuild invocation | `.venv/bin/mogrix build --cross` |
| Forgetting `--cross` flag | Always use `--cross` for IRIX |
| Wrong output paths | Use `-o` to specify output directory |

---

## Checking Available Commands

```bash
.venv/bin/mogrix --help           # List all commands
.venv/bin/mogrix convert --help   # Convert options
.venv/bin/mogrix build --help     # Build options
.venv/bin/mogrix analyze <spec>   # See what rules apply
```

---

## Environment Setup

If mogrix fails with import errors, ensure venv is set up:
```bash
cd /home/edodd/projects/github/unxmaal/mogrix
python3 -m venv .venv
.venv/bin/pip install -e .
```

---

## When Direct rpmbuild IS Needed

Only use direct rpmbuild when:
1. Debugging a specific build issue
2. Testing spec changes without full reconversion
3. The mogrix build command has a bug

In those cases, the correct incantation is:
```bash
rpmbuild \
  --macros="/usr/lib/rpm/macros:/opt/sgug-staging/rpmmacros.irix" \
  --define '_disable_source_fetch 1' \
  --nodeps \
  --target=mips-sgi-irix \
  -ba /path/to/spec.spec
```

Note the required flags:
- `--macros=` with BOTH system macros AND irix macros
- `--target=mips-sgi-irix` for correct arch
- `--nodeps` since cross-compile deps aren't installed
- `--define '_disable_source_fetch 1'` to use local sources
