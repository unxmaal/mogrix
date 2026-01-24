# Claude Code Instructions for Mogrix

## CRITICAL: Mogrix is a Knowledge Repository

**Mogrix's primary mission is STORING KNOWLEDGE.** Every fix you discover for building Linux packages on IRIX MUST be written into mogrix rules so it never needs to be discovered again.

### No Shortcuts

**Mogrix codifies "the right way" to convert SRPMs.** If you take a shortcut, you're codifying that shortcut - it becomes part of the permanent solution.

When you hit an obstacle:
1. **Ask yourself**: Am I avoiding solving the actual problem?
2. **Don't declare blockers "acceptable"** - find a way around them
3. **Be creative**: If sed fails, use perl. If a rule isn't implemented, implement it.
4. **Push through**: The goal is proper solutions, not workarounds

Examples of unacceptable shortcuts:
- Stubbing functionality because porting is "too hard"
- Declaring missing features "optional" to avoid implementing them
- Using workarounds when proper fixes are possible
- Giving up after one approach fails

The extra effort now saves everyone time forever.

### The Cardinal Sin

**NEVER make a fix outside of mogrix rules.**

If you:
- Edit a file in `/opt/sgug-staging/` directly
- Apply a sed command during debugging
- Modify a header in the build directory
- Fix anything manually during a build

...and that fix is NOT stored in mogrix rules, **you have failed the mission**.

The next session will hit the same problem. The next package will hit the same problem. All that knowledge is LOST.

### The Golden Rule

**Every fix goes into mogrix:**

| Fix Type | Where to Store |
|----------|----------------|
| Missing compat function | `compat/catalog.yaml` + implementation |
| Header issue | `cross/include/dicl-clang-compat/` |
| Package-specific fix | `rules/packages/<package>.yaml` |
| Common pattern | `rules/generic.yaml` or new class |
| Build system quirk | Document in rules + fix |

### Patches: Apply Changes Directly, Don't Rule Them

**SGUG-RSE patches are a starting point, not the final answer.**

If you find yourself writing a rule that modifies a patch file:
```yaml
# BAD - rule that modifies a patch
spec_replacements:
  - pattern: "some text in the patch"
    replacement: "fixed text"
```

**STOP.** Instead:
1. Apply the modification directly to the patch file in `mogrix/patches/`
2. Delete the rule that was doing the modification
3. The patch file should be the final, correct version

**Why?**
- Patches ARE knowledge storage - they should contain the complete fix
- Rules that modify patches add unnecessary indirection
- Debugging is harder when fixes are split between patches and rules
- The goal is: patch applies cleanly, no further modification needed

**Workflow for inherited SGUG-RSE patches:**
1. Copy the `.sgifixes.patch` to mogrix
2. If it needs changes for our toolchain, edit the patch directly
3. The mogrix patch becomes the authoritative version
4. No rules should reference or modify patch content

### Before Ending a Session

Ask yourself:
1. Did I make any fixes outside of mogrix source?
2. Are those fixes now stored in mogrix rules?
3. Could someone rebuild from scratch using only mogrix?

If the answer to #3 is "no", you have more work to do.

---

## Using Mogrix Properly

**Mogrix is an SRPM conversion tool.** It converts Fedora SRPMs into IRIX-compatible SRPMs that build without additional manual intervention.

### The Golden Rule

**NEVER build packages manually from upstream tarballs.** Always use the mogrix workflow:

```bash
# Activate the Python environment first
source .venv/bin/activate

# 1. Fetch the Fedora SRPM
mogrix fetch <package>

# 2. Convert to IRIX-compatible SRPM
mogrix convert workdir/<package>-*.src.rpm -o workdir/converted/

# 3. Build using rpmbuild with cross-compilation
rpmbuild --rebuild workdir/converted/<package>-*.src.rpm \
    --define "_topdir $HOME/rpmbuild" \
    --define "__cc /opt/sgug-staging/usr/sgug/bin/irix-cc" \
    --define "__ld /opt/sgug-staging/usr/sgug/bin/irix-ld"
```

### If a Build Fails

**DO NOT manually patch the build.** Instead:

1. **Identify the issue** (missing function, header problem, configure flag)
2. **Fix it in mogrix**:
   - Header issues → Update `cross/include/dicl-clang-compat/` headers
   - Missing functions → Add to `compat/` and `compat/catalog.yaml`
   - Configure/build issues → Update `rules/packages/<package>.yaml`
3. **Re-run the mogrix workflow** to verify the fix works

### Key Directories

| Purpose | Path |
|---------|------|
| Package rules | `rules/packages/*.yaml` |
| Compat headers | `cross/include/dicl-clang-compat/` |
| Compat source code | `compat/` |
| Compat catalog | `compat/catalog.yaml` |
| Generic headers | `headers/generic/` |
| Toolchain wrappers | `cross/bin/irix-cc`, `cross/bin/irix-ld` |

### Compat Headers (cross/include/dicl-clang-compat/)

Headers that override IRIX system headers to fix compatibility issues:

| Header | Purpose |
|--------|---------|
| `stdarg.h` | Define va_list before IRIX headers (prevents conflict) |
| `stdbool.h` | C99 bool type |
| `stdint.h` | Fixed-width integer types |
| `stdlib.h` | Additional function declarations |
| `string.h` | Add strerror_r declaration |
| `ctype.h` | Add isblank() function |
| `complex.h` | Standalone _Complex support (avoids IRIX va_list conflict) |
| `inttypes.h` | C99 format macros (PRIu64, PRId64, etc.) |
| `sys/socket.h` | Force XPG msghdr with msg_flags field |
| `sys/random.h` | Stub getrandom() returning -1/ENOSYS |
| `sys/types.h` | Type compatibility fixes |
| `sys/time.h` | Time structure fixes |
| `sys/procfs.h` | Process filesystem compatibility |
| `sys/termio.h` | Terminal I/O compatibility |

### Creating/Updating Package Rules

Package rules are YAML files in `rules/packages/`. Example structure:

```yaml
package: example
configure_flags:
  add:
    - "--disable-nls"
    - "--without-threads"
  remove:
    - "--enable-shared"
inject_compat_functions:
  - strdup
  - getline
export_vars:
  CC: "/opt/sgug-staging/usr/sgug/bin/irix-cc"
# Fix spec file text (e.g., undefined macros, shell syntax)
spec_replacements:
  - pattern: "%{__global_ldflags}"
    replacement: ""
  - pattern: "mkdir -p $RPM_BUILD_ROOT{dir1,dir2}"
    replacement: "mkdir -p $RPM_BUILD_ROOT/dir1 $RPM_BUILD_ROOT/dir2"
```

### Global Rules (generic.yaml)

The following are handled globally for all packages:

| Rule | Effect |
|------|--------|
| `skip_check: true` | Comments out %check section (tests can't run cross-compiled) |
| `_pkgdocdir` macro | Defined as `%{_docdir}/%{name}` |
| `_docdir` macro | Defined as `/usr/sgug/share/doc` |
| Linux-specific BuildRequires | Dropped (systemd, libselinux, kernel-headers, etc.) |
| `%{gpgverify}` | Removed (source verification not needed) |
| `%ldconfig_scriptlets` | Removed (don't work for cross-compilation) |

### Testing on IRIX

After building, copy to IRIX and test:

```bash
# Copy RPM to IRIX
scp ~/rpmbuild/RPMS/mips/<package>-*.rpm edodd@192.168.0.81:/tmp/

# On IRIX, install and test
ssh edodd@192.168.0.81
sudo rpm -Uvh /tmp/<package>-*.rpm
# Test the installed package
```

---

## IRIX Test Host (SGI Octane)

- **SSH**: `ssh edodd@192.168.0.81`
- **After connecting**: Run `/usr/sgug/bin/sgugshell` immediately to load RSE environment
- **sudo**: Available without password
- **IRIX version**: 6.5.30
- **Important**: Never use `--nodeps` when installing RPMs for testing

## Python Development

- Use `uv` for package management (not pip directly)
  - Install packages: `uv pip install <package>`
  - Create venv: `uv venv`
  - Sync dependencies: `uv pip sync requirements.txt`

- Follow Test-Driven Development (TDD) methodology:
  1. Write a failing test first
  2. Write the minimum code to pass the test
  3. Refactor as needed
  4. Repeat

## Git Usage

- Use git to track changes
- Never commit everything at once - use specific, targeted commits
- Commit messages must be 5 words or fewer
- Never use emoji in commits or anywhere else

## Cross-Compilation - SOLVED
Note this vm is allocated 12 cores, so you can set these:
export MAKEFLAGS=-j12
export NINJA_FLAGS=-j12


### Dynamic Linking (SOLVED)

Cross-compiled dynamically-linked executables and shared libraries now work on IRIX.

**Key discoveries:**

1. **Shared libraries**: Use GNU ld (`mips-sgi-irix6.5-ld.bfd`)
   - GNU ld produces correct 2-LOAD segment layout (R E + RW)
   - LLD produces 3-LOAD segments which crashes IRIX rld
   - The `irix-ld` wrapper automatically selects GNU ld when `-shared` is present

2. **Executables**: Use LLD with `--dynamic-linker=/lib32/rld`
   - LLD defaults to `/usr/lib32/libc.so.1` as interpreter - this segfaults
   - Must explicitly set `--dynamic-linker=/lib32/rld` for IRIX
   - The `irix-ld` wrapper handles this automatically

**The .liblist section is NOT required** - IRIX rld works fine without it when the correct
interpreter and segment layout are used. Native IRIX binaries have .liblist for QUICKSTART
optimization, but it's not mandatory.

### IRIX Header Quirks

- **Header ordering**: Include `<time.h>` before `<sys/time.h>` to avoid redefinition errors
- **_XOPEN_SOURCE=500**: Enables POSIX functions but hides some IRIX-specific APIs
- **scandir/alphasort**: Hidden behind `_SGIAPI` when `_XOPEN_SOURCE` is defined -
  declarations provided in `irix-compat.h`
- **pthread/dlopen incompatibility**: Libraries using pthreads work when directly linked
  but may crash when loaded via dlopen()
- **va_list conflict**: IRIX stdio_core.h defines `va_list` as `char *`, but clang builtins
  require `__builtin_va_list`. The irix-cc wrapper uses `-include` to force-include our
  stdarg.h before any IRIX headers. This defines `_VA_LIST_` guard first.
- **struct msghdr**: IRIX default `_SGIAPI` msghdr lacks msg_control, msg_controllen, msg_flags.
  Override with `_XOPEN_SOURCE=500` to get XPG version (compat sys/socket.h does this).
- **complex.h**: IRIX complex.h requires `__c99` and conflicts with va_list. Use standalone
  compat complex.h that provides _Complex types without including IRIX header.
- **isblank()**: C99 function not in IRIX ctype.h. Compat ctype.h provides inline implementation.
- **sys/random.h / getrandom()**: Linux-specific. Compat header provides stub returning -1/ENOSYS.
- **strerror_r**: Not available on IRIX. Compat provides GNU-style (returns char*) implementation.
- **_rld_new_interface**: IRIX rld interface for dynamic loading. Stub needed for static linking.

### References
- [MIPS ELF Spec](https://irix7.com/techpubs/007-4658-001.pdf)
- [The IRIX LLVM Port - Part 1](https://www.aurxenon.io/posts/the-irix-llvm-port-part-1/)
- [The IRIX LLVM Port - Part 2](https://www.aurxenon.io/posts/the-irix-llvm-port-part-2/)

## rpmbuild Cross-Compilation Setup

The rpmbuild macros for cross-compilation are at `/opt/sgug-staging/rpmmacros.irix`.

Key macros that must be set:
- `__cc` - Path to irix-cc wrapper
- `__ld` - Path to irix-ld wrapper
- `_prefix` - /usr/sgug
- `_libdir` - /usr/sgug/lib32

### Staging Directory

Built libraries and headers go to `/opt/sgug-staging/usr/sgug/`:
- `bin/` - Toolchain wrappers (irix-cc, irix-ld)
- `lib32/` - Cross-compiled libraries
- `include/` - Headers (including dicl-clang-compat overrides)

### Build Dependencies

When building package B that depends on package A:
1. Build package A first using mogrix workflow
2. Install A's libraries/headers to staging: `rpm -ivh --prefix=/opt/sgug-staging A.rpm`
3. Now build package B - it will find A in staging

## Debugging Build Failures

### Common Issues

1. **Missing compat function**: Add to `compat/catalog.yaml` and create implementation in `compat/`
2. **Header conflict**: Create override in `cross/include/dicl-clang-compat/`
3. **Configure detects wrong feature**: Add configure flag to package rule
4. **Libtool issues**: May need `export_vars` rule to set LD explicitly
5. **Source number conflicts**: Specs with Source100+ conflict with mogrix compat sources - use spec_replacements to comment them out
6. **Missing function declarations**: IRIX strict C99 mode hides POSIX functions - add declarations to `compat/include/mogrix-compat/generic/` headers
7. **Missing .so symlinks**: After staging, create unversioned .so symlinks for cmake
8. **Missing pkg-config files**: Install -devel RPMs or create .pc files manually in staging
9. **Multiarch header dispatch**: Copy x86_64 headers to mips64 variants (lua, openssl)
10. **Library file patterns**: Use explicit patterns instead of wildcards in %files

### Viewing mogrix Transformations

```bash
# See what mogrix will change without converting
mogrix analyze <package>.src.rpm

# Convert with verbose output
mogrix convert <package>.src.rpm -v
```

### rpmbuild Tips for Cross-Compilation

1. **Use `--nodeps`**: rpmbuild checks BuildRequires (autoconf, make, etc.) which are host tools.
   Cross-compilation uses host tools, so skip these checks with `--nodeps`.

2. **RPM macros must be properly set**: The `__cc`, `__ld` etc. macros must be set so the spec
   file uses them. Some specs hardcode `gcc` - these need mogrix rules to fix.

3. **Arch mismatch warnings**: The RPM will show arch warnings because the host is x86_64 but
   binaries are MIPS. This is expected for cross-compilation.

4. **Fedora-specific macros**: Some specs use `%{__global_ldflags}`, `%{optflags}`, `%{_pkgdocdir}` etc.
   Common macros (`_pkgdocdir`, `_docdir`) are now injected by generic.yaml. Others may need
   spec_replacements to define or replace them.

5. **Shell compatibility**: rpmbuild uses `/bin/sh` (dash on Ubuntu) for scriptlets.
   - `pushd`/`popd` don't work - replace with POSIX subshell: `(cd dir && command)`
   - Brace expansion after variables doesn't work: `$VAR{a,b}` → use explicit paths

## Validated Build Order for tdnf

The following build order has been validated (2026-01-24):

```
1.  zlib       (no deps)
2.  bzip2      (no deps)
3.  popt       (no deps)
4.  openssl    (zlib)
5.  libxml2    (zlib)
6.  curl       (openssl, zlib)
7.  xz         (no deps)
8.  lua        (readline - static)
9.  file       (zlib)
10. rpm        (popt, lua, openssl, zlib, bzip2, xz, file)
11. libsolv    (zlib, bzip2, xz, libxml2, rpm)
12. tdnf       (libsolv, rpm, curl)
```

All packages produce valid ELF 32-bit MSB MIPS N32 binaries.

### Staging Dependencies

After building each package, stage it for dependent builds:
```bash
source .venv/bin/activate
mogrix stage package.mips.rpm
```

The stage command now handles everything automatically:
- **Auto-devel**: Automatically includes matching -devel packages (headers, .so symlinks, .pc files)
- **Multiarch headers**: Creates mips64 variants from x86_64 (luaconf-mips64.h, configuration-mips64.h)
- **No manual workarounds needed**: Just run `mogrix stage` and all dependencies are ready
