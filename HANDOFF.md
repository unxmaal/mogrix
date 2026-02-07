# Mogrix Cross-Compilation Handoff

**Last Updated**: 2026-02-07
**Status**: Full phased rebuild COMPLETE. All packages built, installed and verified on clean `/opt/chroot` via bootstrap tarball + MCP.

---

## CRITICAL WARNINGS

**NEVER install packages directly to /usr/sgug on the live IRIX system.**

On 2026-01-27, installing directly to /usr/sgug replaced libz.so with zlib-ng, broke sshd, and locked out SSH access. System recovered from console using backup libs.

**Always use /opt/chroot for testing.** This is our clean chroot bootstrapped via `scripts/bootstrap-tarball.sh`. The old `/opt/chroot_0206` has the full SGUG-RSE base (backup only).

**NEVER CHEAT!** Work through problems converting packages. Don't copy from old SGUG-RSE. Don't fake Provides. Check the rules for how to handle being stuck.

**Question SGUG-RSE assumptions.** SGUG-RSE bootstrapped ON IRIX without most deps. They bundled things and skipped system libraries out of necessity. We cross-compile with a full sysroot — prefer system libraries over bundled copies.

---

## Goal

Cross-compile Fedora 40 packages for IRIX using mogrix, a deterministic SRPM conversion engine. Phases 1-3 are complete. Next: coreutils, gettext, and other user-facing packages.

---

## Current Status

### Full Phased Rebuild (COMPLETE - 2026-02-07)

All packages rebuilt from clean state. Bootstrap tarball deployed to bare `/opt/chroot`, all packages installed via MCP (no SSH), verified working. 41 packages in rpm database.

### Phase 1: Bootstrap (14 packages)

| # | Package | Version | Status |
|---|---------|---------|--------|
| 1 | zlib-ng | 2.1.6 | INSTALLED |
| 2 | bzip2 | 1.0.8 | INSTALLED |
| 3 | popt | 1.19 | INSTALLED |
| 4 | xz | 5.4.6 | INSTALLED |
| 5 | openssl | 3.2.1 | INSTALLED |
| 6 | libxml2 | 2.12.5 | INSTALLED |
| 7 | curl | 8.6.0 | INSTALLED |
| 8 | lua | 5.4.6 | INSTALLED |
| 9 | file | 5.45 | INSTALLED |
| 10 | sqlite | 3.45.1 | INSTALLED |
| 11 | rpm | 4.19.1.1 | INSTALLED |
| 12 | libsolv | 0.7.28 | INSTALLED |
| 13 | tdnf | 3.5.14 | INSTALLED |
| 14 | sgugrse-release | 0.0.7beta | INSTALLED |

### Phase 1.5: System Libraries

| Package | Version | Status |
|---------|---------|--------|
| ncurses | 6.4 | INSTALLED |
| readline | 8.2 | INSTALLED |

### Phase 2: Build Tools (6 packages)

| Package | Version | Status | Notes |
|---------|---------|--------|-------|
| m4 | 1.4.19 | INSTALLED | - |
| perl | 5.38.2 | INSTALLED | Monolithic build, explicit Provides |
| bash | 5.2.26 | INSTALLED | System readline, libtinfo for termcap |
| autoconf | 2.71 | INSTALLED | PERL=/usr/bin/perl for host execution |
| automake | 1.16.5 | INSTALLED | shebang + FindBin + AUTOCONF/PERL exports |
| libtool | 2.4.7 | INSTALLED | Touch timestamps to prevent regeneration |

### Phase 3a: Crypto Stack (7 packages)

| Package | Version | Status |
|---------|---------|--------|
| pkgconf | 2.1.0 | INSTALLED |
| libgpg-error | 1.48 | INSTALLED |
| libgcrypt | 1.10.3 | INSTALLED |
| libassuan | 2.5.7 | INSTALLED |
| libksba | 1.6.6 | INSTALLED |
| npth | 1.7 | INSTALLED |
| gnupg2 | 2.4.4 | INSTALLED |

### Phase 3b: GNU Text Tools (3 packages)

| Package | Version | Status |
|---------|---------|--------|
| sed | 4.9 | INSTALLED |
| gawk | 5.3.0 | INSTALLED |
| grep | 3.11 | INSTALLED |

### Phase 4a: User-Facing Utilities (5 packages)

| Package | Version | Status | Key Fixes |
|---------|---------|--------|-----------|
| less | 643 | BUILT | Skip fsync AC_TRY_RUN patch, ac_cv override |
| which | 2.21 | BUILT | getopt_long compat only |
| gzip | 1.13 | BUILT | Source renumbering, gnulib-tests removal, nls-disabled |
| diffutils | 3.10 | BUILT | gnulib select override, gnulib-tests removal, nls-disabled |
| patch | 2.7.6 | BUILT | posix_spawn compat (full impl), skip SELinux patch |

**Total: 37 packages built and staged (32 installed + 5 pending install).**

---

## Fixes Made This Session (Phase 4a)

### 1. posix_spawn compat implementation completed
- `compat/include/mogrix-compat/generic/spawn.h` — added posix_spawnattr_* functions, file_actions_addopen
- `compat/runtime/spawn.c` — full posix_spawn with attr support (flags, sigmask, sigdefault, pgroup)
- Required by patch-2.7.6's gnulib `execute.c` which unconditionally uses posix_spawn

### 2. less fsync AC_TRY_RUN cross-compile fix
- Fedora's `less-475-fsync.patch` replaces AC_CHECK_FUNCS(fsync) with AC_TRY_RUN that hard-errors on cross-compile
- Fix: skip the patch, use `ac_cv_func_fsync: "yes"` override instead

### 3. diffutils gnulib select() override
- gnulib detects IRIX select() as "guessing no" for EBADF detection, builds a replacement
- Replacement select.c can't find the real select() declaration
- Fix: `gl_cv_func_select_detects_ebadf: "yes"` + `gl_cv_func_select_supports0: "yes"`

### 4. gzip Source100 renumbering
- gzip.spec already uses Source100/101 (colorzgrep), conflicts with compat source injection
- Fix: spec_replacements renumber Source100→50, Source101→51

### 5. patch SELinux patch skipped
- Fedora adds unconditional `#include <selinux/selinux.h>` and `-lselinux` via patch-selinux.patch
- IRIX has no SELinux; skip the patch via spec_replacement

---

## Previous Fixes

### 1. `cut -d: -f1` Colon Bug in generic.yaml (FIXED)
- **Problem**: install_cleanup's shebang-fixing pipeline used `cut -d: -f1` to parse `file` output, but Perl man pages have `::` in filenames (e.g., `TAP::Parser.3pm`), causing `cut` to split the filename
- **Fix**: Replaced `cut -d: -f1` with `sed 's/^\(.*\): .*/\1/'` — greedy match finds the LAST `: ` separator

### 2. `re.sub` Lambda Fix for install_cleanup (spec.py)
- **Problem**: Same `re.sub` f-string backreference bug we fixed for prep_commands also affected install_cleanup injection (lines 492 and 505)
- **Fix**: Changed both to use `lambda m: f"{m.group(1)}..."` pattern
- All 3 injection sites now use lambdas: patch injection, prep_commands, install_cleanup

### 3. Staging PATH Removed from rpmmacros.irix
- **Problem**: `%configure` macro added `/opt/sgug-staging/usr/sgug/bin` to PATH, exposing cross-compiled MIPS binaries (sed, perl, m4, etc.) that can't execute on the build host
- **Fix**: Removed `PATH=` export, added explicit tool overrides: `SED=/usr/bin/sed`, `AWK=/usr/bin/gawk`, `PERL=/usr/bin/perl`, `M4=/usr/bin/m4`
- GREP intentionally NOT set — GNU grep's configure rejects pre-set `$GREP`

### 4. Host Tool Exports for Build Tool Packages
- autoconf.yaml: `PERL=/usr/bin/perl`, `M4=/usr/bin/m4`
- automake.yaml: `PERL=/usr/bin/perl`, `AUTOCONF=/usr/bin/autoconf`, `AUTOM4TE=/usr/bin/autom4te`
- libtool.yaml: All autotools + `prep_commands` to touch timestamps

### 5. `drop_requires` for Linux-specific Runtime Deps
- openssl.yaml: `ca-certificates`, `crypto-policies`
- curl.yaml: `libnghttp2`, `libpsl`, `libssh`

### 6. libassuan Header Symlink
- libassuan installs to `include/libassuan2/assuan.h` (Fedora subdirectory convention)
- `libassuan-config --cflags` returns `-I/usr/sgug/include/libassuan2` which doesn't work through sysroot
- Fix: install_cleanup creates `assuan.h → libassuan2/assuan.h` symlink

### 7. Multiarch Header Fixes (from previous session, applied this rebuild)
- OpenSSL: `configuration-mips64.h → configuration-mips.h` (clang defines `__mips64` for n32)
- Lua: `luaconf-mips64.h → luaconf-mips.h` (same issue)

### 8. Bootstrap Tarball + Clean Chroot
- `scripts/bootstrap-tarball.sh` updated: RPM paths → `~/mogrix_outputs/RPMS/`, added `sgugrse-release-common` to manifest
- Bootstrap tarball (40MB compressed) deployed to bare `/opt/chroot`
- All Phase 1.5-3b packages installed via MCP `irix_copy_to` + `irix_exec`
- `pkgconf.yaml`: added `drop_requires: libpkgconf` (static build, no shared lib subpackage)

---

## Environment

| Purpose | Path |
|---------|------|
| Mogrix project | `/home/edodd/projects/github/unxmaal/mogrix/` |
| Cross toolchain | `/opt/cross/bin/` |
| Staging area | `/opt/sgug-staging/usr/sgug/` |
| IRIX sysroot | `/opt/irix-sysroot/` |
| IRIX host | `192.168.0.81` (use MCP, not SSH) |
| IRIX chroot (active) | `/opt/chroot` (bootstrapped, 41 packages) |
| IRIX chroot (backup) | `/opt/chroot_0206` (old SGUG-RSE base) |
| Original FC40 SRPMs | `~/mogrix_inputs/SRPMS/` |
| Converted SRPMs | `~/mogrix_outputs/SRPMS/` |
| Built MIPS RPMs | `~/mogrix_outputs/RPMS/` |
| rpmbuild workspace | `~/rpmbuild/` (ephemeral) |

### Workflow
```bash
# Build on Linux
uv run mogrix fetch <package> -y
uv run mogrix convert ~/mogrix_inputs/SRPMS/<pkg>-*.src.rpm
uv run mogrix build ~/mogrix_outputs/SRPMS/<pkg>-*-converted/<pkg>-*.src.rpm --cross
uv run mogrix stage ~/mogrix_outputs/RPMS/<pkg>*.rpm

# Install on IRIX via MCP (do NOT ssh as root)
# Use irix_copy_to to copy RPMs, irix_exec to run rpm -Uvh
```

---

## Next Steps

1. **Install Phase 4a on IRIX** via MCP: less, which, gzip, diffutils, patch (RPMs in ~/mogrix_outputs/RPMS/)
2. **Phase 4b**: findutils, tar, make, gettext, coreutils
3. **Phase 5: Development tools**: make, binutils, gcc (if cross-compiling GCC is feasible)
4. **Clean up stale files**: `-.o`, `m4.lang`, `perl.spec`, `HANDOFF.020426.md`, `docs/`
5. **tdnf ldconfig warning**: Add `remove_lines: ["/sbin/ldconfig"]` to tdnf.yaml or generic.yaml
6. **Rebuild pkgconf** with new `drop_requires: libpkgconf` rule (currently installed with `--nodeps`)

---

## Known Issues

- **`%zu` format specifier crashes IRIX libc** — use `%u` for size_t
- **Volatile function pointer static initializers crash** — provide `explicit_bzero` for `wipememory`-style patterns
- **GNU grep rejects pre-set $GREP** — don't export GREP in rpmmacros.irix
- **IRIX chroot doesn't fully isolate** — processes see base system paths
- fopencookie crashes on IRIX — use funopen instead
- sqlite3 CLI crashes when writing to files (rpm database writes work fine)
- odump/elfdump crash on LLD-produced binaries (rld loads them fine)
- pkgconf installed with `--nodeps` (needs rebuild with `drop_requires: libpkgconf` rule)
- Bare chroot has no coreutils — `head`, `tail`, `cat`, `ls` etc. are IRIX system versions only

---

## Key Architectural Decisions

| Decision | Rationale |
|----------|-----------|
| LLD 18 for executables, GNU ld + `-z separate-code` for shared libs | LLD for correct relocations; GNU ld with forced 3-segment layout for rld |
| `--image-base=0x1000000` for all executables | Default 0x10000 gives only 1.8MB brk heap |
| dlmalloc via mmap (auto-injected) | IRIX brk() heap limited; mmap accesses 1.2GB |
| No staging PATH in %configure | Cross-compiled MIPS binaries can't execute on build host |
| Explicit tool overrides (SED, AWK, PERL, M4) | Replace PATH-based tool discovery for cross-compilation |
| `sed` greedy match for `file` output parsing | `cut -d:` breaks on filenames with colons (Perl modules) |
| Lambda in re.sub replacements | f-string replacement interprets `\1` as backreference |
| System libs over bundled | Full sysroot available; don't copy SGUG-RSE bootstrap shortcuts |

---

## Reference Documents

| Document | Purpose |
|----------|---------|
| `rules/INDEX.md` | Rules reference, per-package problem guide |
| `rules/methods/before-you-start.md` | Checklist before debugging |
| `rules/generic.yaml` | Universal rules for all packages |
| `compat/catalog.yaml` | Compat function registry |
| `plan.md` | Project plan and architecture |
| `rules/methods/mogrix-workflow.md` | How to run mogrix |
| `rules/methods/irix-testing.md` | IRIX shell rules, chroot, debugging |
