# Mogrix: SRPM Conversion Engine for IRIX

Mogrix is a deterministic SRPM-to-RSE-SRPM conversion engine that transforms Fedora SRPMs into IRIX-compatible packages. It centralizes all platform knowledge required to adapt Linux build intent for IRIX reality.

## Current Status (2026-01-26)

**Phase 9: TDNF REPOSITORY SETUP**

All 12 packages built successfully. Bootstrap tarball created. Now setting up tdnf repository server to validate end-to-end package installation workflow.

**Milestone: tdnf dependency chain COMPLETE (12/12 packages)**
**Next: Set up repo server, configure IRIX chroot, test `tdnf install`**

### Validation Results

1. Clean environment completely (`./cleanup.sh`) - DONE
2. Build each package individually - DONE
3. All packages built successfully
4. One new fix required: `posix_spawn_file_actions_*` functions

### Package Chain (COMPLETE)

| # | Package | Version | Status | Notes |
|---|---------|---------|--------|-------|
| 1 | zlib-ng | 2.1.6 | **COMPLETE** | cmake-based, _XOPEN_SOURCE=600 |
| 2 | bzip2 | 1.0.8 | **COMPLETE** | Built first try |
| 3 | popt | 1.19 | **COMPLETE** | stpcpy compat, libtool fixes |
| 4 | openssl | 3.2.1 | **COMPLETE** | Built first try |
| 5 | libxml2 | 2.12.5 | **COMPLETE** | libtool fixes |
| 6 | curl | 8.6.0 | **COMPLETE** | __mips64 fix, LD export |
| 7 | xz | 5.4.6 | **COMPLETE** | libtool fixes |
| 8 | lua | 5.4.6 | **COMPLETE** | Built first try |
| 9 | file | 5.45 | **COMPLETE** | **Required posix_spawn_file_actions_* fix** |
| 10 | rpm | 4.19.1.1 | **COMPLETE** | spawn.h, fcntl.h, sys/stat.h compat |
| 11 | libsolv | 0.7.28 | **COMPLETE** | Built first try |
| 12 | **tdnf** | **3.5.14** | **COMPLETE** | **TARGET ACHIEVED** (from Photon OS) |

### Validation Round Summary (Completed 2026-01-25)

All 12 packages built successfully. One new fix was needed during validation:

| Fix | Files | Description |
|-----|-------|-------------|
| spawn.h compat | `compat/include/.../spawn.h`, `compat/runtime/spawn.c` | posix_spawn/posix_spawnp via fork+exec |
| **spawn file actions** | `compat/include/.../spawn.h`, `compat/runtime/spawn.c` | **NEW: posix_spawn_file_actions_init/destroy/addclose/adddup2** |
| fcntl.h compat | `compat/include/.../fcntl.h` | O_NOFOLLOW, O_CLOEXEC, AT_* constants, openat decl |
| sys/stat.h compat | `compat/include/.../sys/stat.h` | fstatat, mkdirat, fchmodat, etc. declarations |
| unistd.h updates | `compat/include/.../unistd.h` | faccessat, fchownat, unlinkat, etc. declarations |
| xz libtool fix | `rules/packages/xz.yaml` | Post-configure sed for shared libs |
| file compat linking | `rules/packages/file.yaml` | Add mogrix-compat to src/Makefile LIBS |

### All Known Gaps Fixed

| Issue | Status |
|-------|--------|
| `__mips=1` in irix-cc | Fixed - also added `-U__mips64` |
| unistd.h sysconf compat | Fixed in compat/include |
| spawn.h for rpm | **FIXED** - new compat header + implementation |
| **spawn file actions for file** | **FIXED** - posix_spawn_file_actions_init/destroy/addclose/adddup2 |
| POSIX.1-2008 *at functions | **FIXED** - declarations in fcntl.h, sys/stat.h, unistd.h |
| AT_SYMLINK_NOFOLLOW, O_NOFOLLOW | **FIXED** - defined in fcntl.h |
| multiarch symlinks (OpenSSL, lua) | Auto-handled by staging |
| irix-cc diagnostic options | Fixed - handles -print-search-dirs etc |
| irix-ld diagnostic options | Fixed - passes to GNU ld.bfd |

### Toolchain Fixes (Session 2026-01-25)

| Fix | File | Description |
|-----|------|-------------|
| Clang diagnostic options | `/opt/sgug-staging/usr/sgug/bin/irix-cc` | Handle -print-search-dirs, --version, etc. by passing to clang instead of linker |
| GNU ld diagnostic options | `/opt/sgug-staging/usr/sgug/bin/irix-ld` | Handle -print-search-dirs by passing to GNU ld.bfd |
| `__mips64` macro | `/opt/sgug-staging/usr/sgug/bin/irix-cc` | Undefine `__mips64`/`__mips64__` for N32 ABI (breaks OpenSSL multiarch headers) |
| stpcpy compat | `compat/runtime/stpcpy.c` | IRIX libc doesn't have stpcpy |
| spawn.h compat | `compat/include/.../spawn.h` + `compat/runtime/spawn.c` | posix_spawn/posix_spawnp + file_actions for rpm, file |
| fcntl.h compat | `compat/include/.../fcntl.h` | O_NOFOLLOW, AT_* constants for rpm |
| sys/stat.h compat | `compat/include/.../sys/stat.h` | fstatat, mkdirat, etc. for rpm |
| unistd.h updates | `compat/include/.../unistd.h` | POSIX.1-2008 *at function declarations |

### Key Compat Functions Implemented

| Function | Package | File | Status |
|----------|---------|------|--------|
| POSIX.1-2008 "at" funcs | rpm, libsolv | openat-compat.c | DONE |
| posix_spawn/posix_spawnp | rpm, file | spawn.c | DONE |
| posix_spawn_file_actions_* | file | spawn.c | DONE |
| getprogname/setprogname | rpm | getprogname.c | DONE |
| fopencookie | libsolv | fopencookie.c | DONE |
| getline | libsolv, tdnf, file | getline.c | DONE |
| strdup/strndup | file, xz, libsolv, tdnf | strdup.c, strndup.c | DONE |
| strcasestr | libsolv | strcasestr.c | DONE |
| strsep | tdnf | strsep.c | DONE |
| timegm | libsolv | timegm.c | DONE |
| mkdtemp | libsolv | mkdtemp.c | DONE |
| qsort_r | libsolv, tdnf | qsort_r.c | DONE |
| asprintf/vasprintf | tdnf, file | asprintf.c | DONE |
| getopt_long | file, tdnf | getopt_long.c | DONE |
| sqlite3_stub | tdnf | sqlite3_stub.c | DONE |

### Staging Automation (COMPLETE)

All staging workarounds are now automated in `mogrix stage`:

| Feature | Command | Status |
|---------|---------|--------|
| Auto-devel packages | `mogrix stage pkg.rpm` | DONE - auto-includes matching -devel |
| Multiarch headers | Automatic | DONE - creates mips64 from x86_64 |
| RPM arch naming | `--target mips-sgi-irix` | DONE - packages named .mips.rpm |

**Previous milestone: TDNF RUNNING ON IRIX - GOAL ACHIEVED**

### Major Milestone: tdnf Package Manager Works on IRIX

| Build Type | Result | Status |
|------------|--------|--------|
| Executables | Works | Validated on IRIX |
| Static libraries (.a) | Works | Phase 1/2 validated |
| **Shared libraries (.so)** | **Works** | **SOLVED - use GNU ld** |
| Dynamically linked executables | **Works** | **Use LLD with /lib32/rld interpreter** |
| **tdnf** | **Works** | **GOAL ACHIEVED** |

### Critical Discovery: Dynamic Linker Interpreter

The key to making dynamically linked executables work was specifying the correct interpreter:

```
--dynamic-linker=/lib32/rld
```

**What didn't work:**
- `/usr/lib32/libc.so.1` as interpreter (default) - segfaults
- GNU ld for executables - produces single RWE segment, segfaults

**What works:**
- LLD with `--dynamic-linker=/lib32/rld` - produces working executables

### Linker Selection

| Build Type | Linker | Interpreter | Notes |
|------------|--------|-------------|-------|
| Shared libraries (-shared) | GNU ld | N/A | Correct 2-segment layout |
| Executables | LLD | /lib32/rld | Must specify interpreter explicitly |

### What's Validated

- Cross-compiled executables work (tree, coreutils, tar, gzip, sed, grep, make, patch, bash)
- Static libraries work (zlib, bzip2, xz, ncurses, readline, popt)
- **Shared libraries work** (libsolv, libsolvext, libtdnf, libtdnfcli)
- **Dynamically linked executables work** (dumpsolv, curl, tdnf)
- rpmbuild cross-compilation workflow works
- mogrix conversion produces buildable SRPMs
- **tdnf runs on IRIX** (needs configuration)

---

## Bootstrap Strategy

With shared libraries working, we can now build packages with full shared library support. No static-only workaround needed.

### Build Order

```
Phase A: Foundation (DONE - validated on IRIX)
├── zlib, bzip2, xz           (compression)
├── ncurses, readline         (terminal)
└── popt                      (option parsing) - SHARED LIB VALIDATED

Phase B: Build Tools
├── make, patch, tar, gzip    (DONE - working)
├── autoconf, automake, libtool (need rules)
└── perl (minimal, for autotools)

Phase C: Security/Crypto
├── openssl                   (TLS)
├── libgpg-error, libgcrypt
├── libassuan, libksba
├── nettle, gnutls
└── gnupg2

Phase D: Package Management
├── expat, libxml2            (XML parsing)
├── elfutils, file            (ELF handling)
├── libarchive                (archive extraction)
├── rpm                       ← KEY MILESTONE
├── libsolv                   (dependency solving)
└── tdnf                      ← GOAL

Phase E: Full Ecosystem
├── Additional packages as needed
└── Full SGUG-RSE compatibility
```

---

## Core Concept

**Mogrix is not:**
- A distro
- A ports tree
- A replacement RPM system
- A separate compatibility library (no libdicl dependency)

**Mogrix is:**
- A semantic adapter between Linux SRPM intent and IRIX reality
- A deterministic conversion engine with auditable, testable behavior
- A single choke point for all IRIX platform knowledge
- **Self-contained**: all compatibility fixes are injected per-package, not linked from a shared library

This design follows the insight that libdicl's true value was never the library itself—it was the institutional memory of how to turn Linux intent into IRIX reality. Mogrix absorbs that knowledge and applies it directly to each package, eliminating the need for a separate pre-compiled compatibility library.

**Why no separate libdicl?**
- No pre-emptive cross-compilation of a compat library required
- Each package is self-contained with its fixes
- Easier to test and debug individual packages
- No version skew between libdicl and packages
- Compatibility code is only included when actually needed

---

## What Works Now

| Feature | Status | Location |
|---------|--------|----------|
| Spec file parsing | Done | `mogrix/parser/spec.py` |
| YAML rule loading | Done | `mogrix/rules/loader.py` |
| Rule application engine | Done | `mogrix/rules/engine.py` |
| Header overlay system | Done | `mogrix/headers/overlay.py` |
| Generic clang_compat headers | Done | `headers/generic/` (16 headers) |
| Compat source injection | Done | `mogrix/compat/injector.py` |
| Spec file rewriting | Done | `mogrix/emitter/spec.py` |
| SRPM extraction | Done | `mogrix/parser/srpm.py` |
| SRPM emitter/repackaging | Done | `mogrix/emitter/srpm.py` |
| CLI (analyze, convert, build) | Done | `mogrix/cli.py` |
| Patch catalog | Done | `mogrix/patches/catalog.py` |
| Configure flag manipulation | Done | add/remove flags |
| Conditional handling | Done | %if/%endif blocks |
| Subpackage management | Done | drop debuginfo, langpacks |
| Batch SRPM conversion | Done | `mogrix/batch.py` |
| Dependency resolution | Done | `mogrix/deps/resolver.py` |
| Fedora SRPM fetching | Done | `mogrix/deps/fedora.py` |
| Build with dep detection | Done | `mogrix/cli.py` |
| Fetch command | Done | `mogrix/cli.py` |
| **export_vars rule** | Done | Set env vars in %build |
| **skip_find_lang rule** | Done | Handle NLS disabled |
| **skip_check rule** | Done | Comment out %check for cross-compilation |
| **install_cleanup rule** | Done | Post-install cleanup |
| **GNU ld auto-selection** | Done | `cross/bin/irix-ld` |

### Test Coverage

- **107 tests**, all passing
- Covers: parser, rules, engine, emitter, headers, compat, patches, CLI, batch, deps

### Cross-Compilation Validated (Phase 8 - FC40 + Photon OS)

| Package | Version | Type | Result |
|---------|---------|------|--------|
| zlib-ng | 2.1.6 | shared lib | Works |
| bzip2 | 1.0.8 | shared lib | Works |
| popt | 1.19 | shared lib | Works |
| openssl | 3.2.1 | shared lib | Works |
| libxml2 | 2.12.5 | shared lib | Works |
| curl | 8.6.0 | shared lib + exe | Works |
| xz | 5.4.6 | shared lib | Works |
| lua | 5.4.6 | shared lib | Works |
| file | 5.45 | shared lib + exe | Works |
| rpm | 4.19.1.1 | shared lib + exe | Works |
| libsolv | 0.7.28 | shared lib | Works |
| **tdnf** | **3.5.14** | **shared lib + exe** | **TARGET COMPLETE** |

### Historical Validation (Earlier Sessions)

| Package | Type | Result |
|---------|------|--------|
| tree | executable | Works |
| bash | executable | Works |
| coreutils | executables | Works |
| tar, gzip, sed, grep | executables | Works |
| make, patch | executables | Works |
| ncurses, readline | static libs | Works |

### Full tdnf Dependency Chain: VALIDATED

All 12 packages in the tdnf dependency chain build and link correctly for IRIX. Built RPMs available in `~/rpmbuild/RPMS/mips/`.

---

## Package Rules Created

| Package | Compat Functions | Notes |
|---------|-----------------|-------|
| popt | strdup, strndup | First shared lib validated |
| libgpg-error | strdup, strndup | GnuPG error library |
| libgcrypt | - | Crypto library |
| libassuan | strdup, strndup | IPC library |
| libksba | - | X.509 library |
| nettle | - | Low-level crypto |
| libtasn1 | strdup, strndup, getline | ASN.1 parsing |
| p11-kit | strdup, strndup, asprintf, vasprintf, getline | PKCS#11 |
| gnutls | strdup, strndup, asprintf, vasprintf, getline | TLS library |
| gpgme | strdup, strndup, asprintf, vasprintf | GnuPG Made Easy |
| gnupg2 | strdup, strndup, asprintf, vasprintf, getline | GNU Privacy Guard |
| openssl | - | TLS/SSL (uses ./Configure) |

---

## Next Steps

### Validation Round: COMPLETE (12/12)

All 12 packages built successfully from clean state. All fixes captured in:
- mogrix rules (YAML files in `rules/packages/`)
- compat headers (`compat/include/mogrix-compat/generic/`)
- compat runtime (`compat/runtime/`)

| Fix | Status |
|-----|--------|
| sys/stat.h wrapper | DONE - fstatat, mkdirat, fchmodat etc. |
| fcntl.h wrapper | DONE - O_NOFOLLOW, AT_* constants |
| unistd.h wrapper | DONE - *at function declarations |
| spawn.h compat | DONE - posix_spawn/posix_spawnp + file_actions |
| `%systemd_*` scriptlets | DONE - in `generic.yaml` remove_lines |
| RPM architecture naming | DONE - `--target mips-sgi-irix` added |
| -devel package staging | DONE - auto-included by `mogrix stage` |
| Multiarch headers | DONE - auto-created during staging |
| irix-cc diagnostic options | DONE - handles -print-search-dirs |
| irix-ld diagnostic options | DONE - passes to GNU ld.bfd |
| __mips64 for N32 ABI | DONE - undefined in irix-cc |
| libtool shared lib fixes | DONE - in package YAML rules |

---

## Long-Term Goal: Modern Browser on IRIX

**Target**: WebKitGTK-based browser

### Why WebKitGTK?

1. **Most widely ported browser engine** - iOS, GTK, PlayStation, embedded systems
2. **SGUG-RSE has most dependencies already** - GTK3, Cairo, ICU, glib2, libsoup, etc.
3. **Compatible with our toolchain** - C++ with some C
4. **JavaScriptCore has MIPS support** - interpreter mode will work

### WebKitGTK Dependency Status in SGUG-RSE

| Dependency | Status | Version |
|------------|--------|---------|
| GTK3 | ✓ Present | 3.24.13 |
| Cairo | ✓ Present | - |
| ICU | ✓ Present | 63.2 |
| glib2 | ✓ Present | - |
| Pango | ✓ Present | - |
| HarfBuzz | ✓ Present | - |
| libsoup | ✓ Present | 2.68.4 |
| GStreamer1 | ✓ Present | - |
| freetype | ✓ Present | - |
| fontconfig | ✓ Present | - |
| pixman | ✓ Present | - |
| atk | ✓ Present | - |
| libxml2 | ✓ Present | - |
| libxslt | ✓ Present | - |
| sqlite | ✓ Present | - |
| WebKitGTK | **MISSING** | Target: 2.38.x or 2.40.x |

### WebKitGTK Phases

**Phase 1: Assess & Baseline**
- Pick WebKitGTK version (2.38.x or 2.40.x - last before heavy C++20 requirements)
- Verify all SGUG-RSE dependencies build and run on IRIX via mogrix
- Create mogrix package rule for webkitgtk

**Phase 2: Build Foundation via Mogrix**
- glib2, cairo, pango, harfbuzz, ICU, libsoup
- Verify cross-compilation works

**Phase 3: WebKit Core**
- Attempt WebKitGTK build
- Start with `-DENABLE_JIT=OFF` (interpreter-only JavaScript)
- Apply GOT fixes similar to Qt5 (`-LD_LAYOUT:lgot_buffer=1000`)
- Fix IRIX-specific issues as they arise

**Phase 4: Browser**
- Epiphany (GNOME Web) or Surf (suckless, ~2000 lines)

---

### Immediate Next Steps

#### 1. Set Up tdnf Repository Server (DONE)

On Linux build VM:
```bash
# Update repo with RPMs and generate metadata
./scripts/update-repo.sh

# Start HTTP server
./scripts/start-repo-server.sh
# Serves at http://192.168.8.88:8080/mips/
```

Files created:
- `configs/tdnf/tdnf.conf` - tdnf main configuration
- `configs/tdnf/irix-local.repo` - repository definition
- `scripts/create-repo.py` - generates rpm-md metadata without createrepo_c
- `scripts/update-repo.sh` - copies RPMs and regenerates metadata
- `scripts/start-repo-server.sh` - starts HTTP server
- `scripts/deploy-tdnf-configs.sh` - deploys configs to IRIX via SSH

#### 2. Configure tdnf on IRIX Chroot (NEXT)

Deploy configs using the helper script:
```bash
./scripts/deploy-tdnf-configs.sh edodd@192.168.0.81 /opt/chroot
```

Or manually create:
- `/opt/chroot/etc/tdnf/tdnf.conf` - from `configs/tdnf/tdnf.conf`
- `/opt/chroot/etc/yum.repos.d/irix-local.repo` - from `configs/tdnf/irix-local.repo`

On IRIX chroot:
```bash
# Enter chroot
chroot /opt/chroot /bin/sh

# Set environment
export LD_LIBRARY_PATH=/usr/sgug/lib32
export PATH=/usr/sgug/bin:$PATH

# Test tdnf
tdnf repolist
tdnf makecache
```

#### 3. Test tdnf Install (New Package)

Build a test package that isn't already in the chroot:
```bash
# On build VM - build bash or another simple package
mogrix fetch bash
mogrix convert bash-*.src.rpm
mogrix build --cross bash-*.src.rpm

# Update repo
./scripts/update-repo.sh
```

On IRIX chroot:
```bash
# Refresh metadata
tdnf makecache

# List available packages
tdnf list available

# Install test package
tdnf install bash
```

#### 4. Validate End-to-End Workflow

| Step | Test |
|------|------|
| Build package | `mogrix build --cross pkg.src.rpm` |
| Update repo | `./scripts/update-repo.sh` |
| Refresh on IRIX | `tdnf makecache` |
| Install on IRIX | `tdnf install pkg` |

---

#### 5. Port gpgcheck.c to rpm 4.19 API (Lower Priority)

The gpgcheck.c is stubbed because it uses OLD rpm PGP API:

| Old API (tdnf uses) | New API (rpm 4.19) |
|---------------------|-------------------|
| `pgpDig` type | `pgpDigParams` |
| `pgpNewDig()` | (removed) |
| `pgpFreeDig()` | `pgpDigParamsFree()` |
| `pgpPrtPkts()` | `pgpPrtParams()` |

**Impact of stub:** Package signature verification disabled. Security implications for production.

**Fix:** Create patched gpgcheck.c using new API, add to mogrix patches.

#### 6. Future Improvements (Lower Priority)

| Pattern | Currently In | Should Be In |
|---------|--------------|--------------|
| cmake cross-compile settings | tdnf.yaml | cmake class (future) |
| Python bindings disable | tdnf.yaml | python class (future) |

---

## IRIX Test Environment

- **Host**: IRIX 6.5.30 Octane
- **SSH**: `ssh edodd@192.168.0.81`
- **SGUG shell**: `/usr/sgug/bin/sgugshell` (interactive)
- **Remote commands**: `/opt/sgug-exec <command>` (non-interactive)

## Key Paths

| Purpose | Path |
|---------|------|
| IRIX sysroot | `/opt/irix-sysroot/` |
| Staging area | `/opt/sgug-staging/usr/sgug/` |
| Cross toolchain | `/opt/cross/bin/` |
| Cross GNU ld | `/opt/cross/bin/mips-sgi-irix6.5-ld.bfd` |
| Mogrix project | `/home/edodd/projects/github/unxmaal/mogrix/` |

## Reference Documents

- `HANDOFF.md` - Session-to-session handoff notes
- `CLAUDE.md` - Project instructions and known issues

---

## Architecture Overview

```
FC40 SRPM  ─────────────────────────────────────────────────────────────┐
   │                                                                     │
   │  (extract)                                                          │
   ▼                                                                     │
┌──────────────────────────────────────────────────────────────────┐    │
│                           MOGRIX                                  │    │
│                                                                   │    │
│  ┌─────────────┐    ┌─────────────┐    ┌─────────────┐          │    │
│  │   Parser    │───▶│ Rule Engine │───▶│   Emitter   │          │    │
│  │ (spec+src)  │    │             │    │ (SRPM+report)│          │    │
│  └─────────────┘    └─────────────┘    └─────────────┘          │    │
│                            │                                      │    │
│            ┌───────────────┼───────────────┐                     │    │
│            ▼               ▼               ▼                     │    │
│     ┌──────────┐    ┌──────────┐    ┌──────────┐                │    │
│     │  Rules   │    │  Header  │    │  Compat  │                │    │
│     │ (YAML)   │    │ Overlays │    │ Sources  │                │    │
│     └──────────┘    └──────────┘    └──────────┘                │    │
│                      (stdarg.h,      (strdup.c,                  │    │
│                       stdio.h...)     getline.c...)              │    │
│                                                                   │    │
│  ┌─────────────────────────────────────────────────────────────┐ │    │
│  │                  Dependency Resolution                       │ │    │
│  │  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │ │    │
│  │  │   Resolver   │  │ FedoraRepo   │  │   Fetcher    │──────┼─┼────┘
│  │  │(parse errors)│  │(search/find) │  │  (download)  │      │ │
│  │  └──────────────┘  └──────────────┘  └──────────────┘      │ │
│  │  Parses rpmbuild errors → Finds SRPM URLs → Downloads deps  │ │
│  └─────────────────────────────────────────────────────────────┘ │
└──────────────────────────────────────────────────────────────────┘
   │
   ▼
RSE SRPM (self-contained, no libdicl dependency)
   │
   ├── Modified spec file
   ├── Original sources + patches
   ├── Injected compat headers (bundled)
   └── Injected compat sources (bundled)
```

**Key principle:** Each converted SRPM contains everything needed to build on IRIX. No external compatibility library required. Automatic linker selection ensures shared libraries work correctly.

---

## Success Criteria

1. **Functional:** Can convert and build tdnf and all dependencies
2. **Self-contained:** Converted packages have no libdicl dependency; all compat code is bundled
3. **No bootstrap required:** Pure cross-compilation from source, no pre-existing IRIX packages needed
4. **Auditable:** Every transformation is logged and explainable
5. **Extensible:** Adding new rules requires only YAML, not core code changes
6. **Reproducible:** Same input SRPM + rules = identical output SRPM
7. **Ultimate goal:** Working `tdnf` package manager on IRIX 6.5

---

## Resolved Issues

### RESOLVED: Shared Library Generation

**Status**: SOLVED - Use GNU ld for shared libraries

**Solution**:
- Cross GNU ld (`mips-sgi-irix6.5-ld.bfd`) produces correct IRIX-compatible shared libraries
- LLD's 3-LOAD-segment layout crashed IRIX; GNU ld's 2-LOAD-segment layout works
- The `irix-ld` wrapper automatically selects GNU ld when `-shared` is present

**Validation**:
```bash
# On IRIX
dlopen succeeded!
Found poptGetContext at 5ffe1258
Test return code: 0
```

### RESOLVED: Dynamic Executable Interpreter

**Status**: SOLVED - Use `/lib32/rld` as interpreter

**Problem**: Cross-compiled dynamically linked executables segfaulted on IRIX even with correct shared libraries.

**Root Cause**: LLD defaults to `/usr/lib32/libc.so.1` as the interpreter, but IRIX requires `/lib32/rld`.

**Solution**: Add `--dynamic-linker=/lib32/rld` to the LLD link command for executables.

**Validation**:
```bash
# On IRIX
$ /tmp/dumpsolv-lld-rld
unexpected EOF
not a SOLV file
could not read repository: not a SOLV file
Exit: 1  # Expected - no input file

$ /tmp/tdnf repolist
Error(1602) : No such file or directory
Exit: 66  # Expected - no config file
```

### Documented in CLAUDE.md

- IRIX header ordering requirements (time.h before sys/time.h)
- MIPSpro builtin type definitions needed
- scandir/alphasort hidden behind _SGIAPI when _XOPEN_SOURCE defined

### RESOLVED: Libtool Shared Library Generation

**Status**: SOLVED - Post-configure sed commands

**Problem**: Libtool detects IRIX as "unknown" platform and refuses to build shared libraries.

**Solution**: Add these sed commands after %configure in spec files:
```bash
sed -i 's/build_libtool_libs=no/build_libtool_libs=yes/g' libtool
sed -i 's/deplibs_check_method="unknown"/deplibs_check_method="pass_all"/g' libtool
sed -i 's/^version_type=none$/version_type=linux/g' libtool
sed -i 's/^soname_spec=""$/soname_spec="\\$libname\\${shared_ext}\\$major"/g' libtool
sed -i 's/^library_names_spec=""$/library_names_spec="\\$libname\\${shared_ext}\\$versuffix \\$libname\\${shared_ext}\\$major \\$libname\\${shared_ext}"/g' libtool
```

### RESOLVED: IRIX Feature Macro Chain

**Status**: DOCUMENTED - Use `_XOPEN_SOURCE=600` for snprintf/vsnprintf

**Problem**: Many packages define `_POSIX_SOURCE` or `_POSIX_C_SOURCE`, which breaks IRIX's `_SGIAPI` macro chain and hides functions like snprintf/vsnprintf.

**Solution**: Add `-D_XOPEN_SOURCE=600` to CFLAGS. This enables `_XOPEN5` which exposes snprintf/vsnprintf regardless of POSIX macros.

**Background**: IRIX `<standards.h>` uses:
- `_NO_POSIX` = true only if neither `_POSIX_SOURCE` nor `_POSIX_C_SOURCE` defined
- `_SGIAPI` requires `_NO_POSIX` to be true
- `_XOPEN5` = `_XOPEN_SOURCE >= 500`
- snprintf/vsnprintf require: `defined(__c99) || ((_XOPEN5 || _SGIAPI) && _NO_ANSIMODE)`

### RESOLVED: Clang __mips64 for N32 ABI

**Status**: SOLVED - Undefine in irix-cc wrapper

**Problem**: Clang defines `__mips64=1` even for N32 ABI (which is 32-bit). This breaks OpenSSL's multiarch `configuration.h` which checks `__mips64` before `__mips`.

**Solution**: Add `-U__mips64 -U__mips64__` to CLANG_FLAGS in irix-cc wrapper.

---

# Claude.md Refactoring Strategy

## Background: AGENTS.md Concept (Vercel)

Based on Vercel's research (https://vercel.com/blog/agents-md-outperforms-skills-in-our-agent-evals):

**Key insight**: Passive context (delivered every turn) outperforms active tools (skills that agents must decide to invoke). In Vercel's evals, AGENTS.md achieved 100% pass rate vs 53% baseline and 79% with skills.

**Why it works**:
- Eliminates "decision paralysis" - agents don't have to decide whether to invoke a tool
- Compressed index structure reduces tokens while maintaining coverage
- Explicit directive to prefer retrieval over pre-trained knowledge

**Implementation pattern**:
- Pipe-delimited compressed format (reduced 40KB → 8KB)
- Index structure pointing to files rather than full content inline
- Directive encouraging "retrieval-led reasoning over pre-training-led reasoning"

---

## Current State: mogrix Documentation

**Existing files**:
| File | Size | Purpose |
|------|------|---------|
| `Claude.md` | ~15KB | AI agent instructions, detailed explanations |
| `HANDOFF.md` | ~20KB | Session continuity, current issues |
| `README.md` | ~5KB | Basic user documentation |
| `BOOTSTRAP.md` | ~3KB | Bootstrap status |

**Problem**: Claude.md is detailed but not optimized for passive context injection. It duplicates information and lacks the compressed index format that Vercel found effective.

---

## Claude.md Structure (IMPLEMENTED)

### Section 1: Project Identity (5 lines)
```
# mogrix - Cross-compile Fedora SRPMs for IRIX 6.5
Prefer retrieval from project files over pre-trained knowledge.
When uncertain about IRIX/mogrix patterns, READ the files below.
Pre-trained info about IRIX/MIPS/cross-compilation is likely outdated.
```

### Section 2: Critical Rules (10 lines)
```
## CRITICAL
- NEVER fix outside mogrix rules|every fix stored in rules/packages/*.yaml
- NEVER install to /usr/sgug directly|use /opt/chroot first
- NEVER build from upstream tarballs|use mogrix fetch→convert→build workflow
- Test with: ssh edodd@192.168.0.81|run /usr/sgug/bin/sgugshell first
```

### Section 3: File Index (pipe-delimited, ~40 lines)
```
## Rules Index
rules/generic.yaml|universal rules: skip_check, drop systemd/selinux, path rewrites
rules/packages/*.yaml|per-package rules (64 files): configure_opts|spec_replacements|inject_compat_functions

## Compat Index
compat/catalog.yaml|function registry: strdup|getline|openat|funopen|25+ functions
compat/string/|strdup.c|strndup.c|strcasestr.c|strsep.c
compat/stdio/|getline.c|asprintf.c|funopen.c (BSD cookie I/O)
compat/dicl/|openat-compat.c (17 POSIX "at" functions)|futimens|utimensat
compat/include/mogrix-compat/generic/|stdio.h|stdlib.h|string.h|fcntl.h|spawn.h

## Headers Index
headers/generic/|stdint.h|stdbool.h|stdarg.h|stdlib.h|string.h|ctype.h
cross/include/dicl-clang-compat/|va_list fix|XPG socket.h|complex.h standalone

## Key Patterns (READ these for context)
HANDOFF.md|current issues, session continuity, test commands
rules/packages/rpm.yaml|complex cmake package example
rules/packages/curl.yaml|complex autoconf package example
rules/packages/popt.yaml|simple package example
```

### Section 4: Workflow Reference (10 lines)
```
## Workflow
fetch: mogrix fetch <pkg>
convert: mogrix convert workdir/<pkg>-*.src.rpm
build: rpmbuild --rebuild <converted>.src.rpm --define "__cc /opt/sgug-staging/usr/sgug/bin/irix-cc"
stage: mogrix stage ~/rpmbuild/RPMS/mips/<pkg>*.rpm
test: scp to IRIX|LD_LIBRARYN32_PATH=/opt/chroot/usr/sgug/lib32 /opt/chroot/usr/sgug/bin/<cmd>
```

### Section 5: Common Issues Reference (15 lines)
```
## When Build Fails
missing function → compat/catalog.yaml + compat/<category>/<func>.c
header conflict → headers/generic/ or cross/include/dicl-clang-compat/
configure wrong → rules/packages/<pkg>.yaml: ac_cv_overrides
linking fails → check LD export, --whole-archive for static libs
spec syntax → spec_replacements for shell/macro compatibility

## IRIX Quirks (retrieval > pre-training)
vsnprintf(NULL,0) returns -1 not size → iterative vasprintf in compat/stdio/asprintf.c
no /dev/fd → futimens broken → current issue in HANDOFF.md
uname -m returns IP30/IP32 → hardcode mips in tdnf
va_list conflict → force-include stdarg.h before IRIX headers
```

### Section 6: IRIX Interaction Rules (20 lines)
```
## IRIX Test Host
ssh root@192.168.0.81|chroot at /opt/chroot|SGUG-RSE at /usr/sgug

## Shell Rules
default shell is csh|always use /bin/sh for scripts|no bashisms (pushd/popd/[[/brace expansion)
LD_LIBRARYN32_PATH not LD_LIBRARY_PATH|sgug-exec sets up environment|sgugshell for interactive

## Chroot Behavior (IRIX chroot does NOT fully isolate)
/opt/chroot/tmp ≠ /tmp inside chroot|binaries see base system paths|need symlinks on base system
copy files to /opt/chroot/tmp before accessing|use absolute paths always

## Running Binaries
sgug-exec: /usr/sgug/bin/sgug-exec /usr/sgug/bin/CMD
manual: LD_LIBRARYN32_PATH=/opt/chroot/usr/sgug/lib32 /opt/chroot/usr/sgug/bin/CMD
interactive: chroot /opt/chroot /bin/sh → export LD_LIBRARYN32_PATH=/usr/sgug/lib32

## NEVER Do
install to /usr/sgug directly (breaks sshd)|replace libs running services use|assume bash syntax
run rpmbuild on IRIX (build on Linux)|use relative paths in chroot|forget LD_LIBRARYN32_PATH
```

---

## Rule Structure Decision: Keep Per-Package Files

### Rejected: Individual Numbered Rule Files

A suggestion was considered to refactor rules into individual numbered files:
```
rules/individual/
├── R001_drop_systemd_global.yaml
├── R002_drop_selinux_global.yaml
...
├── R412_tdnf_hardcode_mips_arch.yaml
```

**This was rejected because:**

| Concern | Current Structure | Proposed Structure |
|---------|------------------|-------------------|
| File count | ~65 files | 500-1000+ files |
| Understanding a package | Read 1 file | Grep across hundreds |
| Adding a fix | Edit one file | Create new numbered file |
| Cohesion | All rpm knowledge in rpm.yaml | Scattered across R087-R156 |
| AI comprehension | "Read rpm.yaml" | "Search for package=rpm" |

**The explosion problem:** `tdnf.yaml` alone has ~50 spec_replacements. Each would become a separate file. One package = 60+ files. 64 packages = thousands of files.

**When individual rules make sense:**
- Rule inheritance/composition across packages
- UI-based rule management
- Fine-grained audit requirements
- Rules frequently reused across packages

**Why mogrix doesn't need this:**
- Rules are mostly package-specific (not shared)
- No UI, just YAML files
- Git provides versioning
- Same spec_replacement rarely applies to multiple packages

### Approved: Metadata Headers + Index File

Instead of restructuring, add indexing metadata to existing files:

```yaml
# rules/packages/rpm.yaml
# ---metadata---
# package: rpm
# complexity: high
# build_system: cmake
# key_fixes: [vsnprintf, spawn, TLS_removal, sqlite_db]
# similar_to: [libsolv, tdnf]
# ---
```

Plus create `rules/INDEX.md` with one-line summaries for AI retrieval.

---

## Implementation Steps

### Phase 1: Add Metadata Headers to Package Rules - DONE
Added YAML comment headers to `rules/packages/*.yaml` with:
- complexity (low/medium/high)
- build_system (autoconf/cmake/make/custom)
- key_fixes (list of main issues solved)
- similar_to (related packages for reference)
- depends, status

Completed for: tdnf, libsolv, rpm, popt, curl, openssl, sqlite, file, lua, xz, libxml2, zlib-ng, bzip2

### Phase 2: Create rules/INDEX.md - DONE
Created comprehensive index with:
- tdnf dependency chain table
- By build system categorization
- By compat functions needed
- Common patterns with examples
- "When adding new packages" guide

### Phase 3: Refactor Claude.md - DONE
Refactored (not minified) to:
- Keep critical philosophy sections (Cardinal Sin, Golden Rule, No Shortcuts)
- Add retrieval directive at top
- Add file index table pointing to INDEX.md, catalog.yaml, HANDOFF.md
- Consolidate IRIX interaction rules
- Remove duplicated package info (now in INDEX.md)

### Phase 4: Enhance Catalog Index - DONE
Added index header to `compat/catalog.yaml` with:
- Category listing (string, stdio, stdlib, runtime, dicl, etc.)
- IMPORTANT warnings about fopencookie (crashes) and vasprintf (iterative)

### Phase 5: Test with Fresh Context - TODO
Start new Claude sessions and verify:
- Agent correctly uses retrieval over pre-training
- Agent finds correct files for common issues
- Agent doesn't repeat solved problems

---

## Expected Benefits

1. **Faster context loading** - 80 lines vs 400+ lines
2. **Better retrieval behavior** - explicit directive + file pointers
3. **Reduced hallucination** - pre-training override for IRIX-specific patterns
4. **Easier maintenance** - index structure vs duplicated content

---

## Parallel Track: tdnf Install Issue (Paused)

**Status**: tdnf list/makecache work, but `tdnf install` fails with Error(1525)

**Root cause**: `utimets("/dev/fd/14", ...)` fails with ENOENT during rpm transaction

**Current investigation**:
- IRIX has /dev/fd mounted and working (tested with `exec 5<file; ls /dev/fd`)
- Need to test if IRIX's `utimes()` syscall works with /dev/fd/N paths
- Test program in HANDOFF.md ready to compile and run

**Resume with**: Run the utimes /dev/fd test on IRIX, then fix futimens() based on results

---

# IRIX Interaction Rules

## Personal Setup (edodd)

| Property | Value |
|----------|-------|
| IRIX hostname | `blue` |
| IRIX IP | `192.168.0.81` |
| SSH access | `ssh edodd@192.168.0.81` or `ssh root@192.168.0.81` |
| IRIX version | 6.5.30 |
| Machine type | SGI Octane (IP30) |
| Chroot location | `/opt/chroot` |
| SGUG-RSE installed | Yes, at `/usr/sgug` (rpm 4.15.0) |

---

## Shell Environment

### IRIX Default Shell is csh
```
# When you SSH in, you're in csh (C shell)
# csh uses different syntax than bash/sh:

# csh syntax (WRONG for scripts):
setenv LD_LIBRARYN32_PATH /usr/sgug/lib32
set path = ($path /usr/sgug/bin)

# sh/bash syntax (use this in scripts):
export LD_LIBRARYN32_PATH=/usr/sgug/lib32
export PATH=$PATH:/usr/sgug/bin
```

### Always Use /bin/sh for Scripts
```bash
# CORRECT - explicitly invoke /bin/sh:
ssh root@192.168.0.81 "/bin/sh -c 'export FOO=bar; echo \$FOO'"

# CORRECT - write script with #!/bin/sh and run with /bin/sh:
ssh root@192.168.0.81 "/bin/sh /tmp/test.sh"

# WRONG - relies on login shell (csh):
ssh root@192.168.0.81 "export FOO=bar"  # csh doesn't understand export
```

### Linux Bashisms That Don't Work

| Bashism | Problem | POSIX Alternative |
|---------|---------|-------------------|
| `pushd/popd` | Not in /bin/sh | `(cd dir && cmd)` subshell |
| `$VAR{a,b}` | Brace expansion | `$VAR/a $VAR/b` explicit |
| `[[ ]]` | Bash test | `[ ]` POSIX test |
| `source file` | Bash builtin | `. file` |
| `function name()` | Bash syntax | `name()` without function keyword |
| `echo -e` | Bash echo | `printf` |
| `2>&1 \|` | Process substitution | Redirect to file first |
| `$(< file)` | Bash shortcut | `$(cat file)` |
| `&>` | Bash redirect | `>file 2>&1` |
| `<<<` | Here-string | `echo "str" \|` |

### IRIX /bin/sh Quirks
- No `head` or `tail` in minimal path - use full path `/usr/bin/head`
- `echo` behavior differs from GNU echo
- Some commands in `/usr/bin` not `/bin`
- `test` command more limited than bash `[[`

---

## The Chroot Environment

### What the Chroot Is
```
/opt/chroot/
├── usr/sgug/           # Our cross-compiled packages
│   ├── bin/            # tdnf, rpm, etc.
│   ├── lib32/          # Shared libraries
│   └── etc/            # Config files
├── etc/
│   ├── tdnf/           # tdnf configuration
│   └── yum.repos.d/    # Repository definitions
├── var/
│   ├── cache/tdnf/     # Package cache
│   └── lib/rpm -> ...  # RPM database symlink
└── tmp/                # Writable temp space
```

### CRITICAL: IRIX chroot Does NOT Fully Isolate

**Unlike Linux, IRIX chroot does not create a proper isolated environment.**

When you run `chroot /opt/chroot /bin/sh`:
- The process CAN still see `/etc`, `/usr`, etc. from the base system
- Library loading (`rld`) still searches base system paths
- Symlinks that point outside chroot are followed to base system

**This means:**
- Files at `/opt/chroot/tmp/foo.txt` are NOT accessible as `/tmp/foo.txt` from inside chroot
- Must use full paths or relative paths from chroot root
- Some symlinks on base system are needed (see below)

### Required Symlinks on Base IRIX System

Because chroot doesn't isolate, these symlinks on the BASE system are needed:
```bash
# On base IRIX (not in chroot):
mkdir -p /etc/yum.repos.d
mkdir -p /etc/tdnf/pluginconf.d
ln -sf /opt/chroot/usr/sgug/lib32/rpm /usr/sgug/lib32/rpm
```

### Accessing Files in Chroot

```bash
# WRONG - /tmp inside chroot is NOT /opt/chroot/tmp:
chroot /opt/chroot /bin/sh -c 'cat /tmp/test.txt'  # Fails or reads wrong file

# CORRECT - copy files to a location visible inside chroot:
cp /tmp/test.txt /opt/chroot/tmp/test.txt
chroot /opt/chroot /bin/sh -c 'cat /tmp/test.txt'  # Now works

# OR use full path from outside:
cat /opt/chroot/tmp/test.txt  # Always works from base system
```

---

## Using sgug-exec

### What sgug-exec Does
`sgug-exec` is a wrapper that sets up the SGUG-RSE environment:
- Sets `LD_LIBRARYN32_PATH` to include `/usr/sgug/lib32`
- Sets `PATH` to include `/usr/sgug/bin`
- Ensures SGUG libraries are found before system libraries

### Running SGUG Binaries - Three Methods

**Method 1: sgug-exec (Recommended)**
```bash
# Inside chroot:
/usr/sgug/bin/sgug-exec /usr/sgug/bin/tdnf --version

# From base system (running chroot binary):
/opt/chroot/usr/sgug/bin/sgug-exec /opt/chroot/usr/sgug/bin/tdnf --version
```

**Method 2: Manual LD_LIBRARYN32_PATH**
```bash
# For chroot binaries from base system:
export LD_LIBRARYN32_PATH=/opt/chroot/usr/sgug/lib32:/usr/sgug/lib32
/opt/chroot/usr/sgug/bin/tdnf --version

# Inside chroot:
export LD_LIBRARYN32_PATH=/usr/sgug/lib32
/usr/sgug/bin/tdnf --version
```

**Method 3: sgugshell (Interactive)**
```bash
# Start interactive SGUG shell (sets up environment):
/usr/sgug/bin/sgugshell

# Now all SGUG commands work without prefixes:
tdnf --version
rpm -qa
```

### LD_LIBRARYN32_PATH vs LD_LIBRARY_PATH

| Variable | Purpose |
|----------|---------|
| `LD_LIBRARYN32_PATH` | **Use this** - IRIX n32 ABI library path |
| `LD_LIBRARY_PATH` | Old 32-bit ABI - not for SGUG |
| `LD_LIBRARY64_PATH` | 64-bit ABI - not for SGUG |

SGUG-RSE uses n32 ABI (32-bit pointers, 64-bit registers), so always use `LD_LIBRARYN32_PATH`.

---

## Testing Patterns

### Pattern 1: Quick Command Test
```bash
# From Linux, run single command on IRIX:
ssh root@192.168.0.81 "/bin/sh -c 'LD_LIBRARYN32_PATH=/opt/chroot/usr/sgug/lib32 /opt/chroot/usr/sgug/bin/rpm --version'"
```

### Pattern 2: Script-Based Test
```bash
# Create test script locally:
cat > /tmp/test.sh << 'EOF'
#!/bin/sh
export LD_LIBRARYN32_PATH=/opt/chroot/usr/sgug/lib32:/usr/sgug/lib32
/opt/chroot/usr/sgug/bin/tdnf -c /opt/chroot/etc/tdnf/tdnf.conf --installroot=/opt/chroot repolist
echo "Exit code: $?"
EOF

# Copy and run:
scp /tmp/test.sh root@192.168.0.81:/tmp/
ssh root@192.168.0.81 "/bin/sh /tmp/test.sh"
```

### Pattern 3: Interactive Chroot Session
```bash
# SSH to IRIX:
ssh root@192.168.0.81

# Enter chroot with proper shell:
chroot /opt/chroot /bin/sh

# Set up environment:
export LD_LIBRARYN32_PATH=/usr/sgug/lib32
export PATH=/usr/sgug/bin:$PATH

# Now run commands:
rpm --version
tdnf repolist
```

### Pattern 4: Using sgug-exec in Chroot
```bash
# SSH and chroot:
ssh root@192.168.0.81
chroot /opt/chroot /bin/sh

# Use sgug-exec (handles LD_LIBRARYN32_PATH):
/usr/sgug/bin/sgug-exec /usr/sgug/bin/tdnf -c /etc/tdnf/tdnf.conf --installroot=/ repolist
```

### Pattern 5: Debugging with par
```bash
# par is IRIX's strace equivalent:
par /usr/sgug/bin/sgug-exec /usr/sgug/bin/tdnf repolist > /tmp/par_output.txt 2>&1

# Copy trace to Linux for analysis:
scp root@192.168.0.81:/opt/chroot/tmp/par_output.txt /tmp/
```

---

## Things to NEVER Do

### NEVER Install Directly to /usr/sgug
```bash
# DANGEROUS - can break sshd, lock you out:
rpm -Uvh --root=/ package.rpm  # NO!
rpm -ivh /tmp/package.rpm      # NO! (if it touches /usr/sgug)

# SAFE - always test in chroot first:
rpm --root=/opt/chroot -ivh package.rpm  # YES
```

### NEVER Assume Linux Shell Behavior
```bash
# WRONG - bash array syntax:
files=(*.rpm)

# WRONG - bash process substitution:
diff <(cmd1) <(cmd2)

# WRONG - bash [[ ]] test:
[[ -f file ]]

# CORRECT - POSIX:
for f in *.rpm; do ...; done
[ -f file ]
```

### NEVER Use Relative Paths in Chroot Commands
```bash
# WRONG - relative path may resolve to base system:
chroot /opt/chroot /bin/sh -c 'cd tmp && ls'

# CORRECT - absolute paths:
chroot /opt/chroot /bin/sh -c 'cd /tmp && ls'
```

### NEVER Forget LD_LIBRARYN32_PATH
```bash
# WRONG - will fail to find libraries:
/opt/chroot/usr/sgug/bin/tdnf --version

# CORRECT - set library path:
LD_LIBRARYN32_PATH=/opt/chroot/usr/sgug/lib32 /opt/chroot/usr/sgug/bin/tdnf --version

# OR use sgug-exec:
/opt/chroot/usr/sgug/bin/sgug-exec /opt/chroot/usr/sgug/bin/tdnf --version
```

### NEVER Replace Libraries That Running Services Use
```bash
# DANGEROUS - sshd uses libz, libssl, libcrypto:
rpm -Uvh zlib-ng-compat-*.rpm  # Can break sshd!

# Recovery requires console access:
# cp /usr/sgug_old/lib32/libz.so.1.2.11 /usr/sgug/lib32/
# ln -sf libz.so.1.2.11 /usr/sgug/lib32/libz.so.1
```

### NEVER Run rpmbuild on IRIX
```bash
# WRONG - rpmbuild is for the Linux build host:
ssh irix "rpmbuild --rebuild package.src.rpm"  # NO!

# CORRECT - build on Linux, copy RPMs to IRIX:
rpmbuild --rebuild package.src.rpm  # On Linux
scp ~/rpmbuild/RPMS/mips/*.rpm irix:/tmp/
ssh irix "rpm -ivh /tmp/*.rpm"  # Install on IRIX
```

---

## Quick Reference Card

```
# Connect to IRIX:
ssh root@192.168.0.81

# Enter SGUG shell (sets up environment):
/usr/sgug/bin/sgugshell

# Enter chroot:
chroot /opt/chroot /bin/sh
export LD_LIBRARYN32_PATH=/usr/sgug/lib32

# Run chroot binary from base system:
LD_LIBRARYN32_PATH=/opt/chroot/usr/sgug/lib32 /opt/chroot/usr/sgug/bin/CMD

# Copy file accessible in chroot:
scp file root@192.168.0.81:/opt/chroot/tmp/

# Trace system calls:
par CMD > /tmp/trace.txt 2>&1

# Check library dependencies:
elfdump -Dl /path/to/binary
```

---

# IRIX Bootstrap Strategy

## CRITICAL WARNING - READ FIRST

**NEVER install packages directly to /usr/sgug on the live IRIX system.**

On 2026-01-27, an attempt to install packages directly to /usr/sgug replaced libz.so with zlib-ng-compat, which broke sshd and locked out SSH access. Recovery required console access and manually restoring libz.so.1.2.11 from /usr/sgug_old.

**The chroot exists for a reason.** Always:
1. Test in /opt/chroot first
2. Get explicit user approval before touching /usr/sgug
3. Ensure console access is available as backup
4. Never replace libraries that running services depend on

---

## Bootstrap Status (2026-01-27)

**Chroot testing: SUCCESSFUL**
- rpm 4.19.1.1 works in /opt/chroot
- tdnf 3.5.14 works in /opt/chroot
- curl 8.6.0, openssl 3.2.1 all work
- All shared library dependencies resolve correctly

**Production install: NOT DONE - REQUIRES CAREFUL PLANNING**
- IRIX already has SGUG-RSE with rpm 4.15.0
- Our packages conflict with existing SGUG packages
- Need migration strategy before touching /usr/sgug
