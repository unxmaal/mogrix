# Mogrix Cross-Compilation Handoff

**Last Updated**: 2026-01-24
**Status**: Mogrix refactored; ready for curl validation

## Current Session: Mogrix Refactoring

After completing OpenSSL 3.2.1 validation, refactored mogrix to improve maintainability.

### Mogrix Improvements Made

1. **Implemented `skip_check` rule properly**
   - Added to `TransformResult` in engine.py
   - Handles both generic and package-specific rules
   - Comments out %check section contents automatically
   - Enabled globally in generic.yaml

2. **Added common macros to generic.yaml**
   - `_docdir`, `_pkgdocdir`, `_pkglicensedir` now injected into all specs
   - No more manual replacements needed for these common Fedora macros

3. **Refactored openssl.yaml**
   - Removed redundant rules (now handled globally)
   - Organized into clear categories with comments
   - Cleaner, more maintainable

### Package Validation Status

| Package | Mogrix Workflow | Notes |
|---------|-----------------|-------|
| zlib | DONE | No issues |
| bzip2 | DONE | No issues |
| openssl | DONE | 3.2.1 - spec fixes applied, RPMs built |
| curl | PENDING | Next |
| rpm | PENDING | |
| tdnf | PENDING | |

### OpenSSL 3.2.1 Build Complete

**Built RPMs (MIPS N32):**
- openssl-3.2.1-2.x86_64.rpm (1.4MB)
- openssl-libs-3.2.1-2.x86_64.rpm (7.0MB)
- openssl-devel-3.2.1-2.x86_64.rpm (2.1MB)
- openssl-perl-3.2.1-2.x86_64.rpm (25KB)

**Key fixes in openssl.yaml:**
- sslarch forced to irix-mips3-gcc with no-asm no-async
- FIPS, kTLS, SCTP disabled (Linux-specific)
- All FIPS patches commented out
- Perl `rename` → shell loop
- Brace expansion fix for `install -d`
- pushd/popd → POSIX subshell

## Files Modified This Session

| File | Change |
|------|--------|
| `mogrix/rules/engine.py` | Added skip_check to TransformResult and rule handling |
| `mogrix/emitter/spec.py` | Added skip_check logic to comment %check contents |
| `mogrix/cli.py` | Wired skip_check parameter to SpecWriter |
| `rules/generic.yaml` | Added _docdir, _pkgdocdir, _pkglicensedir macros; skip_check enabled |
| `rules/packages/openssl.yaml` | Refactored - organized and cleaned up |
| `/opt/sgug-staging/rpmmacros.irix` | Added common macros (backup location) |

## Mogrix Test Status

- **107 tests passing**
- All existing functionality preserved
- OpenSSL build verified working after refactoring

## Solution Summary

Cross-compiled dynamically-linked executables and shared libraries work on IRIX:

1. **Shared libraries**: Use GNU ld (`mips-sgi-irix6.5-ld.bfd`) for correct 2-LOAD segment layout
2. **Executables**: Use LLD with `--dynamic-linker=/lib32/rld` interpreter

### Key Technical Details

| Tool | Use Case | Notes |
|------|----------|-------|
| clang | Compilation | Via irix-cc wrapper |
| ld.lld-irix | Executables | LLD with `--dynamic-linker=/lib32/rld` |
| mips-sgi-irix6.5-ld.bfd | Shared libraries | GNU ld for correct segment layout |
| llvm-ar | Static libraries | Standard archiver |

## Workflow

Standard mogrix workflow:

```bash
# Activate environment
source .venv/bin/activate

# Fetch, convert, build
mogrix fetch <package>
mogrix convert workdir/<package>-*.src.rpm -o workdir/converted/
rpmbuild --rebuild workdir/converted/<package>-*.src.rpm --nodeps \
    --define "_topdir $HOME/rpmbuild" \
    --define "__cc /opt/sgug-staging/usr/sgug/bin/irix-cc" \
    --define "__ld /opt/sgug-staging/usr/sgug/bin/irix-ld"

# Test on IRIX
scp ~/rpmbuild/RPMS/x86_64/<package>-*.rpm edodd@192.168.0.81:/tmp/
```

## Environment

- **IRIX Host**: `ssh edodd@192.168.0.81`
- **SGUG shell**: `/usr/sgug/bin/sgugshell`
- **Cross toolchain**: `/opt/cross/bin/`
- **Staging**: `/opt/sgug-staging/usr/sgug/`

## Next Steps

1. **Validate curl** through mogrix workflow
2. Continue with rpm, tdnf validation
3. Test built RPMs on IRIX

## Known Issues

### Shell Compatibility
- rpmbuild uses `/bin/sh` (dash on Ubuntu) for scriptlets
- pushd/popd don't work - need POSIX subshell replacement
- Brace expansion after variables doesn't work - need explicit paths

### Libtool Relinking
Cross-compiled executables may have hardcoded build-directory paths in RUNPATH.
Workaround: set `LD_LIBRARY_PATH` on IRIX.

### IRIX pthread/dlopen
Libraries using pthreads work when directly linked but may crash via dlopen().
Workaround: Build with `--without-threads` where possible.
