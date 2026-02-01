# CMake Cross-Compilation Method

> **For AI agents**: READ THIS for packages using CMake.
> rpm, libsolv, and tdnf all use CMake.

---

## Building CMake Packages

**IMPORTANT**: Always use `--target=mips-sgi-irix` with rpmbuild:

```bash
rpmbuild --macros="/usr/lib/rpm/macros:/opt/sgug-staging/rpmmacros.irix" \
  --define '_disable_source_fetch 1' \
  --nodeps \
  --target=mips-sgi-irix \
  -ba /path/to/spec
```

Without `--target`, RPMs will incorrectly be named `*.x86_64.rpm` instead of `*.mips.rpm`.

---

## Identifying CMake Packages

Look for:
- `CMakeLists.txt` in source root
- `%cmake` macro in spec file
- `-DCMAKE_*` flags in build

---

## CMake Cross-Compilation Boilerplate

Replace `%cmake` with full cross-compilation setup:

```yaml
spec_replacements:
  - pattern: "%cmake"
    replacement: |
      cmake -B _build -S . \
        -DCMAKE_SYSTEM_NAME=IRIX \
        -DCMAKE_SYSTEM_VERSION=6.5 \
        -DCMAKE_C_COMPILER=%{__cc} \
        -DCMAKE_CXX_COMPILER=%{__cxx} \
        -DCMAKE_AR=/opt/cross/bin/llvm-ar \
        -DCMAKE_RANLIB=/opt/cross/bin/llvm-ranlib \
        -DCMAKE_INSTALL_PREFIX=/usr/sgug \
        -DCMAKE_INSTALL_LIBDIR=lib32 \
        -DCMAKE_PREFIX_PATH=/opt/sgug-staging/usr/sgug \
        -DCMAKE_FIND_ROOT_PATH=/opt/sgug-staging/usr/sgug \
        -DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=ONLY \
        -DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=ONLY \
        -DCMAKE_FIND_ROOT_PATH_MODE_PACKAGE=ONLY
```

---

## Key Variables

| Variable | Value | Purpose |
|----------|-------|---------|
| `CMAKE_SYSTEM_NAME` | `IRIX` | Target OS |
| `CMAKE_SYSTEM_VERSION` | `6.5` | Target OS version |
| `CMAKE_C_COMPILER` | `%{__cc}` | Use mogrix cross-compiler |
| `CMAKE_INSTALL_LIBDIR` | `lib32` | SGUG uses lib32 not lib |
| `CMAKE_PREFIX_PATH` | Staging path | Find staged dependencies |
| `CMAKE_FIND_ROOT_PATH_MODE_*` | `ONLY` | Don't use host libraries |

---

## Common Package-Specific Options

### rpm

```yaml
-DENABLE_SQLITE=ON
-DENABLE_NDB=OFF
-DENABLE_BDB_RO=OFF
-DWITH_SELINUX=OFF
-DWITH_AUDIT=OFF
-DWITH_CAP=OFF
-DENABLE_PLUGINS=OFF
-DRPM_CONFIGDIR=/usr/sgug/lib32/rpm
-DSQLite3_INCLUDE_DIR=/opt/sgug-staging/usr/sgug/include
-DSQLite3_LIBRARY=/opt/sgug-staging/usr/sgug/lib32/libsqlite3.so
```

### libsolv

```yaml
-DENABLE_RPMDB=ON
-DENABLE_RPMMD=ON
-DENABLE_COMPLEX_DEPS=ON
-DENABLE_COMPS=ON
-DHAVE_FUNOPEN=1
-DWITH_LIBXML2=ON
-DENABLE_ZSTD_COMPRESSION=OFF
```

### tdnf

```yaml
-DWITH_PLUGIN_REPOGPGCHECK=OFF
-DCMAKE_SKIP_RPATH=ON
-DCMAKE_INSTALL_SYSCONFDIR=%{_sysconfdir}
-DSYSCONFDIR=%{_sysconfdir}
```

**CRITICAL**: tdnf has hardcoded `/etc` paths in `client/defines.h` and `common/config.h`.
These are NOT CMake-configurable and must be patched in source:

```bash
# Patch client/defines.h
sed -i 's|"/etc/tdnf/tdnf.conf"|"/usr/sgug/etc/tdnf/tdnf.conf"|g' client/defines.h
sed -i 's|"/etc/yum.repos.d"|"/usr/sgug/etc/yum.repos.d"|g' client/defines.h
sed -i 's|"/etc/tdnf/pluginconf.d"|"/usr/sgug/etc/tdnf/pluginconf.d"|g' client/defines.h

# Patch common/config.h (has additional VARS_DIRS)
sed -i 's|"/etc/tdnf/tdnf.conf"|"/usr/sgug/etc/tdnf/tdnf.conf"|g' common/config.h
sed -i 's|"/etc/yum.repos.d"|"/usr/sgug/etc/yum.repos.d"|g' common/config.h
sed -i 's|"/etc/tdnf/vars /etc/dnf/vars /etc/yum/vars"|"/usr/sgug/etc/tdnf/vars /usr/sgug/etc/dnf/vars /usr/sgug/etc/yum/vars"|g' common/config.h
sed -i 's|"/etc/tdnf/pluginconf.d"|"/usr/sgug/etc/tdnf/pluginconf.d"|g' common/config.h
```

After building, verify with: `strings libtdnf.so | grep "/etc"` - should show `/usr/sgug/etc` paths

---

## Handling --whole-archive

When linking static archives into shared libraries, symbols not directly referenced won't be included. Force inclusion with:

```yaml
spec_replacements:
  - pattern: "target_link_libraries(libsolv"
    replacement: |
      target_link_libraries(libsolv
        -Wl,--whole-archive
        ${COMPAT_LIB}
        -Wl,--no-whole-archive
```

---

## Minimal CMake Package Rule

```yaml
package: mypackage

rules:
  inject_compat_functions:
    - strdup
    - getline

  spec_replacements:
    - pattern: "%cmake"
      replacement: |
        cmake -B _build -S . \
          -DCMAKE_SYSTEM_NAME=IRIX \
          -DCMAKE_C_COMPILER=%{__cc} \
          -DCMAKE_INSTALL_PREFIX=/usr/sgug \
          -DCMAKE_INSTALL_LIBDIR=lib32 \
          -DCMAKE_PREFIX_PATH=/opt/sgug-staging/usr/sgug \
          -DCMAKE_FIND_ROOT_PATH=/opt/sgug-staging/usr/sgug \
          -DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=ONLY \
          -DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=ONLY \
          -DSOME_FEATURE=OFF

    # Also fix %cmake_build and %cmake_install
    - pattern: "%cmake_build"
      replacement: "cmake --build _build %{?_smp_mflags}"

    - pattern: "%cmake_install"
      replacement: "DESTDIR=%{buildroot} cmake --install _build"
```

---

## Troubleshooting

### "Could not find <Package>"

CMake can't find a dependency. Add:
```yaml
-D<Package>_INCLUDE_DIR=/opt/sgug-staging/usr/sgug/include
-D<Package>_LIBRARY=/opt/sgug-staging/usr/sgug/lib32/lib<pkg>.so
```

### "CMAKE_SIZEOF_VOID_P is empty"

Cross-compilation detection failed. Add:
```yaml
-DCMAKE_SIZEOF_VOID_P=4
```

### Missing symbols at runtime

Static archive symbols not included. Use `--whole-archive` (see above).

### Wrong library path (lib vs lib32)

Ensure:
```yaml
-DCMAKE_INSTALL_LIBDIR=lib32
```

---

## Files Reference

| Example | Complexity | Key Features |
|---------|------------|--------------|
| `rules/packages/zlib-ng.yaml` | Medium | Basic cmake cross |
| `rules/packages/rpm.yaml` | High | SQLite, many disables |
| `rules/packages/libsolv.yaml` | High | funopen, --whole-archive |
| `rules/packages/tdnf.yaml` | High | Plugin disable, arch hardcode |
