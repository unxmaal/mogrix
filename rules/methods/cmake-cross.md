# CMake Cross-Compilation Method

> **For AI agents**: READ THIS for packages using CMake.
> rpm, libsolv, and tdnf all use CMake.

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
```

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
