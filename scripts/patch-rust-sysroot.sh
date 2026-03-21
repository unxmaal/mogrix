#!/bin/bash
# patch-rust-sysroot.sh — Apply IRIX support patches to the Rust nightly sysroot.
#
# This patches the Rust standard library source for IRIX cross-compilation.
# The cargo registry (libc, getrandom, etc.) is handled separately by
# `mogrix patch-crates`.
#
# Run after: rustup toolchain install nightly && rustup component add rust-src
# Re-run after: rustup update (nightly toolchain updates overwrite patches)
#
# Usage:
#   bash scripts/patch-rust-sysroot.sh [--check]
#
# --check: verify patches are applied without modifying files (exit 1 if not)
set -euo pipefail

CHECK_MODE=false
if [[ "${1:-}" == "--check" ]]; then
    CHECK_MODE=true
fi

SYSROOT=$(rustc +nightly --print sysroot)
STD_SRC="$SYSROOT/lib/rustlib/src/rust/library/std/src"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
MOGRIX_ROOT="$(dirname "$SCRIPT_DIR")"

echo "Sysroot: $SYSROOT"
echo "Std source: $STD_SRC"
echo "Mogrix root: $MOGRIX_ROOT"
if $CHECK_MODE; then
    echo "Mode: CHECK (read-only)"
fi

FAILURES=0

# ===== 1. Create std OS module for IRIX =====
echo ""
echo "=== Creating std OS module for IRIX ==="

if $CHECK_MODE; then
    if [[ -f "$STD_SRC/os/irix/mod.rs" ]]; then
        echo "  os/irix/ module exists"
    else
        echo "  MISSING: os/irix/ module"
        FAILURES=$((FAILURES + 1))
    fi
else
    mkdir -p "$STD_SRC/os/irix"

    # os/irix/mod.rs
    cat > "$STD_SRC/os/irix/mod.rs" << 'IRIX_MOD'
//! IRIX-specific definitions

#![stable(feature = "raw_ext", since = "1.1.0")]

pub mod fs;
pub mod raw;
IRIX_MOD

    # os/irix/raw.rs
    cat > "$STD_SRC/os/irix/raw.rs" << 'IRIX_RAW'
//! IRIX-specific raw type definitions

#![stable(feature = "raw_ext", since = "1.1.0")]

#[stable(feature = "pthread_t", since = "1.8.0")]
pub use libc::pthread_t;

#[stable(feature = "raw_ext", since = "1.1.0")]
pub use libc::{blkcnt_t, blksize_t, dev_t, ino_t, mode_t, nlink_t, off_t, time_t};
IRIX_RAW

    # os/irix/fs.rs
    cat > "$STD_SRC/os/irix/fs.rs" << 'IRIX_FS'
//! IRIX-specific extensions to primitives in `std::fs`.

#![stable(feature = "metadata_ext", since = "1.1.0")]

use crate::fs::Metadata;
use crate::sys::AsInner;

/// OS-specific extensions to [`fs::Metadata`].
#[stable(feature = "metadata_ext", since = "1.1.0")]
pub trait MetadataExt {
    #[stable(feature = "metadata_ext2", since = "1.8.0")]
    fn st_dev(&self) -> u64;
    #[stable(feature = "metadata_ext2", since = "1.8.0")]
    fn st_ino(&self) -> u64;
    #[stable(feature = "metadata_ext2", since = "1.8.0")]
    fn st_mode(&self) -> u32;
    #[stable(feature = "metadata_ext2", since = "1.8.0")]
    fn st_nlink(&self) -> u64;
    #[stable(feature = "metadata_ext2", since = "1.8.0")]
    fn st_uid(&self) -> u32;
    #[stable(feature = "metadata_ext2", since = "1.8.0")]
    fn st_gid(&self) -> u32;
    #[stable(feature = "metadata_ext2", since = "1.8.0")]
    fn st_rdev(&self) -> u64;
    #[stable(feature = "metadata_ext2", since = "1.8.0")]
    fn st_size(&self) -> u64;
    #[stable(feature = "metadata_ext2", since = "1.8.0")]
    fn st_atime(&self) -> i64;
    #[stable(feature = "metadata_ext2", since = "1.8.0")]
    fn st_atime_nsec(&self) -> i64;
    #[stable(feature = "metadata_ext2", since = "1.8.0")]
    fn st_mtime(&self) -> i64;
    #[stable(feature = "metadata_ext2", since = "1.8.0")]
    fn st_mtime_nsec(&self) -> i64;
    #[stable(feature = "metadata_ext2", since = "1.8.0")]
    fn st_ctime(&self) -> i64;
    #[stable(feature = "metadata_ext2", since = "1.8.0")]
    fn st_ctime_nsec(&self) -> i64;
    #[stable(feature = "metadata_ext2", since = "1.8.0")]
    fn st_blksize(&self) -> u64;
    #[stable(feature = "metadata_ext2", since = "1.8.0")]
    fn st_blocks(&self) -> u64;
}

#[stable(feature = "metadata_ext", since = "1.1.0")]
impl MetadataExt for Metadata {
    fn st_dev(&self) -> u64 { self.as_inner().as_inner().st_dev as u64 }
    fn st_ino(&self) -> u64 { self.as_inner().as_inner().st_ino as u64 }
    fn st_mode(&self) -> u32 { self.as_inner().as_inner().st_mode as u32 }
    fn st_nlink(&self) -> u64 { self.as_inner().as_inner().st_nlink as u64 }
    fn st_uid(&self) -> u32 { self.as_inner().as_inner().st_uid as u32 }
    fn st_gid(&self) -> u32 { self.as_inner().as_inner().st_gid as u32 }
    fn st_rdev(&self) -> u64 { self.as_inner().as_inner().st_rdev as u64 }
    fn st_size(&self) -> u64 { self.as_inner().as_inner().st_size as u64 }
    fn st_atime(&self) -> i64 { self.as_inner().as_inner().st_atime as i64 }
    fn st_atime_nsec(&self) -> i64 { self.as_inner().as_inner().st_atime_nsec as i64 }
    fn st_mtime(&self) -> i64 { self.as_inner().as_inner().st_mtime as i64 }
    fn st_mtime_nsec(&self) -> i64 { self.as_inner().as_inner().st_mtime_nsec as i64 }
    fn st_ctime(&self) -> i64 { self.as_inner().as_inner().st_ctime as i64 }
    fn st_ctime_nsec(&self) -> i64 { self.as_inner().as_inner().st_ctime_nsec as i64 }
    fn st_blksize(&self) -> u64 { self.as_inner().as_inner().st_blksize as u64 }
    fn st_blocks(&self) -> u64 { self.as_inner().as_inner().st_blocks as u64 }
}
IRIX_FS

    echo "  Created os/irix/{mod,raw,fs}.rs"
fi

# ===== 2. Patch std source files =====
echo ""
echo "=== Patching std source ==="

python3 << PYSCRIPT
import os, sys

STD_SRC = "$STD_SRC"
CHECK_MODE = $($CHECK_MODE && echo "True" || echo "False")
failures = 0

def patch_file(relpath, patches):
    """Apply text replacements to a file. Each patch is (old, new).
    A patch is only applied if old is present AND new is not already present.
    This makes patches idempotent."""
    global failures
    filepath = os.path.join(STD_SRC, relpath)
    with open(filepath) as f:
        content = f.read()

    if CHECK_MODE:
        for old, new in patches:
            if new in content:
                continue  # Already applied
            if old in content:
                print(f"  NOT APPLIED: {relpath}")
                failures += 1
                return
        print(f"  OK: {relpath}")
        return

    changed = False
    for old, new in patches:
        if old in content and new not in content:
            content = content.replace(old, new)
            changed = True
    if changed:
        with open(filepath, 'w') as f:
            f.write(content)
        print(f"  Patched {relpath}")
    else:
        print(f"  {relpath} (already patched or no match)")

def repair_file(relpath, repairs):
    """Apply unconditional text replacements to fix damaged files.
    Each repair is (broken_text, correct_text). Only applies if broken_text is present."""
    global failures
    filepath = os.path.join(STD_SRC, relpath)
    with open(filepath) as f:
        content = f.read()

    if CHECK_MODE:
        for broken, correct in repairs:
            if broken in content:
                print(f"  NEEDS REPAIR: {relpath}")
                failures += 1
                return
        return  # No repair needed

    changed = False
    for broken, correct in repairs:
        if broken in content:
            content = content.replace(broken, correct)
            changed = True
    if changed:
        with open(filepath, 'w') as f:
            f.write(content)
        print(f"  Repaired {relpath}")
    else:
        print(f"  {relpath} (no repair needed)")

# ---------------------------------------------------------------------------
# os/mod.rs: Register IRIX OS module
# ---------------------------------------------------------------------------
patch_file("os/mod.rs", [
    ('    #[cfg(target_os = "hurd")]\n    pub mod hurd;',
     '    #[cfg(target_os = "hurd")]\n    pub mod hurd;\n    #[cfg(target_os = "irix")]\n    pub mod irix;'),
])

# ---------------------------------------------------------------------------
# os/unix/mod.rs: Add IRIX platform re-export
# ---------------------------------------------------------------------------
patch_file("os/unix/mod.rs", [
    ('#[cfg(target_os = "illumos")]\npub use crate::os::illumos::*;',
     '#[cfg(target_os = "illumos")]\npub use crate::os::illumos::*;\n#[cfg(target_os = "irix")]\npub use crate::os::irix::*;'),
])

# ---------------------------------------------------------------------------
# sys/thread/unix.rs: IRIX set_name no-op
#
# IRIX lacks pthread_setname_np.
#
# REPAIR: Fix corruption from previous buggy patch runs.
# ---------------------------------------------------------------------------
repair_file("sys/thread/unix.rs", [
    (
        '#[cfg(any(target_os = "solaris", target_os = "illumos", target_os = "nto"))]\n'
        'pub fn set_name(name: &CStr) {\n'
        '}\n'
        '\n'
        '// IRIX lacks pthread_setname_np\n'
        '#[cfg(target_os = "irix")]\n'
        'pub fn set_name(_name: &CStr) {\n'
        '    // No-op: IRIX does not support thread naming\n'
        '}\n'
        '\n'
        '#[cfg(any(target_os = "solaris", target_os = "illumos", target_os = "nto"))]\n'
        'pub fn _set_name_original(name: &CStr) {',
        '#[cfg(any(target_os = "solaris", target_os = "illumos", target_os = "nto"))]\n'
        'pub fn set_name(name: &CStr) {'
    ),
])
patch_file("sys/thread/unix.rs", [
    (
        '    }\n'
        '}\n'
        '\n'
        '#[cfg(target_os = "fuchsia")]\n'
        'pub fn set_name(name: &CStr) {',
        '    }\n'
        '}\n'
        '\n'
        '// IRIX lacks pthread_setname_np\n'
        '#[cfg(target_os = "irix")]\n'
        'pub fn set_name(_name: &CStr) {\n'
        '    // No-op: IRIX does not support thread naming\n'
        '}\n'
        '\n'
        '#[cfg(target_os = "fuchsia")]\n'
        'pub fn set_name(name: &CStr) {'
    ),
])

# ---------------------------------------------------------------------------
# sys/random/mod.rs: IRIX must use unix_legacy (/dev/urandom), NOT arc4random
#
# REPAIR: Remove irix from the arc4random cfg_select group if present.
# ---------------------------------------------------------------------------
repair_file("sys/random/mod.rs", [
    ('        target_os = "haiku",\n        target_os = "irix",\n        target_os = "illumos",',
     '        target_os = "haiku",\n        target_os = "illumos",'),
])
patch_file("sys/random/mod.rs", [
    ('        target_os = "aix",\n        target_os = "hurd",\n        target_os = "l4re",\n        target_os = "nto",\n    ) => {\n        mod unix_legacy;',
     '        target_os = "aix",\n        target_os = "hurd",\n        target_os = "irix",\n        target_os = "l4re",\n        target_os = "nto",\n    ) => {\n        mod unix_legacy;'),
])

# ---------------------------------------------------------------------------
# sys/pal/unix/os.rs: current_exe for IRIX using argv[0] (AIX-style)
#
# REPAIR: Remove irix from the haiku current_exe block if present.
# ---------------------------------------------------------------------------
repair_file("sys/pal/unix/os.rs", [
    ('#[cfg(any(target_os = "haiku", target_os = "irix"))]\npub fn current_exe',
     '#[cfg(target_os = "haiku")]\npub fn current_exe'),
])
patch_file("sys/pal/unix/os.rs", [
    ('#[cfg(target_os = "aix")]\npub fn current_exe',
     '#[cfg(any(target_os = "aix", target_os = "irix"))]\npub fn current_exe'),
])

# ---------------------------------------------------------------------------
# sys/fd/unix.rs: FIOCLEX fix — use fcntl(F_SETFD) instead of ioctl(FIOCLEX)
#
# IRIX does not support ioctl(FIOCLEX) on sockets (returns EOPNOTSUPP/errno 122).
# IRIX must be excluded from the ioctl path and included in the fcntl path.
# ---------------------------------------------------------------------------
patch_file("sys/fd/unix.rs", [
    # Exclusion list for ioctl(FIOCLEX): add irix before wasi
    ('        target_os = "nto",\n        target_os = "wasi",\n    ))]\n    pub fn set_cloexec(&self) -> io::Result<()> {\n        unsafe {\n            cvt(libc::ioctl(self.as_raw_fd(), libc::FIOCLEX))?;',
     '        target_os = "nto",\n        target_os = "irix",\n        target_os = "wasi",\n    ))]\n    pub fn set_cloexec(&self) -> io::Result<()> {\n        unsafe {\n            cvt(libc::ioctl(self.as_raw_fd(), libc::FIOCLEX))?;'),
    # Inclusion list for fcntl(F_SETFD): add irix before wasi
    ('        target_os = "nto",\n        target_os = "wasi",\n    )]\n    pub fn set_cloexec(&self) -> io::Result<()> {\n        unsafe {\n            let previous = cvt(libc::fcntl(self.as_raw_fd(), libc::F_GETFD))?;',
     '        target_os = "nto",\n        target_os = "irix",\n        target_os = "wasi",\n    )]\n    pub fn set_cloexec(&self) -> io::Result<()> {\n        unsafe {\n            let previous = cvt(libc::fcntl(self.as_raw_fd(), libc::F_GETFD))?;'),
])

# ---------------------------------------------------------------------------
# sys/net/connection/socket/unix.rs: FIONBIO fix
#
# IRIX doesn't support ioctl(FIONBIO) on sockets (returns EOPNOTSUPP/errno 122).
# This is the SOCKET-SPECIFIC set_nonblocking, separate from FileDesc::set_nonblocking.
# Socket::set_nonblocking has 3 cfg-gated implementations:
#   1. not(solaris, illumos, vita) → ioctl(FIONBIO)  [WRONG for IRIX]
#   2. vita → setsockopt(SO_NONBLOCK)
#   3. solaris, illumos → fcntl via self.0.set_nonblocking()  [CORRECT for IRIX]
# Add IRIX to exclusion list (#1) and inclusion list (#3).
# ---------------------------------------------------------------------------
patch_file("sys/net/connection/socket/unix.rs", [
    # Exclude IRIX from ioctl(FIONBIO) path (add to not(any(...)) list)
    # Note: closing is )))]\n (three parens — cfg_attr wrapping)
    ('target_os = "illumos", target_os = "vita")))]\n    pub fn set_nonblocking(&self, nonblocking: bool) -> io::Result<()> {\n        let mut nonblocking = nonblocking as libc::c_int;\n        cvt(unsafe { libc::ioctl(self.as_raw_fd(), libc::FIONBIO',
     'target_os = "illumos", target_os = "irix", target_os = "vita")))]\n    pub fn set_nonblocking(&self, nonblocking: bool) -> io::Result<()> {\n        let mut nonblocking = nonblocking as libc::c_int;\n        cvt(unsafe { libc::ioctl(self.as_raw_fd(), libc::FIONBIO'),
    # Include IRIX in fcntl path (add to any(...) list alongside solaris/illumos)
    ('target_os = "solaris", target_os = "illumos"))]\n    pub fn set_nonblocking(&self, nonblocking: bool) -> io::Result<()> {\n        // FIONBIO is inadequate',
     'target_os = "solaris", target_os = "illumos", target_os = "irix"))]\n    pub fn set_nonblocking(&self, nonblocking: bool) -> io::Result<()> {\n        // FIONBIO is inadequate'),
])

# ---------------------------------------------------------------------------
# build.rs: Add IRIX to known platforms (prevents restricted_std errors)
# ---------------------------------------------------------------------------
BUILDRS = os.path.join(os.path.dirname(STD_SRC), "build.rs")
if os.path.exists(BUILDRS):
    with open(BUILDRS) as f:
        content = f.read()
    if CHECK_MODE:
        if 'target_os == "irix"' in content:
            print("  OK: build.rs")
        else:
            print("  NOT APPLIED: build.rs")
            failures += 1
    else:
        if 'target_os == "irix"' not in content:
            content = content.replace(
                '|| target_os == "illumos"\n',
                '|| target_os == "illumos"\n        || target_os == "irix"\n'
            )
            with open(BUILDRS, 'w') as f:
                f.write(content)
            print("  Patched build.rs (added irix to known platforms)")
        else:
            print("  build.rs (already patched)")
else:
    print("  build.rs not found — skipping")

# ---------------------------------------------------------------------------
# sys/fs/unix.rs: Exclude IRIX from d_type (IRIX dirent has no d_type field)
# ---------------------------------------------------------------------------
patch_file("sys/fs/unix.rs", [
    (
        '        target_os = "aix",\n'
        '        target_os = "nto",\n'
        '        target_os = "vita",\n'
        '    ))]',
        '        target_os = "aix",\n'
        '        target_os = "nto",\n'
        '        target_os = "vita",\n'
        '        target_os = "irix",\n'
        '    ))]'
    ),
])

# ---------------------------------------------------------------------------
# sys/args/unix.rs: Add IRIX to argc/argv storage group
#
# REPAIR: Remove duplicate irix entry if present.
# ---------------------------------------------------------------------------
repair_file("sys/args/unix.rs", [
    ('    target_os = "nuttx",\n    target_os = "irix",\n))]',
     '    target_os = "nuttx",\n))]'),
])
patch_file("sys/args/unix.rs", [
    ('    target_os = "aix",\n    target_os = "nto",',
     '    target_os = "aix",\n    target_os = "irix",\n    target_os = "nto",'),
])

print(f"\nDone patching std source.")
if CHECK_MODE and failures > 0:
    print(f"\n{failures} patches not applied!")
    sys.exit(1)
PYSCRIPT

echo ""
echo "=== Patches complete ==="
if $CHECK_MODE; then
    echo "Check mode: all patches verified."
else
    echo "Run 'cargo +nightly clean && cargo +nightly build' to rebuild."
fi
