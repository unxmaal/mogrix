# Mogrix: SRPM Conversion Engine for IRIX

Mogrix is a deterministic SRPM-to-RSE-SRPM conversion engine that transforms Fedora SRPMs into IRIX-compatible packages. It centralizes all platform knowledge required to adapt Linux build intent for IRIX reality.

## Current Status (2026-01-24)

**Phase 6.5: COMPLETE - FULL DEPENDENCY CHAIN VALIDATED**

All 12 packages in the tdnf dependency chain built successfully. Target achieved.

### Validated Package Chain

| # | Package | Version | Status | Notes |
|---|---------|---------|--------|-------|
| 1 | zlib | 1.2.13 | DONE | Compression library |
| 2 | bzip2 | 1.0.8 | DONE | Compression library |
| 3 | popt | 1.19 | DONE | Option parsing |
| 4 | openssl | 3.1.1 | DONE | TLS/SSL |
| 5 | libxml2 | 2.10.4 | DONE | XML parsing |
| 6 | curl | 8.2.1 | DONE | HTTP/HTTPS |
| 7 | xz | 5.4.6 | DONE | Compression library |
| 8 | lua | 5.4.6 | DONE | Scripting (rpm) |
| 9 | file | 5.45 | DONE | libmagic |
| 10 | rpm | 4.19.1.1 | DONE | Package manager core |
| 11 | libsolv | 0.7.28 | DONE | Dependency solver |
| 12 | **tdnf** | **3.5.14** | **DONE** | **TARGET ACHIEVED** |

All packages produce valid **ELF 32-bit MSB, MIPS N32** binaries for IRIX 6.5.

### tdnf 3.5.14 Build Complete (TARGET ACHIEVED)

**Built RPMs (all .mips.rpm):**
- tdnf-3.5.14-1.mips.rpm - Main package manager
- tdnf-cli-libs-3.5.14-1.mips.rpm - CLI library
- tdnf-devel-3.5.14-1.mips.rpm - Development headers

**Verified MIPS N32 binary:**
```
ELF 32-bit MSB executable, MIPS, N32 MIPS-III version 1 (SYSV),
dynamically linked, interpreter /lib32/rld
```

### Key Compat Functions Implemented

| Function | Package | File | Status |
|----------|---------|------|--------|
| POSIX.1-2008 "at" funcs | rpm, libsolv | openat-compat.c | DONE |
| getprogname/setprogname | rpm | getprogname.c | DONE |
| fopencookie | libsolv | fopencookie.c | DONE |
| getline | libsolv, tdnf | getline.c | DONE |
| strdup/strndup | file, xz, libsolv, tdnf | strdup.c, strndup.c | DONE |
| strcasestr | libsolv | strcasestr.c | DONE |
| strsep | tdnf | strsep.c | DONE |
| timegm | libsolv | timegm.c | DONE |
| mkdtemp | libsolv | mkdtemp.c | DONE |
| qsort_r | libsolv, tdnf | qsort_r.c | DONE |
| asprintf/vasprintf | tdnf | asprintf.c | DONE |
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

### Cross-Compilation Validated on IRIX

| Package | Type | Result |
|---------|------|--------|
| tree | executable | Works |
| bash | executable | Works |
| coreutils | executables | Works |
| tar, gzip, sed, grep | executables | Works |
| make, patch | executables | Works |
| zlib, bzip2, xz | static/shared libs | Works |
| ncurses, readline | static libs | Works |
| popt | shared lib | Works (dlopen validated) |
| openssl | shared lib | Works (dlopen with RTLD_LAZY) |
| curl | dynamically linked exe | Works (HTTP/HTTPS) |
| libsolv | shared lib | Works |
| libsolvext | shared lib | Works (with fopencookie) |
| **dumpsolv** | **dynamically linked exe** | **Works** |
| **tdnf** | **dynamically linked exe** | **Works (needs config)** |

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

### Confidence Assessment: ~95%

A clean rebuild from scratch should succeed. All critical knowledge gaps have been fixed:

| Fix | Status |
|-----|--------|
| sys/stat.h wrapper | DONE - in `cross/include/dicl-clang-compat/sys/stat.h` |
| `%systemd_*` scriptlets | DONE - in `generic.yaml` remove_lines |
| RPM architecture naming | DONE - `--target mips-sgi-irix` added |
| -devel package staging | DONE - auto-included by `mogrix stage` |
| Multiarch headers | DONE - auto-created during staging |

### Remaining Work

#### 1. Test on IRIX Hardware

- Copy all MIPS RPMs to IRIX
- Install and verify tdnf runs
- Test basic package operations

#### 2. Create Bootstrap Repository

- Host all built RPMs
- Configure tdnf.conf for IRIX

#### 3. Port gpgcheck.c to rpm 4.19 API (Lower Priority)

The gpgcheck.c is stubbed because it uses OLD rpm PGP API:

| Old API (tdnf uses) | New API (rpm 4.19) |
|---------------------|-------------------|
| `pgpDig` type | `pgpDigParams` |
| `pgpNewDig()` | (removed) |
| `pgpFreeDig()` | `pgpDigParamsFree()` |
| `pgpPrtPkts()` | `pgpPrtParams()` |

**Impact of stub:** Package signature verification disabled. Security implications for production.

**Fix:** Create patched gpgcheck.c using new API, add to mogrix patches.

#### 4. Future Improvements (Lower Priority)

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
