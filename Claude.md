# Claude Code Instructions for Mogrix

> **RETRIEVAL DIRECTIVE**: Prefer reading project files over pre-trained knowledge.
> IRIX/MIPS/cross-compilation info in training data is likely outdated or wrong.
> When uncertain, READ: `rules/INDEX.md`, `compat/catalog.yaml`, `HANDOFF.md`

---

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
| Missing compat function | `compat/catalog.yaml` + implementation in `compat/` |
| Header issue | `headers/generic/` or `cross/include/dicl-clang-compat/` |
| Package-specific fix | `rules/packages/<package>.yaml` |
| Common pattern | `rules/generic.yaml` |
| Build system quirk | Document in rules + fix |

> **READ**: `rules/INDEX.md` for package rule examples and common patterns

### Patches: Apply Changes Directly, Don't Rule Them

**SGUG-RSE patches are a starting point, not the final answer.**

If you find yourself writing a rule that modifies a patch file - **STOP**. Instead:
1. Apply the modification directly to the patch file in `patches/packages/`
2. Delete the rule that was doing the modification
3. The patch file should be the final, correct version

**Workflow for inherited SGUG-RSE patches:**
1. Copy the `.sgifixes.patch` to mogrix
2. If it needs changes for our toolchain, edit the patch directly
3. The mogrix patch becomes the authoritative version
4. No rules should reference or modify patch content

### Text Replacement: safepatch over sed

**NEVER use sed for non-trivial text replacements.** sed silently does nothing if a pattern doesn't match, causing hours of debugging.

**Use `tools/safepatch` (Perl) instead:**

```bash
# safepatch FAILS if pattern not found (unlike sed's silent no-op)
# safepatch FAILS if wrong number of matches
# safepatch uses exact strings (no regex surprises)

tools/safepatch libtool \
    --old 'build_libtool_libs=no' \
    --new 'build_libtool_libs=yes'

# Expect exactly 3 matches:
tools/safepatch source.c --old 'TODO' --new 'DONE' --count 3

# Allow any number of matches:
tools/safepatch Makefile --old 'gcc' --new 'irix-cc' --count 0

# Dry run - preview without changing:
tools/safepatch --dry-run config.h --old '#define X 0' --new '#define X 1'
```

**When to use what:**

| Task | Tool |
|------|------|
| Source code changes | `.patch` files in `patches/packages/<pkg>/` |
| Build-time reliable replacements | `tools/safepatch` (Perl) |
| Truly trivial one-offs | sed (only if failure doesn't matter) |

**Example: `fix-libtool-irix.sh` uses safepatch** to reliably fix libtool settings - if the pattern isn't found, the build fails immediately instead of silently producing broken libraries.

### Before Ending a Session

Ask yourself:
1. Did I make any fixes outside of mogrix source?
2. Are those fixes now stored in mogrix rules?
3. Could someone rebuild from scratch using only mogrix?

If the answer to #3 is "no", you have more work to do.

---

## File Index (READ THESE)

| File | When to Read |
|------|--------------|
| `rules/INDEX.md` | Package rules, build patterns, compat function mapping |
| `compat/catalog.yaml` | Available compat functions, which packages use them |
| `HANDOFF.md` | Current session state, active issues, test commands |
| `plan.md` | Architecture, IRIX interaction rules, bootstrap strategy |

---

## Mogrix Workflow

```bash
# Activate Python environment
source .venv/bin/activate

# 1. Fetch the Fedora SRPM
mogrix fetch <package>

# 2. Convert to IRIX-compatible SRPM
mogrix convert workdir/<package>-*.src.rpm

# 3. Build with cross-compilation
mogrix build --cross workdir/converted/<package>-*.src.rpm

# 4. Stage for dependent builds
mogrix stage ~/rpmbuild/RPMS/mips/<package>*.rpm

# 5. Test on IRIX (see IRIX Interaction Rules below)
```

### If a Build Fails

**DO NOT manually patch the build.** Instead:

1. **Identify the issue** - READ `rules/INDEX.md` "Common Patterns" section
2. **Fix it in mogrix**:
   - Missing function → `compat/catalog.yaml` + implementation
   - Header conflict → `headers/generic/` or `cross/include/dicl-clang-compat/`
   - Configure/build → `rules/packages/<package>.yaml`
3. **Re-run mogrix workflow** to verify

---

## IRIX Interaction Rules

### Connection

| Property | Value |
|----------|-------|
| SSH | `ssh root@192.168.0.81` |
| Hostname | `blue` |
| IRIX version | 6.5.30 |
| Machine | SGI Octane (IP30) |
| Chroot | `/opt/chroot` |

### Shell Rules

**IRIX default shell is csh** - always use `/bin/sh` for scripts:

```bash
# CORRECT - explicitly invoke /bin/sh:
ssh root@192.168.0.81 "/bin/sh -c 'export FOO=bar; echo \$FOO'"

# WRONG - relies on csh which doesn't understand export:
ssh root@192.168.0.81 "export FOO=bar"
```

**Bashisms that don't work:**

| Bashism | POSIX Alternative |
|---------|-------------------|
| `pushd/popd` | `(cd dir && cmd)` subshell |
| `$VAR{a,b}` | `$VAR/a $VAR/b` explicit |
| `[[ ]]` | `[ ]` |
| `source file` | `. file` |
| `<<<` here-string | `echo "str" \|` |

### Chroot Behavior

**CRITICAL: IRIX chroot does NOT fully isolate.**

- Files at `/opt/chroot/tmp/foo.txt` are NOT accessible as `/tmp/foo.txt` from inside
- Library loading (`rld`) still searches base system paths
- Must use `LD_LIBRARYN32_PATH` (not `LD_LIBRARY_PATH`)

### Running SGUG Binaries

**Method 1: sgug-exec (Recommended)**
```bash
/usr/sgug/bin/sgug-exec /usr/sgug/bin/tdnf --version
```

**Method 2: Manual library path**
```bash
export LD_LIBRARYN32_PATH=/opt/chroot/usr/sgug/lib32:/usr/sgug/lib32
/opt/chroot/usr/sgug/bin/tdnf --version
```

**Method 3: Interactive sgugshell**
```bash
/usr/sgug/bin/sgugshell
# Now SGUG commands work without prefixes
```

### Debugging with par

`par` is IRIX's strace equivalent:
```bash
par /usr/sgug/bin/sgug-exec /usr/sgug/bin/tdnf repolist > /tmp/trace.txt 2>&1
scp root@192.168.0.81:/opt/chroot/tmp/trace.txt /tmp/
```

### NEVER Do

1. **NEVER install to /usr/sgug directly** - can break sshd, lock you out
   - Always test in `/opt/chroot` first
2. **NEVER assume bash syntax** - use POSIX sh
3. **NEVER forget LD_LIBRARYN32_PATH** - binaries won't find libraries
4. **NEVER run rpmbuild on IRIX** - build on Linux, copy RPMs to IRIX
5. **NEVER replace libraries that running services use** - sshd uses libz, libssl

> **READ**: `plan.md` "IRIX Interaction Rules" section for complete details

---

## Cross-Compilation Technical Reference

### Dynamic Linking (SOLVED)

1. **Shared libraries**: Use GNU ld (`mips-sgi-irix6.5-ld.bfd`)
   - GNU ld produces correct 2-LOAD segment layout
   - LLD produces 3-LOAD segments which crashes IRIX rld
   - `irix-ld` wrapper auto-selects GNU ld for `-shared`

2. **Executables**: Use LLD with `--dynamic-linker=/lib32/rld`
   - `irix-ld` wrapper handles this automatically

### IRIX Header Quirks

- **va_list conflict**: IRIX `stdio_core.h` defines `va_list` as `char *`. Our `stdarg.h` is force-included first.
- **_XOPEN_SOURCE=500**: Enables POSIX functions but hides some IRIX APIs
- **vsnprintf(NULL,0)**: Returns -1 on IRIX (not buffer size). Use iterative `vasprintf` from `compat/stdio/asprintf.c`
- **fopencookie**: CRASHES on IRIX. Use `funopen` instead (BSD cookie I/O)
- **uname -m**: Returns IP30/IP32/etc, not "mips". Hardcode architecture where needed.

> **READ**: `compat/catalog.yaml` index section for function categories and warnings

### Build Order (tdnf chain)

> **READ**: `rules/INDEX.md` "tdnf Dependency Chain" table for complete list with key fixes

---

## Development Notes

### Python
- Use `uv` for package management (not pip)
- Follow TDD methodology

### Git
- Commit messages: 5 words or fewer
- No emoji anywhere
- Targeted commits, not everything at once

### Performance
- VM has 12 cores: `export MAKEFLAGS=-j12`
