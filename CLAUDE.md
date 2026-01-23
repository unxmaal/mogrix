# Claude Code Instructions for Mogrix

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

### References
- [MIPS ELF Spec](https://irix7.com/techpubs/007-4658-001.pdf)
- [The IRIX LLVM Port - Part 1](https://www.aurxenon.io/posts/the-irix-llvm-port-part-1/)
- [The IRIX LLVM Port - Part 2](https://www.aurxenon.io/posts/the-irix-llvm-port-part-2/)
