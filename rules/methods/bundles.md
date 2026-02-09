# Bundle Architecture

Mogrix bundles are self-contained app tarballs for IRIX — like Flatpaks for vintage SGI hardware. Users extract a tarball, run `./install`, and the app works. No package manager, no dependency hell, one PATH entry for everything.

## Why Bundles Exist

IRIX has no modern package ecosystem. RPMs are how we build and track dependencies during cross-compilation, but they're not the end-user deliverable. Users don't want to learn RPM, resolve deps, or maintain a package database. They want to download a file, extract it, and run a program.

Bundles solve this by:
- Including all mogrix-built shared libraries the app needs
- Skipping IRIX native libraries (always available on the target machine)
- Setting `LD_LIBRARYN32_PATH` transparently via wrapper scripts
- Providing a single `bin/` directory that holds all installed bundles

## Layout

```
/opt/mogrix-apps/                       # User extracts bundles here
  bin/                                  # ONE directory in PATH (trampolines)
    nano                                # → ../nano-7.2-6a-irix-bundle/nano
    weechat                             # → ../weechat-4.2.1-2a-irix-bundle/weechat
  nano-7.2-6a-irix-bundle/
    nano, rnano, tput, ...              # Wrapper scripts
    install                             # Creates trampolines in ../bin/
    uninstall                           # Removes them
    README                              # Generated docs
    _bin/                               # Actual ELF binaries
    _sbin/                              # System binaries (if any)
    _lib32/                             # Shared libraries (pruned)
    share/                              # Data files
      terminfo/                         # Trimmed to ~30 common terminals
    etc/                                # Config files (if needed)
      pki/tls/certs/ca-bundle.crt       # CA certs for TLS apps
```

## Three Layers of Scripts

### 1. Trampolines (`bin/nano`)

Tiny 4-line scripts that live in the shared `bin/` directory. Their only job is to resolve their own location and exec the real wrapper. One `bin/` directory, one PATH entry, all bundles accessible.

```sh
#!/bin/sh
dir=`dirname "$0"`
case "$dir" in /*) ;; *) dir="`pwd`/$dir" ;; esac
exec "$dir/../nano-7.2-6a-irix-bundle/nano" "$@"
```

Why not symlinks? Because `dirname "$0"` on a symlink resolves to the symlink's directory, not the target's. The wrapper needs its real location to find `_lib32/`.

### 2. Wrappers (`nano-7.2-6a-irix-bundle/nano`)

Set up the runtime environment and exec the real binary. Every command in the bundle gets one.

```sh
#!/bin/sh
dir=`dirname "$0"`
case "$dir" in /*) ;; *) dir="`pwd`/$dir" ;; esac
if [ -n "$LD_LIBRARYN32_PATH" ]; then
    LD_LIBRARYN32_PATH="$dir/_lib32:$LD_LIBRARYN32_PATH"
else
    LD_LIBRARYN32_PATH="$dir/_lib32"
fi
export LD_LIBRARYN32_PATH
TERMINFO="$dir/share/terminfo"
export TERMINFO
exec "$dir/_bin/nano" "$@"
```

Key design decisions:
- **Bourne shell only**: IRIX `/bin/sh` is the original Bourne shell. No `$(...)`, no `${var:+...}`, no arrays. Only backticks, basic `if/else`, `case`.
- **Relative paths**: No baked-in absolute paths. Works regardless of where the bundle is installed.
- **Prepend `LD_LIBRARYN32_PATH`**: Bundle libs take priority but don't clobber existing paths.
- **TERMINFO override**: Points to the trimmed terminfo database in the bundle.

### 3. Install/Uninstall (`install`, `uninstall`)

`install` creates the trampolines in `../bin/`. `uninstall` removes them. Both are generated automatically from the list of binaries in the bundle.

## Dependency Resolution

Bundles resolve dependencies via **ELF NEEDED scanning**, not RPM metadata (since rpmmacros.irix sets `AutoReq: no`).

The process:
1. Extract target RPM + sibling RPMs (same SOURCERPM, excluding -devel/-debuginfo)
2. `readelf -d` every ELF binary to find NEEDED sonames
3. For each soname:
   - **IRIX native** (317 sonames in `/usr/lib32` + `/lib32`): Skip — always available
   - **mogrix-built RPM**: Add that RPM + its siblings to the queue
   - **Staging fallback**: Copy from `/opt/sgug-staging/usr/sgug/lib32/`
   - **Unresolved**: Warning (bundle may not work)
4. Repeat until all transitive deps are resolved

SOURCERPM sibling grouping means `groff` auto-includes `groff-base` and `groff-perl`, because they're built from the same source.

## Bundle Optimization

After extraction, the bundle is aggressively optimized:

| Optimization | What it does | Typical savings |
|-------------|-------------|-----------------|
| Library pruning | Remove .so files not transitively NEEDED by any binary | ncurses-libs: 14 → 3 libs |
| Doc stripping | Remove `share/doc/`, `share/man/`, `share/info/`, `share/licenses/` | Several MB |
| Terminfo trimming | Keep only ~30 common terminals (iris-ansi, xterm, vt100, screen, etc.) | ~12MB → ~50KB |
| Staging symlinks | Symlink soname → real file instead of two full copies | Avoids duplicates |
| CA cert inclusion | Copy build host CA bundle for TLS apps | ~200KB |

Result: nano bundle 48MB → 3.9MB (tarball 13MB → 0.9MB).

## App-Specific Customization

Some apps need more than just `LD_LIBRARYN32_PATH`. The wrapper template has two extension points:

- **`{extra_env_block}`**: Additional environment variables. Example: `WEECHAT_EXTRA_LIBDIR` for plugin dlopen paths.
- **`{extra_args}`**: CLI arguments prepended before `"$@"`. Example: weechat's `-r "/set weechat.network.gnutls_ca_user ..."` for CA cert configuration.

These are currently detected heuristically in `bundle.py`:
- weechat plugin directory detected → `WEECHAT_EXTRA_LIBDIR` set
- CA bundle present + weechat detected → `-r` argument for gnutls CA config
- CA bundle present → `SSL_CERT_FILE` set (for OpenSSL-based apps)

Future apps may need similar customization. The pattern is: detect in `create_bundle()`, add to the `extra_env_lines` list or `extra_args_map` dict.

## Versioning

Bundle names include an alphabetic revision suffix: `weechat-4.2.1-2a-irix-bundle`. When rebuilding, the suffix auto-increments (a → b → c). Old bundle directories and tarballs for the same version are cleaned up automatically.

This lets users distinguish between rebuilds of the same package version (e.g., after fixing a TLS bug).

## Invocation

```bash
uv run mogrix bundle <package>
```

Output goes to `~/mogrix_outputs/bundles/`. The tarball is what gets copied to IRIX.

## User Installation

```sh
# On IRIX (one-time setup)
mkdir -p /opt/mogrix-apps
PATH=/opt/mogrix-apps/bin:$PATH; export PATH
# Add that PATH line to ~/.profile

# Per bundle
cd /opt/mogrix-apps
gzip -dc /tmp/weechat-4.2.1-2a-irix-bundle.tar.gz | tar xf -
cd weechat-4.2.1-2a-irix-bundle
./install
weechat
```

## Key Files

| File | Purpose |
|------|---------|
| `mogrix/bundle.py` | Bundle builder implementation |
| `mogrix/cli.py` | CLI `bundle` subcommand |
| `~/mogrix_outputs/bundles/` | Output directory |
| `~/mogrix_outputs/RPMS/` | Input RPMs |
| `/opt/irix-sysroot/` | IRIX native soname catalog |
