# Git / Makefile-based packages

**Problem: IRIX unistd.h includes getopt.h → struct option conflict**
IRIX `<unistd.h>` (line 392) includes `<getopt.h>`, which pulls in our compat getopt.h defining `struct option` (POSIX: name, has_arg, flag, val). Apps like git that define their own `struct option` with different fields get redefinition errors.
**Fix:** Use `_MOGRIX_NO_GETOPT_STRUCT_OPTION` guard. Add `-D_MOGRIX_NO_GETOPT_STRUCT_OPTION` to CFLAGS. Our compat getopt.h wraps struct option/getopt_long in `#ifndef _MOGRIX_NO_GETOPT_STRUCT_OPTION`.

**Problem: Makefile-based builds don't link compat library**
Mogrix injects compat via `export LIBS="-L$COMPAT_DIR -lmogrix-compat"`. Works for autoconf. Makefile-only builds (like git) ignore `LIBS`.
**Fix:** Use the package's own Makefile variables. Git uses `EXTLIBS += -L./mogrix-compat -lmogrix-compat`. Also check if the package has built-in compat (git has NO_MKDTEMP, NO_SETENV, NO_UNSETENV, NO_MEMMEM, NO_STRCASESTR, HAVE_GETDELIM, etc.).

**Problem: Linux auto-detection in cross builds (sysinfo, CLOCK_MONOTONIC, sync_file_range, PROCFS)**
Git's `config.mak.uname` detects Linux and adds `-DHAVE_SYSINFO` directly to BASIC_CFLAGS (not through ifdef). Setting `HAVE_SYSINFO=` in config.mak doesn't help.
**Fix:** prep_command: `sed -i '/HAVE_SYSINFO\|PROCFS_EXECUTABLE_PATH\|HAVE_PLATFORM_PROCINFO/d' config.mak.uname`. For `ifdef`-based vars, override with empty: `HAVE_CLOCK_GETTIME =`, `HAVE_CLOCK_MONOTONIC =`, `HAVE_SYNC_FILE_RANGE =`.

**Problem: IRIX atfork_parent/atfork_prepare name collision**
IRIX `<unistd.h>` declares `extern int atfork_parent(void (*)(int,int))`. Git's run-command.c defines `static void atfork_parent(struct atfork_state *)`. Link-time or compile-time conflict.
**Fix:** prep_command: `sed -i 's/atfork_parent/git_atfork_parent/g; s/atfork_prepare/git_atfork_prepare/g' run-command.c`

**Problem: IRIX fileno macro breaks with void* argument**
IRIX fileno is a macro. In http.c, `fileno(result)` where `result` is `void*` causes macro expansion to dereference void pointer.
**Fix:** prep_command: `sed -i 's/fileno(result)/(((FILE *)result)->_file)/g' http.c`

**Problem: dirname/basename live in libgen.so on IRIX, not libc**
Missing `dirname` and `basename` at link time.
**Fix:** Add `-lgen` to EXTLIBS or LDFLAGS. IRIX has `<libgen.h>` and both functions in `/usr/lib32/libgen.so`.

**Problem: pushd/popd in spec files (bash-isms)**
Fedora specs use `pushd`/`popd` which don't exist in `/bin/sh`.
**Fix:** spec_replacements: replace `pushd dir > /dev/null ... popd > /dev/null` with `(cd dir && ...)` subshells, or skip entirely if the operation isn't needed.

**Problem: Brace expansion in spec files**
Fedora specs use `{foo,bar}` brace expansion (bash-ism) in find commands, rm, etc.
**Fix:** spec_replacements: expand manually. E.g., `find %{buildroot}{%{_bindir},%{_libexecdir}}` → `find %{buildroot}%{_bindir} %{buildroot}%{_libexecdir}`.

**Problem: irix-cc wrapper misroutes -MM (dependency generation)**
Makefile-based builds that use `$(CC) -MM` for generating dependency files fail because the irix-cc wrapper doesn't recognize `-MM` as a preprocessing operation and falls through to compile+link mode.
**Fix:** Pre-create empty deps.mk and remove the regeneration rule from the Makefile: `touch src/deps.mk && sed -i '/^deps.mk:/,/>/d' src/Makefile`. See feh.yaml.

**Problem: _SC_HOST_NAME_MAX / HOST_NAME_MAX missing on IRIX**
Code using `sysconf(_SC_HOST_NAME_MAX)` or `HOST_NAME_MAX` fails. IRIX has `MAXHOSTNAMELEN` (256) but not the POSIX constant.
**Fix:** `sed -i '1i#ifndef HOST_NAME_MAX\n#define HOST_NAME_MAX 255\n#endif' src/file.c`. See feh.yaml.

**Problem: Make CFLAGS= on command line kills += additions**
When a Makefile uses `CFLAGS += -DPREFIX=... -DPACKAGE=...`, passing `CFLAGS=value` on the make command line overrides ALL `+=` additions, stripping crucial defines.
**Fix:** Use `export CFLAGS=value` (environment variable) instead of `make CFLAGS=value`. The env var allows `?=` to skip but `+=` to still append. See feh.yaml.

**Problem: IRIX scandir function pointer type mismatch**
IRIX `scandir()` uses `int (*)(dirent_t *)` instead of POSIX `int (*)(const struct dirent *)`. Causes incompatible-function-pointer-types errors.
**Fix:** Add `-Wno-incompatible-function-pointer-types` to CFLAGS. See feh.yaml.

**Problem: osdef.h socket prototype conflicts (screen)**
Generated `osdef.h` declares `connect()`/`bind()`/`accept()` with `int` length parameters, but IRIX socket headers use `socklen_t`. Conflicting type errors.
**Fix:** Run `make osdef.h` first, then sed to delete conflicting declarations, then run full `make`. See screen.yaml.

**Problem: %{_rundir} / %{_tmpfilesdir} macros undefined in cross-compile**
Systemd-specific RPM macros not defined in IRIX cross-compilation environment.
**Fix:** Replace `%{_rundir}/pkg` with `%{_localstatedir}/run/pkg`. Remove entire tmpfiles.d blocks. See screen.yaml.

**Problem: poll() conflicting types (vim)**
Source declares `poll(struct pollfd *, long, int)` but IRIX declares `poll(struct pollfd *, nfds_t, int)` where `nfds_t = unsigned long`.
**Fix:** `sed` to remove the poll declaration from the source, letting the IRIX system header provide it. See vim.yaml.

**Problem: sigvec() detection on IRIX (vim)**
IRIX has `sigvec()` but `struct sigvec` is behind `_BSD_SIGNALS` guards. Configure detects the function, but compilation fails with "incomplete type".
**Fix:** `ac_cv_func_sigvec=no` — makes the app use standard `signal()` path instead. See vim.yaml.

**Problem: -flto produces LLVM bitcode incompatible with IRIX linker**
Autotools `AX_APPEND_COMPILE_FLAGS` or configure adds `-flto` when CFLAGS is empty. LLVM bitcode objects are incompatible with ld.lld-irix-18.
**Fix:** Set CFLAGS to a non-empty value via `export_vars` so configure skips its SCROT_FLAGS/optimization block. See scrot.yaml.

**Problem: Hand-written specs don't export CC for cross-compilation**
`%configure` exports `CC="%{__cc}"` (the cross-compiler) automatically. But `%make_build` just runs `make` — it does NOT export CC. Makefile-based packages with hand-written specs (using `build_system: makefile` in upstream config) silently use the host `gcc` instead of the cross-compiler, producing host-arch binaries or link failures ("skipping incompatible" messages from ld).
**Fix:** Add a `spec_replacements` rule to inject `export CC="%{__cc}"` before `%make_build`:
```yaml
rules:
  spec_replacements:
  - pattern: '%make_build'
    replacement: "export CC=\"%{__cc}\"\nexport LD=\"%{__cc}\"\n%make_build"
```
**Affected packages:** ir8 (and any future hand-written Makefile spec)

**Problem: pkgconfig() first on multi-dep BuildRequires line (engine bug)**
`drop_buildrequires` can't remove `pkgconfig(foo)` when it's the first item on a multi-package BuildRequires line.
**Fix:** Use `spec_replacements` to remove the entire line before `drop_buildrequires` phase runs. See scrot.yaml.

**Problem: Meson cross-compilation for IRIX**
`%meson` macro doesn't exist on build host; meson cross-compile needs a cross file with compat lib/pthread link args.
**Fix:** Use explicit `meson setup _build --cross-file=...` with package-specific cross file. Copy `cross/meson-irix-cross.ini`, add link args. Replace `%meson_build`→`meson compile -C _build`, `%meson_install`→`DESTDIR=$RPM_BUILD_ROOT meson install -C _build --no-rebuild`. See p11-kit.yaml.

**Problem: CMAKE_ROOT resolution fails in mogrix bundles**
cmake resolves data directory by stripping `bin` suffix from exe_dir. Bundle binaries are in `_bin/` not `bin/`, so resolution produces wrong prefix.
**Fix:** Patch `Source/cmSystemTools.cxx` to add fallback: try `exe_dir/../share/cmake-<version>` when standard resolution fails. See specs/packages/cmake.spec.

**Problem: IRIX bsearch() crashes when nmemb=0**
IRIX libc's `bsearch()` does not handle `nmemb=0`. It computes `last = base + size * (nmemb - 1)` which underflows to `0xFFFFFFE0` when nmemb=0 and base=NULL (size * (0-1) wraps unsigned). The overflow check `sltu(last < base)` fails because `0xFFFFFFE0 >= 0` in unsigned comparison. Execution falls through to the search loop and dereferences a garbage pointer, causing SIGSEGV. This crashes GTK3 (property_cache bsearch with empty cache) and potentially any code that calls bsearch with an empty array.
**Fix:** `compat/stdlib/bsearch.c` added to `generic.yaml` `inject_compat_functions` — all packages get it in the executable. **However**, IRIX rld does NOT preempt executable symbols for shared library calls (unlike Linux). When libgtk-3.so calls bsearch, rld resolves it through libgtk-3.so's NEEDED chain → libc.so.1, never checking the executable. The real runtime fix is `libmogrix_compat.so` (in staging at `/opt/mogrix/lib32/`), preloaded via `_RLDN32_LIST=libmogrix_compat.so:DEFAULT` in bundle wrapper scripts. The bundler automatically includes this .so and sets the env var.

**Problem: IRIX rld does NOT preempt shared library symbols from executable**
On Linux, symbols in the main executable override same-named symbols in shared libraries (symbol interposition via PLT/GOT). On IRIX MIPS n32, rld resolves each shared library's symbols through its own NEEDED chain first (e.g. libgtk-3.so → libc.so.1) without checking the executable's .dynsym. To override libc functions called from shared libraries, use `_RLDN32_LIST=libfoo.so:DEFAULT` (n32 equivalent of `LD_PRELOAD`). The bundler includes `libmogrix_compat.so` in `_lib32/` and sets `_RLDN32_LIST` in all wrapper scripts.
**Fix:** `mogrix/bundle.py` copies `libmogrix_compat.so` from staging into every bundle's `_lib32/`. Wrapper templates set `_RLDN32_LIST=libmogrix_compat.so:DEFAULT`. The pruner always keeps this .so (hardcoded in `_prune_unused_libs`).

**Problem: Bundler missing unversioned soname symlinks**
When a library has an unversioned SONAME (e.g. `libz.so` from zlib-ng), the unversioned symlink (`libz.so` → `libz.so.1`) lives in the `-devel` RPM, which is excluded from bundles. At runtime, rld resolves the NEEDED `libz.so` to IRIX's ancient system `/usr/lib32/libz.so` instead of the bundled zlib-ng version. This causes ABI-incompatible link failures (e.g. libpng compiled against zlib-ng but running against IRIX system zlib).
**Fix:** Three changes in `mogrix/bundle.py`: (a) dependency resolution now checks mogrix-built RPMs before the IRIX sysroot, ensuring our libraries are preferred; (b) new `_create_soname_symlinks()` reads ELF SONAME headers and creates any missing unversioned symlinks in the bundle's `_lib32/` directory; (c) `_prune_unused_libs()` now walks symlink chains step-by-step to preserve intermediate links that would otherwise be pruned.
