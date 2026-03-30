# Hand-Written Rust Specs for IRIX Cross-Compilation

## Why Hand-Written Specs

Rust packages (ripgrep, termusic, ncspot) bypass mogrix's SRPM conversion pipeline
entirely. The standard pipeline assumes `%prep`/`%build` follow autoconf or cmake
patterns. Rust packages have none of that — Cargo manages the entire build including
dependency fetching, crate selection, and linking.

Hand-written specs let us control exactly what happens: which cargo flags, which
env vars, which compat objects get linked, and in what order. The SRPM conversion
engine doesn't have a `build_system: cargo` mode yet (see "Long-Term Intent" below).

**Reference specs**: `/home/edodd/rpmbuild/SPECS/ripgrep.spec` (minimal, no C deps),
`termusic.spec` (complex: OpenSSL, SQLite, protobuf, Cargo.toml surgery),
`ncspot.spec` (OpenSSL + SDL2).

---

## Required %prep Boilerplate

Every Rust IRIX spec needs this sequence in `%prep`, in this order:

```spec
%prep
%autosetup -n %{name}-%{version}

export PATH="$HOME/.local/bin:$HOME/.cargo/bin:$MOGRIX_ROOT/cross/bin:$PATH"

# 1. Copy target spec and write .cargo/config.toml
cp "$MOGRIX_ROOT/cross/targets/mips-sgi-irix6.5.json" .
mkdir -p .cargo
cat > .cargo/config.toml << 'CARGO_CFG'
[build]
target = "mips-sgi-irix6.5.json"
rustflags = [
    "--cfg", "mio_unsupported_force_poll_poll",
    "--cfg", "mio_unsupported_force_waker_pipe",
]

[unstable]
build-std = ["std", "panic_abort"]
json-target-spec = true
CARGO_CFG

# 2. Fetch crates (clean registry to ensure pristine sources before patching)
rm -rf $HOME/.cargo/registry/src/index.crates.io-*/
cargo +nightly fetch 2>/dev/null || true

# 3. Patch Rust sysroot for IRIX
bash "$MOGRIX_ROOT/scripts/patch-rust-sysroot.sh"

# 4. Patch crates for IRIX (applies rules/crates/*.yaml to registry)
uv run --directory "$MOGRIX_ROOT" mogrix patch-crates

# 5. Build compat library
mkdir -p compat
%{__cc} -c "$MOGRIX_ROOT/compat/rust/rust_compat.c" -o compat/rust_compat.o \
  -I$MOGRIX_STAGING/include \
  -I"$MOGRIX_ROOT/compat/include"
%{__cc} -c "$MOGRIX_ROOT/compat/rust/errno_location.c" -o compat/errno_location.o \
  -I$MOGRIX_STAGING/include
%{__cc} -c "$MOGRIX_ROOT/compat/dicl/openat-compat.c" -o compat/openat-compat.o \
  -I"$MOGRIX_ROOT/compat/include" -I$MOGRIX_STAGING/include
%{__cc} -c "$MOGRIX_ROOT/compat/string/strnlen.c" -o compat/strnlen.o \
  -I"$MOGRIX_ROOT/compat/include" -I$MOGRIX_STAGING/include
%{__cc} -c "$MOGRIX_ROOT/compat/stdlib/setenv.c" -o compat/setenv.o \
  -I"$MOGRIX_ROOT/compat/include" -I$MOGRIX_STAGING/include
%{__cc} -c "$MOGRIX_ROOT/compat/error/strerror_r.c" -o compat/strerror_r.o \
  -I"$MOGRIX_ROOT/compat/include" -I$MOGRIX_STAGING/include
%{__ar} rcs compat/librust_irix_compat.a compat/*.o
```

**Why clean the registry before fetch**: `mogrix patch-crates` mutates crate source
files in `~/.cargo/registry/src/`. A dirty registry from a previous build may have
partially-applied patches. Always wipe and re-fetch.

**Why sysroot before crates**: The sysroot patches touch `std` source in the rustup
toolchain directory. They must be applied before cargo tries to compile `std`
(via `-Zbuild-std`). The crate patches are independent but should come after fetch
so the registry is populated.

### Cargo.toml Surgery (when needed)

Some packages need `Cargo.toml` edits before `cargo fetch`. Do this before step 2:

- Switch `rustls` → `native-tls` (ring crate has no IRIX backend)
- Remove `use_zbus` from souvlaki (no D-Bus on IRIX)
- Switch `rusqlite` from `bundled` → system sqlite (cc-crate amalgamation ABI issue)
- After edits that change dep versions, `rm Cargo.lock && cargo +nightly generate-lockfile`

### Patched Crate Path Injection

If `mogrix patch-crates` modifies a crate's `Cargo.toml` (e.g., getrandom adds a
libc dependency), the cargo registry index won't know about it. Fix by appending
`[patch.crates-io]` entries pointing to the patched registry paths. See
`termusic.spec` for the Python snippet that auto-detects and injects these.

---

## Required %build Environment Variables

Always set:

```spec
%build
export PATH="$HOME/.local/bin:$HOME/.cargo/bin:$MOGRIX_ROOT/cross/bin:$PATH"
rm -rf target/   # Force std recompile to pick up sysroot patches
export RUSTFLAGS="--cfg mio_unsupported_force_poll_poll \
  --cfg mio_unsupported_force_waker_pipe \
  -L $(pwd)/compat"
```

The `-L $(pwd)/compat` is critical — it tells the linker where to find
`librust_irix_compat.a`. The target spec's `late-link-args` references
`-lrust_irix_compat`, which resolves to this archive.

For packages with C library dependencies:

| Dependency | Variables needed |
|------------|-----------------|
| OpenSSL | `OPENSSL_DIR`, `OPENSSL_LIB_DIR`, `OPENSSL_INCLUDE_DIR`, `OPENSSL_NO_VENDOR=1` |
| SQLite (system) | `SQLITE3_LIB_DIR`, `SQLITE3_INCLUDE_DIR` |
| Protobuf | `PROTOC` (absolute path to host protoc binary) |
| SDL2 link | `-lSDL2` appended to `RUSTFLAGS` |
| cc-crate C deps | `CC_mips_sgi_irix6_5="$MOGRIX_ROOT/cross/bin/irix-cc"`, `AR_mips_sgi_irix6_5=ar` |

All sysroot paths use `$MOGRIX_STAGING/{lib32,include}`.

### The cargo invocation

Minimal form:

```spec
cargo +nightly build \
  -Zbuild-std=std,panic_abort \
  --target mips-sgi-irix6.5.json \
  --release
```

With feature flags (package-specific):

```spec
cargo +nightly build \
  --no-default-features \
  --features "sdl_backend,crossterm_backend" \
  -Zbuild-std=std,panic_abort \
  --target mips-sgi-irix6.5.json \
  --release
```

`--no-default-features` is common for Rust apps that have Linux-specific defaults
(systemd, dbus, etc.) that won't compile on IRIX.

---

## %install and %files

Rust produces a single binary (or a few). No shared libraries.

```spec
%install
install -Dpm 0755 target/mips-sgi-irix6.5/release/%{name} %{buildroot}%{_bindir}/%{name}

%files
%license LICENSE
%{_bindir}/%{name}
```

The IRIX path macros block from the top of mogrix-converted specs is NOT required for
hand-written specs — mogrix bundles these apps with their own prefix. If you want
`%{_bindir}` to expand correctly without the macro block, either add the macros block
or use literal paths.

---

## Testing

Rust binaries are N32 MIPS binaries. Test via the standard mogrix test path:

```
# Copy to IRIX chroot and test
irix_copy_to local_path=target/mips-sgi-irix6.5/release/rg remote_path=/opt/mogrix/bin/rg

# Run (via MCP):
irix_exec "rg --version"
```

For bundled apps (ncspot, termusic), use `test_bundle` after bundling:

```
uv run mogrix bundle <pkgname>
test_bundle bundle=/path/to/<pkgname>.run
```

For crash investigation use `irix-diag.sh`, not raw par_trace. See
`rules/methods/irix-testing.md`.

---

## When Builds Fail

1. **MCP first**: `report_error` with the cargo error text before doing anything.
2. **Crate compile error**: Check `rules/crates/` — a rule probably exists.
   If not, add `rules/crates/<crate>.yaml` with `text_replacements`.
3. **Missing libc function**: Add to `compat/rust/rust_compat.c`, rebuild compat lib.
4. **Linker error (undefined symbol)**: Check if it's a libc function called from
   a shared lib — `inject_compat_functions` won't help there; it must go in
   `libmogrix_compat.so`. See `rules/methods/compat-functions.md`.
5. **Wrong count from `mogrix patch-crates`**: The crate version changed. Update the
   YAML rule in `rules/crates/<crate>.yaml`.
6. `add_rule` immediately after each confirmed fix.

---

## Long-Term Intent

Hand-written specs are the current workflow because the Rust port is young and each
package needs manual surgery (feature flag selection, Cargo.toml patching, IRIX-specific
sed fixes). The intent is for `crate_patcher.py` (`mogrix/crate_patcher.py`) to feed
into a future `mogrix convert-rust` command that auto-generates specs from upstream
Rust package sources. When that command exists, the boilerplate in this doc becomes
the template it emits.

Until then: copy the closest existing spec, adapt the cargo invocation and deps, and
follow the `%prep` sequence above exactly.
