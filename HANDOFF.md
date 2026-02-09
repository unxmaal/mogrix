# Mogrix Cross-Compilation Handoff

**Last Updated**: 2026-02-09 (session 11)
**Status**: 81 source packages cross-compiled (217 RPMs). C++ cross-compilation WORKING (groff). OpenSSH, aterm, tdnf all running on IRIX.

---

## CRITICAL WARNINGS

**NEVER install packages directly to /usr/sgug on the live IRIX system.** Always use `/opt/chroot` for testing.

**NEVER CHEAT.** Don't copy from SGUG-RSE. Don't fake Provides. Every FC40 package must be properly cross-compiled. Check `rules/methods/before-you-start.md` if stuck.

**Question SGUG-RSE assumptions.** We cross-compile with a full sysroot — prefer system libraries over bundled copies.

**NEVER use raw SSH to IRIX.** Always use MCP tools (`irix_exec`, `irix_copy_to`, `irix_read_file`, `irix_par`).

---

## IMMEDIATE NEXT: Choose Next Targets

34 packages have rules but no RPMs yet. Priorities:

**Autotools (ready to build):** gperf, jq, pcre, gd, gnutls (needs nettle+libtasn1 staged), libarchive, elfutils, expect, tk, gtk2

**Meson (need `%meson` macro):** cairo 1.18, harfbuzz, pango, glib2, p11-kit. Pixman was already built with meson.

**Still blocked:** lolcat (wchar I/O), bc (host binary bootstrap), xclip (IRIX Xmu too old)

---

## Recent Session (11): groff + C++ Milestone

### groff 1.23.0 — First C++ package!
Built with `irix-cxx` (clang++ + GCC 9 libstdc++). Key fixes:
1. **c++config.h**: Disabled `_GLIBCXX_USE_C99_MATH_TR1` and `_GLIBCXX_USE_STD_SPEC_FUNCS` (IRIX libm lacks C99 math)
2. **wchar_t keyword**: `-D_WCHAR_T` in irix-cxx prevents IRIX typedef
3. **Doc generation**: Override all PROCESSED*/GENERATED* make vars (can't run MIPS groff on build host)
4. **Bashisms**: Brace expansion, pushd/popd expanded manually
5. **update-alternatives**: Drop `Requires(post/preun/postun)` — doesn't exist on IRIX
6. **Dropped subpackages**: doc, x11 + install_cleanup for leftover files

Also completed: libxslt, giflib, libstrophe (sockaddr_storage fix in dicl-clang-compat/sys/socket.h).

### Sessions 7-10 Summary
- **Session 10**: C++ cross-compilation infrastructure (irix-cxx, CRT objects, c++config.h patches)
- **Session 9**: batch-build of 68 packages, 8 new packages built (figlet, sl, time, cmatrix, gmp, mpfr, hyphen, libevent)
- **Session 8**: nano, rsync, unzip, zip. Long double crash pattern (`ac_cv_type_long_double_wider: "no"`)
- **Session 7**: `mogrix batch-build` command implemented

### Sessions 4-6 Summary
- **Session 6**: tdnf Error(1602) root cause (IRIX pre-C99 vsnprintf). cmake cross-compilation regression fixes.
- **Session 5**: OpenSSH 9.6p1 (R_MIPS_REL32 dispatch pattern, fork-mode broken, debug-mode works)
- **Session 4**: aterm (first X11 app), `-rdynamic` filter, tdnf repo setup, roadmap performance fix

---

## Package Status

**Total: 81 source packages (217 RPMs)**

### Phases 1-4c: 41 packages (ALL INSTALLED)
Bootstrap (14) + system libs (2) + build tools (6) + crypto (7) + text tools (3) + utilities (9).

### Phase 5+: 40 packages

| Package | Version | Status | Key Fixes |
|---------|---------|--------|-----------|
| pcre2 | 10.42 | INSTALLED | JIT/sealloc blocks, inttypes.h C99 |
| symlinks | 1.7 | INSTALLED | CC override for raw Makefile |
| tree-pkg | 2.1.0 | INSTALLED | Clean build |
| oniguruma | 6.9.9 | INSTALLED | fix-libtool-irix.sh |
| libffi | 3.4.4 | INSTALLED | -fno-integrated-as (LLVM #52785) |
| tcl | 8.6.13 | INSTALLED | tcl_cv_sys_version=IRIX64-6.5 |
| flex | 2.6.4 | INSTALLED | nls-disabled, inline basename |
| chrpath | 0.16 | INSTALLED | byteswap.h compat |
| libpng | 1.6.40 | INSTALLED | pngcp/pngfix stubs |
| bison | 3.8.2 | INSTALLED | gl_cv_header_working_stdint_h |
| libunistring | 1.1 | BUILT | Clean build |
| gettext | 0.22.5 | INSTALLED | %files substring collision, dep fixes |
| zstd | 1.5.5 | INSTALLED | Makefile CC/AR/RANLIB, pthread, LDFLAGS |
| fontconfig | 2.15.0 | INSTALLED | Host gperf, expat, __sync atomics |
| freetype | 2.13.2 | INSTALLED | Explicit Provides for ISA + .so |
| expat | 2.6.0 | STAGED | man page touch, --without-docbook |
| nettle | 3.9.1 | STAGED | --enable-mini-gmp |
| libtasn1 | 4.19.0 | STAGED | GTKDOCIZE=true |
| fribidi | 1.0.13 | STAGED | autotools path, utime rename |
| libjpeg-turbo | 3.0.2 | STAGED | raw cmake, SIMD off |
| pixman | 0.43.0 | STAGED | meson cross file |
| uuid | 1.6.2 | STAGED | ac_cv_va_copy=C99 |
| aterm | 1.0.1 | INSTALLED | First X11 app. Sysroot autodetect |
| openssh | 9.6p1 | INSTALLED | R_MIPS_REL32 dispatch, debug-mode only |
| unzip | 6.0 | INSTALLED | Simple build |
| zip | 3.0 | INSTALLED | Simple build |
| nano | 7.2 | INSTALLED | -lgen for dirname/basename |
| rsync | 3.2.7 | INSTALLED | Long double crash fix |
| figlet | 2.2.5 | BUILT | Makefile CC injection |
| sl | 5.02 | BUILT | Makefile + ncurses |
| time | 1.9 | BUILT | Drop texinfo/gnupg2 |
| cmatrix | 2.0 | BUILT | Drop help2man/console-setup |
| gmp | 6.2.1 | BUILT | strdup compat |
| mpfr | 4.2.1 | BUILT | --disable-thread-safe |
| hyphen | 2.8.8 | BUILT | pushd/popd → cd |
| libevent | 2.1.12 | BUILT | strsep compat |
| libxslt | 1.1.39 | BUILT | Drop python bindings |
| giflib | 5.2.2 | INSTALLED | cmake cross-build |
| libstrophe | 0.13.1 | INSTALLED | res_query, sockaddr_storage |
| groff | 1.23.0 | INSTALLED | First C++ package! |

---

## /usr/sgug Swap Procedure

**Safety invariant**: At least one working sshd must be listening at all times.

```bash
# 1. Backup from existing SSH session:
cd /opt && tar cf sgug-live-backup.tar /usr/sgug

# 2. Start test sshd from chroot (must use -d, fork mode broken):
chroot /opt/chroot /bin/sh -c 'LD_LIBRARYN32_PATH=/usr/sgug/lib32 /usr/sgug/sbin/sshd -d -e -p 2222'

# 3. Verify port 2222 works, then from that session:
mv /usr/sgug /usr/sgug.old && cp -a /opt/chroot/usr/sgug /usr/sgug

# 4. Rollback if needed (from console):
mv /usr/sgug /usr/sgug.new && mv /usr/sgug.old /usr/sgug
```

---

## Environment

| Purpose | Path |
|---------|------|
| Mogrix project | `/home/edodd/projects/github/unxmaal/mogrix/` |
| Cross toolchain | `/opt/cross/bin/` |
| Staging area | `/opt/sgug-staging/usr/sgug/` |
| IRIX sysroot | `/opt/irix-sysroot/` |
| IRIX host | `192.168.0.81` (use MCP, not SSH) |
| IRIX chroot | `/opt/chroot` (bootstrapped) |
| Original SRPMs | `~/mogrix_inputs/SRPMS/` |
| Converted SRPMs | `~/mogrix_outputs/SRPMS/` |
| Built RPMs | `~/mogrix_outputs/RPMS/` |

### Workflow
```bash
uv run mogrix fetch <package> -y
uv run mogrix convert ~/mogrix_inputs/SRPMS/<pkg>-*.src.rpm
uv run mogrix build ~/mogrix_outputs/SRPMS/<pkg>-*-converted/<pkg>-*.src.rpm --cross
uv run mogrix stage ~/mogrix_outputs/RPMS/<pkg>*.rpm
# Install on IRIX via MCP: irix_copy_to + irix_exec "rpm -Uvh"
```

---

## Known Issues

- **`%zu` format specifier** crashes IRIX libc — use `%u` for size_t
- **Volatile function pointer static initializers** crash on IRIX rld — use dispatch functions
- **fopencookie** crashes — use funopen instead
- **`--export-dynamic`** crashes IRIX rld with large symbol tables
- **R_MIPS_REL32 relocations** from function pointers in static data — use switch/strcmp dispatch
- **Long double (128-bit)** not supported on MIPS n32 — `__extenddftf2` crashes. Fix: `ac_cv_type_long_double_wider: "no"`
- **openssh fork-mode broken** — use `-d` (debug/no-fork) in a loop
- **Man pages not compressed** — brp scripts disabled; use `*.1*` not `*.1.gz` in %files
- **Explicit Provides required** — `AutoProv: no` in rpmmacros.irix; every .so needs explicit Provides
- **C++ c++config.h patches** — staging only, not in mogrix source. Must disable `_GLIBCXX_USE_C99_MATH_TR1` and `_GLIBCXX_USE_STD_SPEC_FUNCS`
- **autoreconf undoes prep_commands** — patches to configure in %prep get overwritten. Use spec_replacements after autoreconf
- **sockaddr_storage** hidden by `_XOPEN_SOURCE=500` in dicl-clang-compat — defined explicitly in overlay

---

## Reference Documents

| Document | Purpose |
|----------|---------|
| `rules/INDEX.md` | Rules reference, pattern catalog, invariants |
| `rules/methods/before-you-start.md` | Checklist before debugging |
| `rules/generic.yaml` | Universal rules for all packages |
| `compat/catalog.yaml` | Compat function registry |
| `plan.md` | Project plan and architecture |
