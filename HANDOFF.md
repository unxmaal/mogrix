# Mogrix Cross-Compilation Handoff

**Last Updated**: 2026-01-25 15:25
**Status**: Ready to build curl

---

## Goal

Validate mogrix workflow by building tdnf dependency chain one package at a time.

**Critical rule**: Use FC40 exclusively. Never mix Fedora releases - the whole point is that Fedora solved dependencies for us.

---

## Build Order (FC40)

| # | Package | Version | Status | Notes |
|---|---------|---------|--------|-------|
| 1 | **zlib-ng** | 2.1.6 | **COMPLETE** | FC40 replaced zlib with zlib-ng; cmake-based |
| 2 | bzip2 | 1.0.8 | **COMPLETE** | Built first try |
| 3 | popt | 1.19 | **COMPLETE** | Required stpcpy compat, libtool fixes |
| 4 | openssl | 3.2.1 | **COMPLETE** | Built first try |
| 5 | libxml2 | 2.12.5 | **COMPLETE** | Required libtool fixes |
| 6 | curl | 8.6.0 | PENDING | |
| 7 | xz | 5.4.6 | PENDING | |
| 8 | lua | 5.4.6 | PENDING | |
| 9 | file | 5.45 | PENDING | |
| 10 | rpm | 4.19.1.1 | PENDING | Known blocker: spawn.h |
| 11 | libsolv | 0.7.28 | PENDING | |
| 12 | **tdnf** | **3.5.14** | PENDING | **TARGET** |

---

## Completed: zlib-ng

### Root Cause
zlib-ng's `zbuild.h` defines `_POSIX_SOURCE` and `_POSIX_C_SOURCE` which breaks IRIX's feature macro chain. This makes `_NO_POSIX` false, which makes `_SGIAPI` false, which prevents snprintf/vsnprintf from being declared.

### Solution
Add `-D_XOPEN_SOURCE=600` to CMAKE_C_FLAGS. This makes `_XOPEN5` true, which exposes snprintf/vsnprintf regardless of POSIX macros.

Full flag set needed:
```
-DCMAKE_C_FLAGS="-I/usr/sgug/include/mogrix-compat/generic -D_SGI_SOURCE -D__EXTENSIONS__ -D_XOPEN_SOURCE=600"
```

### Additional Fixes Required
1. **llabs declaration**: IRIX libc doesn't have `llabs()`. Added extern declaration in compat/stdlib.h that Clang resolves via builtin.
2. **Versioned library symlinks**: CMake doesn't create versioned .so files for IRIX. Added post-install commands to create `libz-ng.so.2.1.6`, `libz-ng.so.2` symlinks, and similar for compat `libz.so.1`, `libz.so.1.3.0.zlib-ng`.

### Files Changed
- `rules/packages/zlib-ng.yaml` - complete cmake cross-compilation setup
- `compat/include/mogrix-compat/generic/stdlib.h` - llabs declaration
- `compat/include/mogrix-compat/generic/stdint.h` - UINT64_C etc.

---

## Fetch Command Update

`mogrix fetch` now defaults to FC40 and shows the full archive URL:
```
Fetching SRPMs from Fedora 40 archives
https://archives.fedoraproject.org/pub/archive/fedora/linux/releases/40/Everything/source/tree/Packages/
```

---

## Cleanup

Use `./cleanup.sh` for a complete reset. The script:
1. Cleans staging lib32/ and include/
2. Restores compat headers
3. Builds runtime libraries (libsoft_float_stubs.a, libatomic.a)
4. Cleans rpmbuild directories

---

## Per-Package Workflow

```bash
source .venv/bin/activate
mogrix fetch <package>
mogrix convert ~/rpmbuild/SRPMS/<package>-*.src.rpm
mogrix build ~/rpmbuild/SRPMS/<package>-*-converted/<package>-*.src.rpm --cross
mogrix stage ~/rpmbuild/RPMS/mips/<package>*.rpm
```

---

## Key Discoveries This Session

### IRIX Feature Macro Chain
IRIX uses a complex macro chain in `<standards.h>`:
- `_NO_POSIX` = true only if neither `_POSIX_SOURCE` nor `_POSIX_C_SOURCE` defined
- `_SGIAPI` = `(_SGI_SOURCE && _NO_POSIX && _NO_XOPEN4 && _NO_XOPEN5) || ...`
- `_XOPEN5` = `(_XOPEN_SOURCE >= 500) || ...`
- `_NO_ANSIMODE` = `__EXTENSIONS__ || ...`

Functions like snprintf/vsnprintf require:
```c
#if defined(__c99) || ((_XOPEN5 || _SGIAPI) && _NO_ANSIMODE)
```

**Key insight**: Many packages define POSIX macros for compatibility, breaking `_SGIAPI`. Use `_XOPEN_SOURCE=600` to get `_XOPEN5` instead.

### CMake Packages Need Macro Expansion
Fedora uses `%cmake`, `%cmake_build`, `%cmake_install` macros that aren't available in our build environment. Must replace with explicit cmake commands in spec_replacements.

### CMake Doesn't Handle IRIX Library Versioning
CMake creates unversioned .so files for unknown platforms. Need manual symlink creation for proper versioned libraries.

### Libtool Refuses Shared Libraries for IRIX
Libtool detects IRIX as unknown and sets `build_libtool_libs=no`. Need post-configure sed commands:
```bash
sed -i 's/build_libtool_libs=no/build_libtool_libs=yes/g' libtool
sed -i 's/deplibs_check_method="unknown"/deplibs_check_method="pass_all"/g' libtool
sed -i 's/^version_type=none$/version_type=linux/g' libtool
sed -i 's/^soname_spec=""$/soname_spec="\\$libname\\${shared_ext}\\$major"/g' libtool
sed -i 's/^library_names_spec=""$/library_names_spec="\\$libname\\${shared_ext}\\$versuffix \\$libname\\${shared_ext}\\$major \\$libname\\${shared_ext}"/g' libtool
```

### New Compat Function: stpcpy
Added `stpcpy()` to libcompat.a - copies string and returns pointer to end.
- Implementation: `compat/runtime/stpcpy.c`
- Declaration: `compat/include/mogrix-compat/generic/string.h`
- Link with: `-lcompat` (added to rpmmacros.irix LDFLAGS)

### Clang Builtins Fill Gaps
IRIX libc is missing some C99 functions (llabs), but Clang provides them as builtins. Just need the declaration; Clang handles the implementation.

### New Compat Functions Added
| Function | Header | Purpose |
|----------|--------|---------|
| `llabs()` | stdlib.h | long long absolute value (C99) - extern decl for Clang builtin |
| `UINT64_C()` etc | stdint.h | Integer constant macros (C99) |

---

## Environment

| Purpose | Path |
|---------|------|
| Mogrix project | `/home/edodd/projects/github/unxmaal/mogrix/` |
| Cross toolchain | `/opt/cross/bin/` |
| Staging area | `/opt/sgug-staging/usr/sgug/` |
| IRIX sysroot | `/opt/irix-sysroot/` |
| Python venv | `.venv/bin/activate` |

---

## Next Steps

1. Build bzip2
2. Continue through dependency chain
