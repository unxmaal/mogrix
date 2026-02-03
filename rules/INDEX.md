# Mogrix Rules Index

Quick lookup table for rules and methods. **Read the linked files for details.**

---

## STOP - Before Making Significant Changes

**If you're stuck, frustrated, or about to make manual changes to already-built packages:**

### 1. Check SGUG-RSE first
```bash
ls /home/edodd/projects/github/sgug-rse/packages/<package>/
cat /home/edodd/projects/github/sgug-rse/packages/<package>/*.sgifixes.patch
```
SGUG-RSE solved many IRIX problems already. Their patches are battle-tested.

### 2. Check git history
```bash
git log --all --oneline --grep="<keyword>"
git log --all --oneline -- '**/filename'
git show <commit>
```
You may have already solved this problem in a previous session.

### 3. Check HANDOFF.md
The "What Failed" sections document approaches that don't work. Don't repeat them.

### 4. Check rules/methods/
There may be a method document covering your exact problem:
- Linker issues? → [linker-selection.md](methods/linker-selection.md) (CRITICAL)
- Missing functions? → [compat-functions.md](methods/compat-functions.md)
- Build failures? → [irix-quirks.md](methods/irix-quirks.md)

### 5. Read lld-fixes/README.md
If anything involves linking or relocations, the answer is probably LLD 18 with patches.

### 6. Clean build environment before rebuilds
```bash
rm -rf ~/rpmbuild/BUILD/*
rm -rf ~/rpmbuild/BUILDROOT/*
```
Builds are **tainted** if done without cleaning first. Old object files, patched sources,
or stale configs from previous attempts can silently pollute the new build. If debugging
a build issue, ALWAYS clean before rebuilding.

**DO NOT** start experimenting with linker changes, large sed replacements, or rebuilding
multiple packages without first checking these resources AND cleaning the build environment.

---

## Methods

| Topic | File |
|-------|------|
| **Mogrix workflow** | [methods/mogrix-workflow.md](methods/mogrix-workflow.md) |
| **Linker selection** | [methods/linker-selection.md](methods/linker-selection.md) |
| Build order | [methods/build-order.md](methods/build-order.md) |
| Patch creation | [methods/patch-creation.md](methods/patch-creation.md) |
| Text replacement | [methods/text-replacement.md](methods/text-replacement.md) |
| Compat functions | [methods/compat-functions.md](methods/compat-functions.md) |
| IRIX testing | [methods/irix-testing.md](methods/irix-testing.md) |
| IRIX quirks | [methods/irix-quirks.md](methods/irix-quirks.md) |
| Autoconf packages | [methods/autoconf-cross.md](methods/autoconf-cross.md) |
| CMake packages | [methods/cmake-cross.md](methods/cmake-cross.md) |

## Package Rules

All package rules are in `rules/packages/*.yaml`.

### Bootstrap Chain (Working)

| Package | Build | File |
|---------|-------|------|
| zlib-ng | cmake | [zlib-ng.yaml](packages/zlib-ng.yaml) |
| bzip2 | make | [bzip2.yaml](packages/bzip2.yaml) |
| popt | autoconf | [popt.yaml](packages/popt.yaml) |
| openssl | custom | [openssl.yaml](packages/openssl.yaml) |
| libxml2 | autoconf | [libxml2.yaml](packages/libxml2.yaml) |
| curl | autoconf | [curl.yaml](packages/curl.yaml) |
| xz | autoconf | [xz.yaml](packages/xz.yaml) |
| lua | make | [lua.yaml](packages/lua.yaml) |
| file | autoconf | [file.yaml](packages/file.yaml) |
| sqlite | autoconf | [sqlite.yaml](packages/sqlite.yaml) |
| rpm | cmake | [rpm.yaml](packages/rpm.yaml) |
| libsolv | cmake | [libsolv.yaml](packages/libsolv.yaml) |
| tdnf | cmake | [tdnf.yaml](packages/tdnf.yaml) |

### Utilities

| Package | Build | File |
|---------|-------|------|
| tree | make | [tree-pkg.yaml](packages/tree-pkg.yaml) |
| jq | autoconf | [jq.yaml](packages/jq.yaml) |
| pkg-config | autoconf | [pkg-config.yaml](packages/pkg-config.yaml) |

## Other Files

| File | Purpose |
|------|---------|
| [generic.yaml](generic.yaml) | Rules applied to ALL packages |
| [../compat/catalog.yaml](../compat/catalog.yaml) | Compat function definitions |
| [../HANDOFF.md](../HANDOFF.md) | Current session state |
| [../Claude.md](../Claude.md) | Instructions for Claude |
