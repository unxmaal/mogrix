# Mogrix: SRPM Conversion Engine for IRIX

Mogrix is a deterministic SRPM-to-RSE-SRPM conversion engine that transforms Fedora SRPMs into IRIX-compatible packages. It centralizes all platform knowledge required to adapt Linux build intent for IRIX reality.

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

## Current Status

**Phase 3 Complete** - Ready for Phase 4 (Full tdnf Stack)

### What Works Now

| Feature | Status | Location |
|---------|--------|----------|
| Spec file parsing | Done | `mogrix/parser/spec.py` |
| YAML rule loading | Done | `mogrix/rules/loader.py` |
| Rule application engine | Done | `mogrix/rules/engine.py` |
| Header overlay system | Done | `mogrix/headers/overlay.py` |
| Generic clang_compat headers | Done | `headers/generic/` (16 headers) |
| Compat source injection | Done | `mogrix/compat/injector.py` |
| Spec file rewriting | Done | `mogrix/emitter/spec.py` |
| CLI (analyze, convert) | Done | `mogrix/cli.py` |
| Patch catalog | Done | `mogrix/patches/catalog.py` |
| Configure flag manipulation | Done | add/remove flags |
| Conditional handling | Done | %if/%endif blocks |
| Subpackage management | Done | drop debuginfo, langpacks |

### Test Coverage

- **59 tests**, all passing
- Covers: parser, rules, engine, emitter, headers, compat, patches, CLI

### Package Rules Created

| Package | Compat Functions | Notes |
|---------|-----------------|-------|
| popt | strdup, strndup | First LIBDICL package |
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

### Usage

```bash
# Activate virtual environment
source .venv/bin/activate

# Analyze a spec file (show what rules apply)
mogrix analyze path/to/package.spec

# Convert a spec file
mogrix convert path/to/package.spec -o output.spec

# Run tests
make test

# Format and lint
make format
make lint
```

---

## Target Package Set: tdnf and Dependencies

**Strategic goal:** Build `tdnf` (Tiny Dandified Yum) — a lightweight package manager. With tdnf working on IRIX, users can install additional packages easily, dramatically improving adoption and usability.

**Analysis:** `./analysis/analyze_tdnf_deps.py` → `./analysis/tdnf_deps.json`

### Summary

| Metric | Count |
|--------|-------|
| Total packages to build | 123 |
| Packages with sgifixes patches | 45 (37%) |
| Packages needing libdicl compat | 26 (21%) |

### Build Order (dependencies first)

The following packages must be built in order. Packages are listed with their key dependencies and flags indicating special handling needed:

- **SGIFIXES** — Has existing IRIX patches in SGUG-RSE
- **LIBDICL** — Needs compat sources injected by mogrix

#### Tier 1: No dependencies (build first)
1. byacc, bzip2, chrpath [LIBDICL], docbook-style-dsssl, docbook-style-xsl
2. expect [SGIFIXES], giflib, gperf, help2man, initial-sgug
3. libimagequant, libunistring, lzip, npth
4. perl-Carp, perl-SGMLSpm, perl-podlators, pixman [SGIFIXES]
5. pkgconfig, python-rpm-macros, sgug-rpm-config, symlinks [SGIFIXES]
6. xorg-x11-util-macros, xz

#### Tier 2: Basic build tools
7. ncurses ← pkgconfig
8. readline ← ncurses
9. libidn2 ← libunistring
10. unzip ← bzip2
11. docbook-dtds ← unzip
12. imake ← pkgconfig, xorg-x11-util-macros

#### Tier 3: Core libraries
13. zlib ← autoconf, automake, libtool
14. openssl [SGIFIXES] ← perl-podlators, zlib
15. popt [LIBDICL] ← libdicl
16. expat ← autoconf, libtool
17. libffi [SGIFIXES] ← dejagnu
18. libgpg-error [LIBDICL] ← autoconf, automake, libtool, texinfo
19. libxml2 [SGIFIXES] ← python3, xz, zlib

#### Tier 4: Development infrastructure
20. autoconf ← emacs, help2man, m4
21. automake [SGIFIXES] ← autoconf, bison, dejagnu, ...
22. libtool [SGIFIXES] ← autoconf, automake, help2man, m4
23. m4 [SGIFIXES] ← autoconf, automake, texinfo
24. bison ← autoconf, flex, m4
25. flex [SGIFIXES] ← automake, bison, help2man, libtool

#### Tier 5: Security & crypto
26. libgcrypt ← libgpg-error, pkgconfig, texinfo
27. libassuan [SGIFIXES, LIBDICL] ← libgpg-error
28. libksba ← libgcrypt, libgpg-error
29. nettle ← autoconf, automake, gmp, libtool
30. libtasn1 [SGIFIXES, LIBDICL] ← autoconf, automake, bison, help2man
31. p11-kit [SGIFIXES, LIBDICL] ← libdicl, libffi, libtasn1
32. gnutls [LIBDICL] ← autoconf, automake, ca-certificates, gperf, ...
33. gpgme [SGIFIXES, LIBDICL] ← gnupg2, libassuan, libdicl, libgpg-error
34. gnupg2 [SGIFIXES, LIBDICL] ← bzip2, curl, docbook-utils, gnutls, ...

#### Tier 6: Python and scripting
35. tcl [SGIFIXES] ← zlib
36. tk [SGIFIXES] ← autoconf, libX11, libXft, tcl
37. python3 [SGIFIXES, LIBDICL] ← autoconf, bzip2, expat, libffi, openssl, ...
38. lua [SGIFIXES] ← autoconf, automake, libtool, ncurses

#### Tier 7: Graphics stack (for dependencies)
39. freetype [SGIFIXES] ← bzip2, libpng, zlib
40. fontconfig [LIBDICL] ← autoconf, automake, docbook-utils, expat, freetype
41. cairo [SGIFIXES, LIBDICL] ← fontconfig, freetype, glib2, libX11, ...
42. harfbuzz [SGIFIXES] ← cairo, freetype, glib2, gobject-introspection
43. pango [SGIFIXES] ← cairo, fontconfig, fribidi, glib2

#### Tier 8: GLib ecosystem
44. glib2 [SGIFIXES, LIBDICL] ← elfutils, libdicl, libffi, pcre, python3
45. gobject-introspection [SGIFIXES, LIBDICL] ← bison, flex, glib2, python3

#### Tier 9: Package management core
46. elfutils [SGIFIXES, LIBDICL] ← bison, bzip2, flex, libdicl, zlib
47. file [LIBDICL] ← autoconf, automake, libdicl, libtool, python3, zlib
48. libarchive [SGIFIXES] ← automake, bison, bzip2, libxml2, xz, zlib
49. zchunk [SGIFIXES, LIBDICL] ← openssl
50. rpm [SGIFIXES, LIBDICL] ← automake, bzip2, elfutils, file, ...
51. libsolv [SGIFIXES, LIBDICL] ← bzip2, libdicl, libxml2, python3, rpm, xz, zlib
52. libmetalink ← CUnit, expat
53. curl ← automake, groff, openssl, pkgconfig, zlib

#### Tier 10: Target
54. **tdnf** [SGIFIXES, LIBDICL] ← curl, gpgme, libmetalink, libsolv, openssl, rpm, zchunk

### Packages Needing Compat Source Injection (26)

These packages previously required `libdicl-devel`. Mogrix must inject compat sources:

```
cairo, chrpath, elfutils, file, fontconfig, fribidi, gd, gcr, glib2,
gnupg2, gobject-introspection, gpgme, gnutls, libassuan, libdicl,
libgpg-error, libsolv, libtasn1, p11-kit, pcre, popt, python3, rpm,
tdnf, xmlto, zchunk
```

### Packages with Existing sgifixes Patches (45)

These packages have IRIX-specific patches in SGUG-RSE that should be incorporated:

```
automake, binutils, cairo, dejagnu, elfutils, emacs, expect, flex,
freetype, gcr, gd, git, glib2, gnupg2, gobject-introspection, gpgme,
groff, gtk2, harfbuzz, libarchive, libassuan, libdb, libffi,
libjpeg-turbo, libsecret, libsolv, libtasn1, libtool, libxml2, lua,
m4, openjade, openssl, p11-kit, pango, pixman, python3, rpm, sqlite,
symlinks, tcl, tdnf, tk, uuid, zchunk
```

---

## Architecture Overview

```
FC40 SRPM
   │
   │  (extract)
   ▼
┌──────────────────────────────────────────────────────────────────┐
│                           MOGRIX                                  │
│                                                                   │
│  ┌─────────────┐    ┌─────────────┐    ┌─────────────┐          │
│  │   Parser    │───▶│ Rule Engine │───▶│   Emitter   │          │
│  │ (spec+src)  │    │             │    │ (SRPM+report)│          │
│  └─────────────┘    └─────────────┘    └─────────────┘          │
│                            │                                      │
│            ┌───────────────┼───────────────┐                     │
│            ▼               ▼               ▼                     │
│     ┌──────────┐    ┌──────────┐    ┌──────────┐                │
│     │  Rules   │    │  Header  │    │  Compat  │                │
│     │ (YAML)   │    │ Overlays │    │ Sources  │                │
│     └──────────┘    └──────────┘    └──────────┘                │
│                      (stdarg.h,      (strdup.c,                  │
│                       stdio.h...)     getline.c...)              │
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

**Key principle:** Each converted SRPM contains everything needed to build on IRIX. No external compatibility library required.

---

## Component Design

### 1. SRPM Parser

Extracts and parses the input SRPM:

- Extract SRPM contents (spec file, sources, patches)
- Parse spec file into AST-like structure:
  - Preamble (Name, Version, Release, BuildRequires, Requires)
  - Scriptlets (%prep, %build, %install, %check, %files)
  - Macros and conditionals
- Catalog source tarballs and existing patches
- Identify build system type (autotools, cmake, meson, make, etc.)

**Output:** Structured representation of the package

### 2. Rule Engine

The heart of Mogrix. Applies transformation rules in priority order:

#### Rule Categories (applied in order):

1. **Generic Rules** — Universal Linux→IRIX transformations
2. **Package-Class Rules** — Patterns for families of packages
3. **Package-Specific Rules** — Explicit overrides for individual packages

#### Rule Types:

| Rule Type | Description | Example |
|-----------|-------------|---------|
| `drop_requires` | Remove dependencies that don't exist/apply on IRIX | systemd, selinux, udev |
| `add_requires` | Add IRIX-specific dependencies | sgug-rpm-config |
| `rewrite_path` | Transform paths to SGUG layout | /usr/lib → /usr/sgug/lib32 |
| `inject_macro` | Add RPM macros for IRIX | %_prefix, toolchain vars |
| `disable_feature` | Turn off features via configure flags | --disable-selinux |
| `add_patch` | Apply IRIX-specific patches | from patch catalog |
| `set_env` | Set environment variables | CC, CFLAGS, etc. |
| `force_static` | Build static libraries only | compression libs |
| `drop_subpackage` | Remove subpackages that can't build | -debuginfo, -langpack |
| `rewrite_scriptlet` | Transform build scriptlet content | %configure args |
| `header_overlays` | Add replacement/wrapper headers | stdarg.h, stdio.h |
| `inject_compat_source` | Bundle compat .c files into package | strdup.c, getline.c |

### 3. Emitter

Produces the converted SRPM and a human-readable conversion report:

- Rewrite spec file with all transformations applied
- Copy/modify sources as needed
- Generate new patches if required
- Package into valid SRPM
- Emit detailed conversion report (for debugging and review)

---

## Empirical Analysis: SGUG-RSE Patterns

Analysis of 2700+ packages in the existing SGUG-RSE repository reveals the actual patterns needed to convert Fedora packages to IRIX. This data directly informs the mogrix rule engine.

**Source:** `/home/edodd/projects/github/sgug-rse/packages/`
**Analysis scripts:** `./analysis/analyze_sgifixes.py`, `./analysis/analyze_spec_diffs.py`

### Source Code Fixes (from 198 .sgifixes.patch files)

| Pattern | Frequency | Notes |
|---------|-----------|-------|
| `#if defined(__sgi)` guards | 40% | IRIX-specific code paths |
| Added `#include` statements | 40% | Missing headers |
| Include `time.h` before `sys/time.h` | 14% | IRIX header ordering dependency |
| Integer type fixes (int64_t, etc.) | 12% | POSIX type availability |
| pthread-related fixes | 7% | Threading differences |
| AIX compatibility patterns | 7% | IRIX shares AIX quirks |
| strdup/strndup fixes | 6% | Not in IRIX libc |
| clock_gettime fixes | 5% | Availability/usage |
| FPU handling fixes | 3% | MIPS floating point |
| visibility attribute fixes | 3% | Symbol visibility |

**Most frequently added includes:**
```
sys/time.h       (51 packages)
unistd.h         (50 packages)
stdlib.h         (40 packages)
sys/types.h      (37 packages)
time.h           (25 packages)
string.h         (24 packages)
stdio.h          (24 packages)
alloca.h         (17 packages)
bstring.h        (16 packages)  -- IRIX-specific
fcntl.h          (16 packages)
errno.h          (15 packages)
sys/stat.h       (14 packages)
```

### Spec File Modifications (from 854 spec pairs)

| Pattern | Frequency | Notes |
|---------|-----------|-------|
| Modified BuildRequires | 33% | Add/remove build deps |
| Removed BuildRequires | 32% | Remove unavailable deps |
| sgifixes patches added | 18% | IRIX-specific patches |
| Removed Requires | 15% | Runtime dep changes |
| LDFLAGS modified | 13% | Linker flag changes |
| `%{sgug_optimised_ldflags}` | 12% | SGUG macro usage |
| CFLAGS modified | 9% | Compiler flag changes |
| Configure --disable flags | 8% | Feature disable |
| /usr/sgug prefix usage | 7% | SGUG path prefix |
| Commented out arch patches | 5% | Remove x86/ARM patches |
| autoreconf added | 4% | Regenerate build system |
| systemd removal | 2% | Not on IRIX |
| SELinux removal | 2% | Not on IRIX |
| Subpackages commented | 2% | Remove unbuildable |

**Most frequently removed BuildRequires:**
```
doxygen                  (51)  -- often optional
gtk-doc                  (32)  -- documentation
glibc-common             (15)  -- Linux-specific
alsa-lib-devel           (12)  -- Linux audio
desktop-file-utils        (9)  -- freedesktop.org
gnupg2                    (8)  -- optional signing
systemd                   (8)  -- Linux init
libselinux-devel          (8)  -- Linux security
systemtap-sdt-devel       (7)  -- Linux tracing
libacl-devel              (7)  -- Linux ACLs
libcap-devel              (7)  -- Linux capabilities
krb5-devel                (7)  -- often optional
```

**Most frequently added BuildRequires:**
```
libdicl-devel            (16)  -- ABSORBED INTO MOGRIX (no longer a dependency)
gcc                       (9)  -- explicit compiler
glibc-devel               (7)  -- explicit libc
ncurses-devel             (5)  -- terminal handling
zlib-devel                (5)  -- compression
pkgconfig                 (5)  -- pkg-config tool
libffi-devel              (5)  -- FFI support
autoconf                  (5)  -- build system
```

**Note:** The 16 packages that needed `libdicl-devel` now get compat sources injected directly by mogrix.

**Most common --disable-* configure flags:**
```
--disable-static         (32)  -- prefer shared libs
--disable-rpath          (11)  -- SGUG handles rpath
--disable-silent-rules   (11)  -- verbose builds
--disable-gtk-doc         (7)  -- skip docs
--disable-dependency-tracking (5)
--disable-bootstrap       (3)  -- skip bootstrap
--disable-ldap            (3)  -- optional feature
--disable-cups            (3)  -- printing
--disable-examples        (3)  -- skip examples
```

**Most common --without-* configure flags:**
```
--without-gtk-doc         (4)
--without-dbus            (4)
--without-included-regex  (2)
--without-python          (2)
--without-brotli          (2)
--without-libmetalink     (2)
--without-libpsl          (2)
--without-ldap            (2)
```

### Key Insights for Mogrix Rules

1. **Compat functions needed by 16+ packages** — strdup, strndup, asprintf, getline, etc. (previously libdicl; now injected per-package)
2. **Documentation often skipped** — doxygen, gtk-doc frequently removed or disabled
3. **Linux-specific features universally disabled** — systemd, SELinux, ALSA, capabilities
4. **SGUG macros standardize flags** — 108 packages use `%{sgug_optimised_ldflags}`
5. **Header ordering matters** — `time.h` must precede `sys/time.h` on IRIX
6. **Missing libc functions** — strdup, strndup, asprintf, getline need compat source injection
7. **`__sgi` ifdef is the standard guard** — 40% of patches use this pattern
8. **Self-contained packages** — Each package bundles its own compat code, no shared library

---

## Rule Definitions

Rules are defined in YAML for readability and maintainability.

### Generic Rules (`rules/generic.yaml`)

```yaml
# Universal Linux → IRIX transformations
# Based on empirical analysis of 854 SGUG-RSE spec files

generic:
  # Dependencies to always drop (don't exist on IRIX)
  # Derived from most-removed BuildRequires in SGUG-RSE
  drop_buildrequires:
    - systemd
    - systemd-devel
    - systemd-libs
    - systemd-rpm-macros
    - selinux-policy
    - libselinux
    - libselinux-devel
    - libselinux-utils
    - udev
    - kernel-headers
    - glibc-langpack-*
    - glibc-all-langpacks
    - alsa-lib-devel           # Linux audio
    - pulseaudio-libs-devel    # Linux audio
    - libcap-devel             # Linux capabilities
    - libcap-ng-devel
    - libacl-devel             # Linux ACLs
    - libattr-devel            # Linux xattrs
    - audit-libs-devel         # Linux audit
    - systemtap-sdt-devel      # Linux tracing
    - pkgconfig(systemd)
    - pkgconfig(libselinux)
    - pkgconfig(libudev)
    - pkgconfig(libcap)
    - pkgconfig(libsystemd)

  # Optional: often removed but may be wanted
  drop_buildrequires_optional:
    - doxygen                  # Documentation (51 packages removed)
    - gtk-doc                  # Documentation (32 packages removed)
    - desktop-file-utils       # freedesktop.org
    - /usr/bin/appstream-util

  # Dependencies to always add
  add_buildrequires:
    - sgug-rpm-config          # SGUG RPM macros

  # NOTE: libdicl is NOT added as a dependency
  # Instead, mogrix injects compat headers and sources directly into each package
  # This makes packages self-contained with no external compat library needed

  # Path rewrites
  rewrite_paths:
    /usr/lib64: /usr/sgug/lib32
    /usr/lib: /usr/sgug/lib32
    /usr/include: /usr/sgug/include
    /usr/bin: /usr/sgug/bin
    /usr/share: /usr/sgug/share
    /etc: /usr/sgug/etc

  # Configure flags to always inject (based on SGUG-RSE patterns)
  configure_flags:
    add:
      - --prefix=/usr/sgug
      - --libdir=/usr/sgug/lib32
      - --disable-silent-rules  # 11 packages - verbose builds
      - --disable-rpath         # 11 packages - SGUG handles rpath
    remove:
      - --with-selinux
      - --enable-selinux
      - --with-systemd
      - --enable-systemd
      - --with-libcap
      - --enable-libcap

  # Features to disable globally (from empirical analysis)
  configure_disable:
    - selinux                   # Linux security
    - systemd                   # Linux init
    - udev                      # Linux device manager
    - inotify                   # Linux file monitoring (2 packages)
    - epoll                     # Linux I/O (2 packages)
    - fanotify                  # Linux file access
    - timerfd                   # Linux timer (1 package)
    - libcap                    # Linux capabilities
    - audit                     # Linux audit

  # Optional feature disables (frequently used but may be wanted)
  configure_disable_optional:
    - gtk-doc                   # 7 packages
    - doxygen                   # 2 packages
    - doxygen-docs              # 2 packages
    - ldap                      # 3 packages
    - cups                      # 3 packages
    - nls                       # 2 packages - may want localization
    - dbus                      # 2 packages

  # Toolchain settings
  toolchain:
    CC: /opt/cross/bin/irix-cc-bootstrap
    CXX: /opt/cross/bin/irix-cxx-bootstrap
    AR: /opt/cross/bin/mips-sgi-irix6.5-ar
    RANLIB: /opt/cross/bin/mips-sgi-irix6.5-ranlib
    CFLAGS: "-O2 -mabi=n32 -march=mips3"
    LDFLAGS: "-L/opt/sgug-staging/usr/sgug/lib32"

  # Subpackages to always drop
  drop_subpackages:
    - "*-debuginfo"
    - "*-debugsource"
    - "*-langpack-*"

  # Header overlays (applied to all packages)
  header_overlays:
    - generic                   # Always include generic clang/IRIX fixes
```

### Package-Class Rules (`rules/classes/*.yaml`)

```yaml
# rules/classes/compression.yaml
class: compression
match:
  names: [zlib, bzip2, xz, lz4, zstd, libzip]

rules:
  force_static: true
  configure_flags:
    add:
      - --disable-shared
      - --enable-static
  drop_requires:
    - "*-devel"  # Self-referential devel deps

---
# rules/classes/autotools.yaml
class: autotools
match:
  files: ["configure.ac", "configure.in", "Makefile.am"]

rules:
  # Force autoconf cache values for cross-compilation
  ac_cv_overrides:
    ac_cv_func_malloc_0_nonnull: "yes"
    ac_cv_func_realloc_0_nonnull: "yes"
    ac_cv_func_mmap_fixed_mapped: "no"
    ac_cv_func_getpgrp_void: "yes"
    ac_cv_func_setpgrp_void: "yes"
    ac_cv_func_vfork_works: "no"
    gl_cv_func_working_getdelim: "yes"
    gl_cv_func_working_strerror: "yes"

---
# rules/classes/gnulib.yaml
class: gnulib
match:
  files: ["gnulib/**", "gl/**", "lib/gnulib/**"]

rules:
  # Prefer libdicl replacements over gnulib built-ins
  configure_flags:
    add:
      - --with-libdicl
  add_requires:
    build:
      - libdicl-devel
  notes:
    - "gnulib functions may conflict with libdicl; prefer libdicl"
```

### Package-Specific Rules (`rules/packages/*.yaml`)

```yaml
# rules/packages/coreutils.yaml
package: coreutils
version: ">=8.30"

rules:
  add_patch:
    - coreutils-irix-vfork.patch
    - coreutils-irix-getprogname.patch
    - coreutils-irix-eaccess.patch
  configure_flags:
    add:
      - --disable-libcap
      - --disable-acl
      - --disable-xattr
      - --without-selinux
    remove:
      - --with-openssl
  disable_features:
    - libcap
    - acl
    - xattr
  notes:
    - "vfork/fork conflicts require patching"
    - "getprogname needs IRIX procfs simplification"

---
# rules/packages/bash.yaml
package: bash
version: ">=5.0"

rules:
  add_patch:
    - bash-irix-readline.patch
  configure_flags:
    add:
      - --without-bash-malloc
      - --disable-nls
  ac_cv_overrides:
    bash_cv_job_control_missing: "present"
    bash_cv_sys_named_pipes: "present"
    bash_cv_func_sigsetjmp: "present"
```

---

## Header Overlay System

Rules alone cannot solve all IRIX compatibility issues. Many problems require **replacement headers** that intercept includes before the real IRIX headers are processed. Mogrix maintains a header overlay system inspired by libdicl's `clang_compat` headers.

### The Problem

IRIX headers assume MIPSpro compiler behavior:
- `stdarg.h` uses MIPSpro-specific `__builtin` macros that clang doesn't have
- `stdio.h` defines `va_list` as `char *`, conflicting with clang's `__builtin_va_list`
- `sys/types.h` uses `__scint_t` and other MIPSpro compiler builtins
- Various headers have C99/C11 assumptions that IRIX libc doesn't satisfy

We cannot modify the pristine IRIX sysroot. Instead, we place wrapper headers earlier in the include path.

### Header Overlay Architecture

```
Include search order (injected via -I flags):

1. /opt/sgug-staging/usr/sgug/include/mogrix-compat/     ← Package-specific overrides
2. /opt/sgug-staging/usr/sgug/include/dicl-clang-compat/ ← Generic clang fixes
3. /opt/sgug-staging/usr/sgug/include/                   ← Cross-built package headers
4. /opt/irix-sysroot/usr/include/                        ← Pristine IRIX headers
```

### Header Types

#### 1. Complete Replacement Headers

Entirely replace the IRIX header (don't chain to original):

```c
/* stdarg.h - complete replacement for IRIX stdarg.h */
#ifndef _STDARG_H
#define _STDARG_H

typedef __builtin_va_list va_list;

#define va_start(ap, param) __builtin_va_start(ap, param)
#define va_end(ap)          __builtin_va_end(ap)
#define va_arg(ap, type)    __builtin_va_arg(ap, type)
#define va_copy(dest, src)  __builtin_va_copy(dest, src)

#endif /* _STDARG_H */
```

Used when: The IRIX header is fundamentally incompatible and must be wholly replaced.

#### 2. Wrapper Headers (with `#include_next`)

Pre-define or fix things, then chain to the real IRIX header:

```c
/* stdio.h - wrapper that fixes va_list before including IRIX stdio.h */
#ifndef _DICL_CLANG_COMPAT_STDIO_H
#define _DICL_CLANG_COMPAT_STDIO_H

/* Pre-define va_list to prevent IRIX from defining it as char* */
#ifndef _VA_LIST_
#define _VA_LIST_
typedef __builtin_va_list va_list;
#endif

/* Now include the real IRIX stdio.h */
#include_next <stdio.h>

#endif
```

Used when: The IRIX header is mostly fine but needs specific fixes applied before inclusion.

#### 3. Augmentation Headers

Include the real header, then add missing declarations:

```c
/* sys/types.h - augment IRIX sys/types.h with missing types */
#ifndef _DICL_SYS_TYPES_H
#define _DICL_SYS_TYPES_H

/* Define MIPSpro builtins that clang doesn't provide */
#ifndef __scint_t
typedef int __scint_t;
#endif
#ifndef __scunsigned_t
typedef unsigned int __scunsigned_t;
#endif

#include_next <sys/types.h>

/* Add POSIX types IRIX lacks */
#ifndef _SIGSET_T
#define _SIGSET_T
/* sigset_t definition if needed */
#endif

#endif
```

Used when: The IRIX header works but is missing types or declarations that modern code expects.

### Header Catalog Structure

```
headers/
├── generic/                    # Apply to all builds
│   ├── stdarg.h               # Complete replacement
│   ├── stdbool.h              # Complete replacement
│   ├── stddef.h               # Wrapper (fixes offsetof)
│   ├── stdio.h                # Wrapper (fixes va_list)
│   ├── setjmp.h               # Augmentation (adds sigjmp_buf)
│   ├── signal.h               # Augmentation (sigset_t)
│   └── sys/
│       └── types.h            # Augmentation (MIPSpro builtins)
│
├── classes/
│   └── gnulib/                # Headers needed by gnulib-using packages
│       ├── stdint.h           # C99 integer types
│       └── inttypes.h         # printf format macros
│
└── packages/
    ├── coreutils/             # Package-specific headers
    │   └── lib/
    │       └── getprogname.h  # IRIX-specific getprogname
    └── bash/
        └── config-top.h       # bash-specific config overrides
```

### Integration with Rules

Header overlays are specified in rules and automatically injected:

```yaml
# rules/generic.yaml
generic:
  header_overlays:
    - generic                   # Always include generic fixes

# rules/classes/gnulib.yaml
class: gnulib
rules:
  header_overlays:
    - generic
    - classes/gnulib            # Add gnulib-specific headers

# rules/packages/coreutils.yaml
package: coreutils
rules:
  header_overlays:
    - generic
    - packages/coreutils        # Add coreutils-specific headers
```

### How Mogrix Applies Header Overlays

During conversion, Mogrix:

1. **Collects applicable overlays** from generic, class, and package rules
2. **Injects CPPFLAGS** in the spec file to add overlay directories:
   ```
   CPPFLAGS="-I/usr/sgug/include/mogrix-compat/packages/coreutils \
             -I/usr/sgug/include/mogrix-compat/generic \
             $CPPFLAGS"
   ```
3. **Bundles headers** into the RSE SRPM if they're package-specific, or references the installed overlay package for generic headers

### Overlay Packaging Options

**Option A: Single mogrix-headers package**
- All generic headers installed to `/usr/sgug/include/mogrix-compat/`
- RSE packages `BuildRequire: mogrix-headers`
- Simplest approach, single point of maintenance

**Option B: Headers embedded in SRPM**
- Package-specific headers bundled into converted SRPM
- Extracted during `%prep` phase
- More self-contained but duplicates generic headers

**Option C: Hybrid**
- Generic headers in `mogrix-headers` package
- Package-specific headers embedded in SRPM
- Best balance of maintainability and specificity

**Recommendation:** Option C (Hybrid)

### Creating New Header Overlays

When a package fails to build due to header issues:

1. **Identify the problem** (compiler error pointing to IRIX header)
2. **Determine fix type** (replacement, wrapper, or augmentation)
3. **Create header file** in appropriate location under `headers/`
4. **Test build** with overlay in include path
5. **Add to rules** (generic, class, or package level)
6. **Document** the issue and fix in the header file comments

Example workflow:
```bash
# Package fails with: /opt/irix-sysroot/usr/include/foo.h:42: error: unknown type 'bar_t'

# Create wrapper header
cat > headers/packages/mypackage/foo.h << 'EOF'
/* foo.h - wrapper to define bar_t before including IRIX foo.h */
#ifndef _MOGRIX_FOO_H
#define _MOGRIX_FOO_H

typedef int bar_t;  /* IRIX lacks this POSIX type */

#include_next <foo.h>

#endif
EOF

# Add to package rules
# rules/packages/mypackage.yaml:
#   header_overlays:
#     - packages/mypackage
```

### Existing Headers (from libdicl clang_compat)

The following headers are already developed and validated:

| Header | Type | Purpose |
|--------|------|---------|
| `stdarg.h` | Replacement | Use clang builtins for varargs |
| `stdbool.h` | Replacement | C99 bool without MIPSpro check |
| `stddef.h` | Wrapper | Fix `offsetof` macro |
| `stdio.h` | Wrapper | Pre-define `va_list` |
| `setjmp.h` | Augmentation | Add `sigjmp_buf` |
| `signal.h` | Augmentation | Ensure `sigset_t` |
| `sys/types.h` | Augmentation | Define `__scint_t`, `__scunsigned_t` |
| `stdint.h` | Replacement | C99 integer types |
| `ctype.h` | Wrapper | Character classification fixes |
| `wchar.h` | Wrapper | Wide character fixes |
| `unistd.h` | Augmentation | POSIX function declarations |
| `utmpx.h` | Wrapper | utmp/utmpx compatibility |
| `netdb.h` | Wrapper | Network database fixes |
| `sys/time.h` | Wrapper | Time-related fixes |

---

## Compatibility Source Injection

Beyond header fixes, many packages need implementations of functions missing from IRIX libc. Rather than linking against a separate compatibility library (libdicl), mogrix injects these source files directly into each package that needs them.

### The Problem

IRIX 6.5 libc lacks many POSIX/GNU functions that modern code expects:
- `strdup`, `strndup` — String duplication
- `getline`, `getdelim` — Line reading
- `asprintf`, `vasprintf` — Formatted string allocation
- `strerror_r` — Thread-safe error strings
- `posix_memalign` — Aligned memory allocation
- `clock_gettime` — High-resolution time
- `mkostemp` — Secure temp file with flags

Previously, these were provided by libdicl. Now, mogrix bundles the implementations directly.

### Compat Source Catalog

```
compat/
├── string/
│   ├── strdup.c              # char *strdup(const char *)
│   ├── strndup.c             # char *strndup(const char *, size_t)
│   ├── stpcpy.c              # char *stpcpy(char *, const char *)
│   └── stpncpy.c             # char *stpncpy(char *, const char *, size_t)
│
├── stdio/
│   ├── getline.c             # ssize_t getline(char **, size_t *, FILE *)
│   ├── getdelim.c            # ssize_t getdelim(char **, size_t *, int, FILE *)
│   ├── asprintf.c            # int asprintf(char **, const char *, ...)
│   ├── vasprintf.c           # int vasprintf(char **, const char *, va_list)
│   └── open_memstream.c      # FILE *open_memstream(char **, size_t *)
│
├── stdlib/
│   ├── posix_memalign.c      # int posix_memalign(void **, size_t, size_t)
│   ├── reallocarray.c        # void *reallocarray(void *, size_t, size_t)
│   └── secure_getenv.c       # char *secure_getenv(const char *)
│
├── time/
│   ├── clock_gettime.c       # int clock_gettime(clockid_t, struct timespec *)
│   └── nanosleep.c           # int nanosleep(const struct timespec *, ...)
│
├── unistd/
│   ├── eaccess.c             # int eaccess(const char *, int)
│   ├── pipe2.c               # int pipe2(int[2], int)
│   ├── dup3.c                # int dup3(int, int, int)
│   └── getprogname.c         # const char *getprogname(void)
│
└── error/
    ├── strerror_r.c          # int strerror_r(int, char *, size_t)
    └── err.c                 # err(), errx(), warn(), warnx()
```

### How Injection Works

1. **Detection:** Rules specify which compat sources a package needs
2. **Bundling:** Mogrix copies required `.c` files into the SRPM sources
3. **Spec modification:** Adds compilation of compat sources to `%build`
4. **Linking:** Compat objects are linked into the final binaries

### Rule Specification

```yaml
# rules/packages/coreutils.yaml
package: coreutils

rules:
  inject_compat_sources:
    - string/strdup.c
    - string/strndup.c
    - stdio/getline.c
    - unistd/getprogname.c
    - unistd/eaccess.c

# Alternative: inject by function name (mogrix resolves to file)
  inject_compat_functions:
    - strdup
    - getline
    - eaccess
```

### Spec File Modifications

Mogrix modifies the spec to compile and link compat sources:

```spec
# Added to %prep
mkdir -p mogrix-compat
cp %{SOURCE100} %{SOURCE101} %{SOURCE102} mogrix-compat/

# Added to %build (before main build)
for f in mogrix-compat/*.c; do
  %{__cc} %{optflags} -c $f -o ${f%.c}.o
done

# Modified LDFLAGS or LIBS
export LIBS="mogrix-compat/*.o $LIBS"
```

### Compat Source Guidelines

Each compat source file must:

1. **Be self-contained** — No dependencies on other compat files
2. **Guard against double-definition** — Check if function already exists
3. **Use `__sgi` guards** — Only compile on IRIX
4. **Match expected signatures** — POSIX/GNU compatible prototypes

Example structure:

```c
/* strdup.c - provide strdup() for IRIX */
#include <stdlib.h>
#include <string.h>

#if defined(__sgi) && !defined(HAVE_STRDUP)

char *strdup(const char *s)
{
    size_t len = strlen(s) + 1;
    char *dup = malloc(len);
    if (dup)
        memcpy(dup, s, len);
    return dup;
}

#endif /* __sgi && !HAVE_STRDUP */
```

### Class-Based Injection

Common patterns can be specified at the class level:

```yaml
# rules/classes/gnulib.yaml
class: gnulib
match:
  files: ["gnulib/**", "gl/**"]

rules:
  # gnulib packages often need these
  inject_compat_sources:
    - stdio/getline.c
    - stdio/getdelim.c
    - string/strdup.c
    - string/strndup.c
```

### Benefits Over libdicl

| Aspect | libdicl (old) | Mogrix injection (new) |
|--------|---------------|------------------------|
| Build dependency | Required | None |
| Runtime dependency | Required | None |
| Package size | Smaller (shared) | Slightly larger (bundled) |
| Isolation | Shared library | Self-contained |
| Debugging | Separate library | Code in package |
| Version skew | Possible | Impossible |
| Per-package customization | Hard | Easy |

---

## Patch Catalog

Pre-prepared patches organized by package, version, and purpose:

```
patches/
├── generic/
│   ├── getprogname-irix.patch      # Simplified getprogname for IRIX
│   ├── vfork-to-fork.patch         # vfork/fork conflict resolution
│   ├── eaccess-stub.patch          # eaccess() unavailable on IRIX
│   └── vasnprintf-irix.patch       # vsnprintf NULL buffer fix
│
├── coreutils/
│   └── 8.30/
│       ├── sgifixes01.patch        # From SGUG-RSE
│       └── soft-float-stubs.patch  # compiler-rt long double
│
├── bash/
│   └── 5.2/
│       └── readline-pselect.patch
│
└── tar/
    └── 1.30/
        └── obstack-irix.patch
```

---

## CLI Interface

```bash
# Basic conversion
mogrix convert \
  --srpm zlib-1.2.13-5.fc40.src.rpm \
  --target irix6.5-n32 \
  --output ./zlib-rse.src.rpm

# With verbose report
mogrix convert \
  --srpm coreutils-8.32-35.fc40.src.rpm \
  --target irix6.5-n32 \
  --output ./coreutils-rse.src.rpm \
  --report ./coreutils-conversion.md

# Dry-run (show what would change)
mogrix convert \
  --srpm bash-5.2.21-1.fc40.src.rpm \
  --target irix6.5-n32 \
  --dry-run

# List rules that would apply
mogrix analyze \
  --srpm grep-3.8-3.fc40.src.rpm

# Validate rules
mogrix validate-rules

# List header overlays that would apply
mogrix headers \
  --package coreutils

# Install header overlays to staging area
mogrix install-headers \
  --dest /opt/sgug-staging/usr/sgug/include/mogrix-compat/

# Build converted SRPM (optional convenience)
mogrix build \
  --srpm ./zlib-rse.src.rpm \
  --macros /opt/sgug-staging/rpmmacros.irix
```

---

## Conversion Report Format

Each conversion produces a detailed report:

```markdown
# Conversion Report: zlib-1.2.13-5.fc40 → zlib-1.2.13-5.rse

## Summary
- **Status**: Success
- **Rules Applied**: 12
- **Patches Added**: 0
- **Warnings**: 2

## Rules Applied

### Generic Rules
1. ✓ drop_requires: systemd (not present, skipped)
2. ✓ rewrite_path: /usr/lib64 → /usr/sgug/lib32
3. ✓ inject_macro: %_prefix=/usr/sgug

### Class Rules (compression)
4. ✓ force_static: enabled --disable-shared
5. ✓ configure_flags: added --enable-static

### Package Rules
(none specific to zlib)

## Spec Changes
- Line 23: Removed `BuildRequires: systemd-rpm-macros`
- Line 45: Changed `%configure` to `%configure --disable-shared`
- Line 67: Changed `/usr/lib64` to `/usr/sgug/lib32`

## Header Overlays Applied
- generic/stdarg.h (replacement - clang varargs builtins)
- generic/stdio.h (wrapper - va_list pre-definition)
- generic/sys/types.h (augmentation - __scint_t, __scunsigned_t)

## CPPFLAGS Injected
```
-I/usr/sgug/include/mogrix-compat/generic
```

## Warnings
- W001: Skipped subpackage zlib-debuginfo (no debug on IRIX)
- W002: Feature test ac_cv_func_mmap assumed 'no'

## Patches Included
- (original) zlib-1.2.7-autotools-glibc.patch
- (original) zlib-1.2.11-optimized-s390.patch (disabled for IRIX)
```

---

## Implementation Phases

**Ultimate Goal:** Build `tdnf` and all 123 dependencies on IRIX.

### Phase 1: Core Engine (MVP) - COMPLETED

**Goal:** Functional conversion of foundation packages (zlib, bzip2, xz, pkgconfig)

- [x] Basic spec file parser (preamble + scriptlets) - `mogrix/parser/spec.py`
- [x] Generic rule loader (YAML) - `mogrix/rules/loader.py`
- [x] Rule application engine - `mogrix/rules/engine.py`
- [x] Header overlay manager (inject -I paths) - `mogrix/headers/overlay.py`
- [x] Port generic clang_compat headers from libdicl - `headers/generic/` (16 headers)
- [x] Spec file rewriter - `mogrix/emitter/spec.py`
- [x] Basic CLI (`mogrix convert`, `mogrix analyze`) - `mogrix/cli.py`
- [ ] SRPM extraction (spec files only for now)
- [ ] SRPM emitter (outputs spec content, not full SRPM)
- [ ] Conversion report generator

**Deliverable:** Core engine functional, can convert spec files

### Phase 2: Compat Source System - COMPLETED

**Goal:** Handle the 26 packages needing compat function injection

- [x] Compat source catalog (strdup, getline, asprintf, etc.) - `compat/catalog.yaml`
- [x] Compat source injector (bundle .c files into SRPM) - `mogrix/compat/injector.py`
- [x] Spec modification for compat compilation
- [x] Package-specific rule support - `rules/packages/*.yaml`
- [x] ac_cv override injection
- [x] drop_requires and remove_lines support

**Deliverable:** Successfully converts popt with compat source injection

### Phase 3: Build Chain Expansion - COMPLETED

**Goal:** Build through the dependency chain toward tdnf

- [x] Patch catalog integration - `mogrix/patches/catalog.py`
- [x] Package-specific rule files for security/crypto stack (11 packages)
- [x] Configure flag manipulation (add/remove)
- [x] Conditional handling in specs (%if/%endif)
- [x] Subpackage management (drop debuginfo, langpacks)
- [x] `mogrix analyze` command

**Deliverable:** Package rules for Tier 5 security/crypto stack created and tested

### Phase 4: Full tdnf Stack

**Goal:** Complete the tdnf dependency chain

- [ ] Python3 and GLib ecosystem packages
- [ ] rpm, libsolv, libarchive, zchunk
- [ ] Multi-source packages
- [ ] Rule inheritance and composition
- [ ] Batch conversion mode with dependency ordering
- [ ] `mogrix build` convenience command

**Deliverable:** Convert all 123 packages including tdnf

### Phase 5: Integration & Polish

**Goal:** Production-ready tooling

- [ ] Integration with rpmbuild workflow
- [ ] CI/CD pipeline support
- [ ] Rule validation and linting
- [ ] Documentation and examples
- [ ] Bootstrap testing on real IRIX hardware

**Deliverable:** Working tdnf installation on IRIX 6.5

---

## Technology Choices

### Language: Python 3.10+

Rationale:
- Rich ecosystem for text parsing (rpm-python, PyYAML)
- Good spec file manipulation libraries available
- Easy to extend and maintain
- Runs on Linux build hosts (not on IRIX)

### Dependencies

```
python >= 3.10
pyyaml >= 6.0          # Rule file parsing
rpm >= 4.18            # SRPM handling (rpm-python bindings)
jinja2 >= 3.0          # Spec file templating
click >= 8.0           # CLI framework
rich >= 13.0           # Pretty output and reports
```

### Project Structure

```
mogrix/
├── mogrix/
│   ├── __init__.py
│   ├── cli.py              # Click-based CLI
│   ├── parser/
│   │   ├── __init__.py
│   │   ├── srpm.py         # SRPM extraction
│   │   └── spec.py         # Spec file parser
│   ├── rules/
│   │   ├── __init__.py
│   │   ├── engine.py       # Rule application
│   │   ├── loader.py       # YAML rule loading
│   │   └── types.py        # Rule type definitions
│   ├── emitter/
│   │   ├── __init__.py
│   │   ├── spec.py         # Spec file writer
│   │   ├── srpm.py         # SRPM packager
│   │   └── report.py       # Conversion report
│   ├── patches/
│   │   └── catalog.py      # Patch catalog manager
│   ├── headers/
│   │   └── overlay.py      # Header overlay manager
│   └── compat/
│       └── injector.py     # Compat source injection
│
├── rules/
│   ├── generic.yaml
│   ├── classes/
│   │   ├── autotools.yaml
│   │   ├── cmake.yaml
│   │   ├── compression.yaml
│   │   └── gnulib.yaml
│   └── packages/
│       ├── bash.yaml
│       ├── coreutils.yaml
│       └── ...
│
├── headers/                 # Header overlay catalog
│   ├── generic/            # Universal clang/IRIX fixes
│   │   ├── stdarg.h
│   │   ├── stdbool.h
│   │   ├── stddef.h
│   │   ├── stdio.h
│   │   ├── setjmp.h
│   │   ├── signal.h
│   │   └── sys/
│   │       └── types.h
│   ├── classes/            # Class-specific headers
│   │   └── gnulib/
│   │       └── stdint.h
│   └── packages/           # Package-specific headers
│       └── coreutils/
│           └── lib/
│
├── patches/
│   ├── generic/
│   └── packages/
│
├── compat/                  # Compat source files (replaces libdicl)
│   ├── string/             # strdup.c, strndup.c, stpcpy.c
│   ├── stdio/              # getline.c, getdelim.c, asprintf.c
│   ├── stdlib/             # posix_memalign.c, reallocarray.c
│   ├── time/               # clock_gettime.c
│   ├── unistd/             # eaccess.c, pipe2.c, getprogname.c
│   └── error/              # strerror_r.c
│
├── tests/
│   ├── test_parser.py
│   ├── test_rules.py
│   ├── test_headers.py
│   ├── test_compat.py
│   └── fixtures/
│
├── pyproject.toml
└── README.md
```

---

## Relationship to Existing Work

### Uses from /src/plan.md:
- Cross-compilation toolchain paths and settings
- rpmmacros.irix configuration
- Validated package list and known fixes
- SGUG-RSE path conventions (/usr/sgug)

### Absorbs from /src/dicl:
- clang_compat headers → mogrix `headers/` overlay system
- repl_headers patterns → wrapper/augmentation techniques in overlays
- gnulib replacement functions → mogrix `compat/` source files
- IRIX header quirks knowledge → encoded in rules and headers

**libdicl is fully absorbed** — no separate compatibility library needed.

### Does NOT replace:
- rpmbuild (mogrix produces input for rpmbuild)
- The cross-toolchain (still needed to actually build)

---

## Success Criteria

1. **Functional:** Can convert all 123 tdnf dependency packages from FC40 SRPMs to buildable RSE SRPMs
2. **Self-contained:** Converted packages have no libdicl dependency; all compat code is bundled
3. **Auditable:** Every transformation (rules, headers, compat sources) is logged and explainable
4. **Extensible:** Adding new rules, headers, or compat sources requires only YAML/C, not core code changes
5. **Reproducible:** Same input SRPM + rules + headers + compat = identical output SRPM
6. **Testable:** Rule application and header overlay inclusion can be tested without building packages
7. **Header-complete:** Generic clang_compat headers cover common IRIX/clang conflicts
8. **Ultimate goal:** Working `tdnf` package manager on IRIX 6.5

---

## Risks and Mitigations

| Risk | Impact | Mitigation |
|------|--------|------------|
| Spec file parsing too complex | High | Start with simple packages; expand parser incrementally |
| Rule conflicts | Medium | Priority ordering; conflict detection in validator |
| FC40 spec changes break rules | Medium | Version constraints in rules; CI testing |
| Missing patches for packages | Low | Patch catalog grows organically; fallback to manual |
| Header overlay conflicts | Medium | Strict include order; test with multiple packages |
| New IRIX header issues discovered | Medium | Document fix pattern; add to overlay catalog |
| `#include_next` portability | Low | Standard clang/gcc feature; well-documented pattern |

---

## Next Steps (Phase 4)

Phase 4 focuses on completing the full tdnf dependency chain:

1. **Create rules for Python3/GLib ecosystem**
   - python3 (SGIFIXES, LIBDICL)
   - glib2 (SGIFIXES, LIBDICL)
   - gobject-introspection (SGIFIXES, LIBDICL)

2. **Create rules for package management core**
   - elfutils (SGIFIXES, LIBDICL)
   - file (LIBDICL)
   - libarchive (SGIFIXES)
   - rpm (SGIFIXES, LIBDICL)
   - libsolv (SGIFIXES, LIBDICL)
   - zchunk (SGIFIXES, LIBDICL)

3. **Create rules for network stack**
   - curl
   - libmetalink

4. **Create tdnf rule and test full conversion**
   - tdnf (SGIFIXES, LIBDICL)

5. **Implement batch conversion mode**
   - Dependency-ordered conversion
   - `mogrix build` convenience command

6. **Add SRPM handling**
   - Extract SRPMs
   - Package converted specs back into SRPMs

**Priority packages remaining (critical path to tdnf):**
- python3, glib2 (ecosystem)
- rpm, libsolv, libarchive (package management)
- curl, libmetalink (network)
- tdnf (target)
