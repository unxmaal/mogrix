# Mogrix Workflow

> **For AI agents**: READ THIS FIRST before running any build commands.
> This documents the correct way to invoke mogrix.

---

## CRITICAL: Correct Invocation

**ALWAYS run mogrix via 'uv run':**
```bash
uv run mogrix <command>
```


**NEVER use:**
- `python -m mogrix` - wrong (not a runnable module)
- `python3 -m mogrix` - wrong
- `python3 -m mogrix.convert` - wrong (submodules are not entry points)
- `python3 -m mogrix.cli` - wrong

---

## Directory Conventions

| Directory | Purpose |
|-----------|---------|
| `~/rpmbuild/SRPMS/fc40/` | Original Fedora 40 SRPMs (persistent inputs) |
| `/tmp/mogrix-converted/<pkg>/` | Conversion output (ephemeral, regenerate anytime) |
| `~/rpmbuild/RPMS/mips/` | Built MIPS packages (rpmbuild output) |
| `~/rpmbuild/RPMS/noarch/` | Built noarch packages |
| `/tmp/mogrix-repo/` | Distribution repository (copy of what ships) |

**Key principles:**
- **Never store build artifacts in the mogrix repo** - keep it code-only
- **Original SRPMs are inputs** - persistent, reusable
- **Converted output is ephemeral** - regenerate from originals + rules
- **Repo is for distribution** - copy built RPMs here, run createrepo_c

---

## Standard Workflow


### 1. Fetch SRPM (if not already present)
```bash
# Fetch to standard location
uv run mogrix fetch popt -o ~/rpmbuild/SRPMS/fc40/
```

### 2. Convert SRPM
```bash
# From an SRPM (preferred - extracts, converts, repackages)
uv run mogrix convert ~/rpmbuild/SRPMS/fc40/popt-1.19-6.fc40.src.rpm -o /tmp/mogrix-converted/popt/

# From a spec file (just transforms the spec)
uv run mogrix convert path/to/package.spec -o /tmp/output-dir/
```

### 3. Build for IRIX (cross-compile)
```bash
# Use --cross flag - handles rpmbuild invocation correctly
uv run mogrix build /tmp/mogrix-converted/popt/popt-1.19-6.src.rpm --cross
```

The `--cross` flag automatically:
- Uses `/opt/sgug-staging/rpmmacros.irix`
- Sets `--target=mips-sgi-irix`
- Passes `--nodeps`

### 4. Stage for dependent builds
```bash
uv run mogrix stage ~/rpmbuild/RPMS/mips/popt*.rpm
```

### 5. Copy to repo and update metadata
```bash
cp ~/rpmbuild/RPMS/mips/popt*.rpm /tmp/mogrix-repo/
cp ~/rpmbuild/RPMS/noarch/popt*.rpm /tmp/mogrix-repo/ 2>/dev/null || true
createrepo_c --update /tmp/mogrix-repo/
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
mogrix build converted.src.rpm --cross
```

---

## Common Mistakes to Avoid

| Mistake | Correct |
|---------|---------|
| `python -m mogrix` | `source .venv/bin/activate && mogrix` |
| `python3 -m mogrix.convert` | `mogrix convert` (after activating venv) |
| `python3 -m mogrix.cli` | `mogrix` (after activating venv) |
| Manual rpmbuild invocation | `mogrix build --cross` |
| Forgetting `--cross` flag | Always use `--cross` for IRIX |
| Wrong output paths | Use `-o` to specify output directory |

---

## Checking Available Commands

```bash
uv run mogrix --help           # List all commands
uv run mogrix convert --help   # Convert options
uv run mogrix build --help     # Build options
uv run mogrix analyze <spec>   # See what rules apply
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
