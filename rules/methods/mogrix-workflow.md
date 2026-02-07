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
| `~/mogrix_inputs/SRPMS/` | Original Fedora 40 SRPMs (fetched, read-only inputs) |
| `~/mogrix_outputs/SRPMS/` | Converted SRPMs (mogrix convert output) |
| `~/mogrix_outputs/RPMS/` | Built RPMs (known-good outputs) |
| `~/rpmbuild/` | Ephemeral rpmbuild workspace (can delete anytime) |
| `/opt/sgug-staging/` | Cross-compilation staging (setup-cross + staged packages) |

**Key principles:**
- **Never store build artifacts in the mogrix repo** - keep it code-only
- **Original SRPMs are inputs** - fetched to `~/mogrix_inputs/SRPMS/`
- **Converted SRPMs are intermediate** - regenerate from originals + rules
- **Built RPMs are outputs** - copied to `~/mogrix_outputs/RPMS/`
- **rpmbuild is ephemeral** - workspace only, nothing of value stored there

---

## Standard Workflow


### 1. Fetch SRPM (if not already present)
```bash
# Defaults to ~/mogrix_inputs/SRPMS/
uv run mogrix fetch popt -y
```

### 2. Convert SRPM
```bash
# From an SRPM (preferred - extracts, converts, repackages)
# Defaults to ~/mogrix_outputs/SRPMS/<pkg>-converted/
uv run mogrix convert ~/mogrix_inputs/SRPMS/popt-1.19-6.fc40.src.rpm
```

### 3. Build for IRIX (cross-compile)
```bash
# RPMs copied to ~/mogrix_outputs/RPMS/ automatically
uv run mogrix build ~/mogrix_outputs/SRPMS/popt-1.19-6.fc40.src-converted/popt-1.19-6.src.rpm --cross
```

The `--cross` flag automatically:
- Uses `/opt/sgug-staging/rpmmacros.irix`
- Sets `--target=mips-sgi-irix`
- Passes `--nodeps`

### 4. Stage for dependent builds
```bash
uv run mogrix stage ~/mogrix_outputs/RPMS/popt*.rpm
```

### 5. Install on IRIX
```bash
scp ~/mogrix_outputs/RPMS/popt*.rpm root@192.168.0.81:/opt/chroot/tmp/
# On IRIX: rpm -Uvh /tmp/popt*.rpm
```

---

## Check Package Dependencies BEFORE Building

Before building a new package, check what it will require at install time:

```bash
# After converting, check the spec for Requires:
grep -E "^Requires:" ~/mogrix_outputs/SRPMS/<pkg>-converted/<pkg>.spec

# Or check the built RPM:
rpm -qp --requires ~/mogrix_outputs/RPMS/<pkg>-*.rpm
```

**Important considerations:**

1. **Subpackages have dependencies** - e.g., `pkgconf-pkg-config` requires `pkgconf-m4`
2. **noarch packages** - Some subpackages (like `-m4`, `-doc`) are `noarch`, not `mips`
3. **Build order matters** - If package B requires package A, build and stage A first

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
| `python -m mogrix` | `uv run mogrix` |
| `python3 -m mogrix.convert` | `uv run mogrix convert` |
| `python3 -m mogrix.cli` | `uv run mogrix` |
| Manual rpmbuild invocation | `uv run mogrix build --cross` |
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
