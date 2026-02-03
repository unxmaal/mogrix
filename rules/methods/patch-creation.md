# Patch Creation

Mogrix automatically creates a `.origfedora` copy of extracted source during `%prep`.
Use this to create patches instead of using sed commands in YAML.

## Basic Workflow

```bash
# 1. Convert the package
mogrix convert ~/rpmbuild/SRPMS/fc40/foo-1.0.src.rpm -o /tmp/foo/

# 2. Run prep phase only (extracts source + creates .origfedora copy)
rpmbuild -bp /tmp/foo/foo.spec --nodeps

# 3. Make your changes
cd ~/rpmbuild/BUILD/foo-1.0
vim src/file.c

# 4. Create patch
mkpatch diff .
# -> patches/packages/foo/foo.irixfixes.patch

# 5. Add to YAML
# add_patch:
#   - foo.irixfixes.patch
```

## When to Use Patches vs sed

| Change Type | Approach |
|-------------|----------|
| Source code fixes (headers, functions) | **Patch file** |
| CMake subdirectory disables | sed (trivial comment-out) |
| Config file path edits | sed in install_cleanup |
| Spec macro changes | spec_replacements |

## Multiple Focused Patches

Create separate patches for different concerns:

```bash
# Reset and create first patch
rm -rf ~/rpmbuild/BUILD/foo-1.0
rpmbuild -bp spec.spec --nodeps
cd ~/rpmbuild/BUILD/foo-1.0
# make header changes only
mkpatch diff .
mv patches/packages/foo/foo.irixfixes.patch patches/packages/foo/foo-headers.patch

# Reset and create second patch
rm -rf ~/rpmbuild/BUILD/foo-1.0
rpmbuild -bp spec.spec --nodeps
cd ~/rpmbuild/BUILD/foo-1.0
# make different changes
mkpatch diff .
mv patches/packages/foo/foo.irixfixes.patch patches/packages/foo/foo-statfs.patch
```

## Tool Reference

| Tool | Purpose |
|------|---------|
| `tools/mkpatch diff [dir]` | Generate patch from changes vs .origfedora |
| `tools/mkpatch extract <srpm>` | Standalone extraction (alternative to rpmbuild -bp) |
| `tools/mkpatch help` | Full usage documentation |
