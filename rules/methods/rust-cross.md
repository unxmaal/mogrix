# Rust Cross-Compilation for IRIX

How to build Rust applications for IRIX 6.5 (MIPS N32) using mogrix.

## Architecture Overview

Rust has no upstream IRIX support. We make it work with three layers:

1. **Sysroot patches** — modify Rust std source to handle IRIX platform quirks
2. **Crate rules** — patch third-party crates in the cargo registry via `mogrix patch-crates`
3. **Compat library** — C stubs for POSIX functions missing from IRIX libc

All IRIX-specific changes live in mogrix. No maintained forks.

## Target Spec

`cross/targets/mips-sgi-irix6.5.json` defines the Rust target:

- **ABI**: N32 (32-bit pointers, 64-bit registers)
- **Linker**: `irix-cc` (mogrix cross-compiler wrapper)
- **Panic**: abort (no unwinding support)
- **TLS**: disabled (`has-thread-local: false`)
- **PIE/RELRO**: disabled (IRIX rld doesn't support them)
- **Late-link**: `-lrust_irix_compat -lgcc_s` (compat library always linked)

The target spec uses relative names (`irix-cc`, not absolute paths). The spec's
`%prep` puts `$MOGRIX_ROOT/cross/bin` on PATH.

## Sysroot Patches (`scripts/patch-rust-sysroot.sh`)

Run after `rustup update` — nightly updates overwrite our patches. The script:

1. Creates `os/irix/{mod,raw,fs}.rs` in std source
2. Patches std modules:
   - `sys/fd/unix.rs` — FIOCLEX: use fcntl(F_SETFD) instead of ioctl(FIOCLEX) on sockets
   - `sys/net/.../socket/unix.rs` — FIONBIO: use fcntl(F_SETFL) instead of ioctl(FIONBIO)
   - `sys/random/mod.rs` — use unix_legacy (/dev/urandom), NOT arc4random
   - `sys/thread/unix.rs` — set_name no-op (no pthread_setname_np)
   - `sys/fs/unix.rs` — exclude d_type (IRIX dirent has no d_type field)
   - `sys/pal/unix/os.rs` — current_exe via argv[0] (AIX-style)
   - `sys/args/unix.rs` — argc/argv storage
   - `build.rs` — add irix to known platforms
3. Includes repair blocks for corruption from previous patch runs

Use `--check` flag to verify patches without modifying files.

## Crate Patching (`mogrix patch-crates`)

Rules in `rules/crates/*.yaml` patch crates in `~/.cargo/registry/src/`.

### Key rules:

| Crate | What's patched | Why |
|-------|----------------|-----|
| `libc` | IRIX module injected via `add_source` | Struct layouts, constants, function declarations |
| `getrandom` | Backend routing, errno routing | IRIX uses /dev/urandom, `__oserror()` for errno |
| `mio` | Force poll(2), pipe waker, exclude SOCK_CLOEXEC | No epoll/kqueue/eventfd on IRIX |
| `socket2` | IovLen, exclude accept4/TCP_KEEPIDLE | Missing socket options |
| `tokio` | ucred impl | No SO_PEERCRED on IRIX |
| `rustix` | Solarish group, d_type exclusion, makedev | IRIX is SVR4-derived |
| `signal-hook-registry` | NULL siginfo handling | IRIX signals may deliver NULL siginfo |
| `generic_rust` | Add IRIX to NTO cfg gates | Catch-all for crates using NTO as fallback |

### skip_generic

Crates with complex non-standard patterns set `skip_generic: true` to avoid
false matches from the generic rule. Examples: mio, rustix, socket2, open.

### Validation

- Crate-specific rules are **strict** — pattern must match exactly
- Generic rules are **lenient** — missing patterns silently skipped
- `expected_count: 0` overrides to lenient (for patterns that exist in some versions only)

## Compat Library

`compat/rust/` provides C stubs linked into every Rust binary:

| Function | Why |
|----------|-----|
| `dirfd`, `fdopendir` | Missing from IRIX libc |
| `pthread_condattr_setclock` | IRIX only supports CLOCK_REALTIME |
| `preadv`, `pwritev` | Emulated with pread/pwrite loops |
| `dup3` | Emulated with dup2 + fcntl |
| `posix_fadvise` | No-op stub |
| `posix_fallocate` | Emulated with seek + write |
| `cfmakeraw` | Terminal raw mode setup |
| `flock` | Emulated with fcntl locks |
| `__errno_location` | Alias for `__oserror()` |

Additional compat objects from `compat/dicl/`, `compat/string/`, `compat/stdlib/`
are also linked (openat, strnlen, setenv).

## Build Pipeline

A Rust package spec does this:

```
%prep
# 1. Set up target spec and .cargo/config.toml
cp "$MOGRIX_ROOT/cross/targets/mips-sgi-irix6.5.json" .
# 2. Strip [patch.crates-io] from Cargo.toml (no forks needed)
# 3. cargo +nightly fetch
# 4. bash "$MOGRIX_ROOT/scripts/patch-rust-sysroot.sh"
# 5. uv run mogrix patch-crates
# 6. Build compat library → compat/librust_irix_compat.a

%build
RUSTFLAGS="-L $(pwd)/compat" cargo +nightly build \
  -Zbuild-std=std,panic_abort \
  --target mips-sgi-irix6.5.json --release
```

## What IRIX Lacks

| Feature | Impact | Workaround |
|---------|--------|------------|
| ELF TLS | No thread-local storage | `has-thread-local: false` in target spec |
| epoll/kqueue | No high-perf I/O mux | `--cfg mio_unsupported_force_poll_poll` |
| eventfd | No lightweight waker | `--cfg mio_unsupported_force_waker_pipe` |
| ioctl(FIOCLEX) on sockets | Returns EOPNOTSUPP | Sysroot patch: use fcntl(F_SETFD) |
| ioctl(FIONBIO) on sockets | Returns EOPNOTSUPP | Sysroot patch: use fcntl(F_SETFL) |
| arc4random | No CSPRNG in libc | Sysroot patch: use /dev/urandom |
| pthread_setname_np | No thread naming | Sysroot patch: no-op |
| dirent.d_type | Missing field | Sysroot patch: lstat fallback |
| SOCK_CLOEXEC/SOCK_NONBLOCK | Socket creation flags missing | Crate patch: fcntl after socket() |
| accept4 | Missing | Crate patch: accept + fcntl |

## IRIX libc Module (`patches/crates/libc/`)

The `unix/irix/mod.rs` file contains 1400+ lines of empirically-verified bindings:

- **Critical**: SOCK_DGRAM=1, SOCK_STREAM=2 (SWAPPED from Linux!)
- **Critical**: SOL_SOCKET=0xFFFF (not 1)
- **Critical**: time_t is i32 (32-bit, not 64-bit)
- All struct layouts verified on IRIX 6.5.30 hardware via struct_sizer.c
- ioctl constants use `c_ulong` (matches Rust libc convention)

## Cargo.toml Dep Resolution

Patching `Cargo.toml` in the registry works because:
1. `cargo fetch` resolves deps platform-independently (all platform deps included)
2. The lockfile captures getrandom→libc dep for ALL platforms
3. After patching, Cargo reads the local (patched) Cargo.toml at build time
4. libc is already in the lockfile, so the dependency is satisfied

## Adding a New Rust Package

1. Get upstream source, create a hand-written spec following ncspot.spec pattern
2. Run `cargo +nightly fetch` then `uv run mogrix patch-crates` to see if existing rules suffice
3. If build fails on a crate:
   - `report_error` MCP tool first
   - Check if the pattern matches an existing rule (usually `generic_rust.yaml` covers it)
   - If not, create `rules/crates/<crate>.yaml` with the needed text_replacements
   - Use `skip_generic: true` if the crate has non-standard cfg patterns
4. If build fails on a missing libc function: add to `compat/rust/rust_compat.c`
5. `add_rule` MCP tool immediately after fix confirmed

## File Locations

| What | Where |
|------|-------|
| Target spec | `cross/targets/mips-sgi-irix6.5.json` |
| Cargo config template | `cross/cargo/config.toml.irix-rust` |
| Sysroot patcher | `scripts/patch-rust-sysroot.sh` |
| Crate rules | `rules/crates/*.yaml` |
| libc IRIX module | `patches/crates/libc/unix/irix/mod.rs` |
| Compat C stubs | `compat/rust/rust_compat.c`, `compat/rust/errno_location.c` |
| Crate patcher engine | `mogrix/crate_patcher.py` |
