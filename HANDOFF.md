# Mogrix Cross-Compilation Handoff

**Last Updated**: 2026-02-10 (session 18 continued)
**Status**: 88 source packages cross-compiled (240+ RPMs). **glib2 (first meson package!) + bitlbee (IRC gateway) + bitlbee-discord plugin built and bundled for IRIX!** bitlbee connects via IRC on port 6667 — users can chat on XMPP/Jabber, Twitter, and Discord from any IRC client (weechat already shipping). Meson cross-compilation UNBLOCKED. C++ cross-compilation WORKING (groff). OpenSSH, aterm, tdnf, weechat, st all running on IRIX.

---

## CRITICAL WARNINGS

**NEVER install packages directly to /usr/sgug on the live IRIX system.** Always use `/opt/chroot` for testing.

**NEVER CHEAT.** Don't copy from SGUG-RSE. Don't fake Provides. Every FC40 package must be properly cross-compiled. Check `rules/methods/before-you-start.md` if stuck.

**Question SGUG-RSE assumptions.** We cross-compile with a full sysroot — prefer system libraries over bundled copies.

**NEVER use raw SSH to IRIX.** Always use MCP tools (`irix_exec`, `irix_copy_to`, `irix_read_file`, `irix_par`).

---

## IMMEDIATE NEXT

**bitlbee bundle is SHIPPING with Discord support** — IRC gateway running on IRIX with discord.so plugin. Users connect with weechat (already shipping) to get XMPP/Jabber, Twitter, and Discord access.

**glib2 UNBLOCKS meson packages** — first meson cross-compilation working! Pattern established in `rules/packages/glib2.yaml` + `cross/meson-irix-cross.ini`.

Priorities:
- **Ship bitlbee + weechat + st tarballs** to community — ALL READY
- **More bundle candidates (already built):** nano, groff, openssh, aterm
- **Meson packages NOW UNBLOCKED:** cairo 1.18, harfbuzz, pango, p11-kit (glib2 pattern established)
- **Autotools (ready to build):** gperf, jq, pcre, gd, libarchive, elfutils, expect, tk, gtk2
- **Still blocked:** lolcat (wchar I/O), bc (host binary bootstrap), xclip (IRIX Xmu too old)
- **gnutls-utils installs libtool wrapper scripts** instead of real binaries (certtool, gnutls-cli etc). Needs fix-libtool-irix.sh or similar.
- **Implement `extra_cflags`** in rules engine (currently validated but dead code)
- **GLib warnings on IRIX**: `g_hash_table_lookup: assertion 'hash_table != NULL'` during bitlbee startup — non-fatal but indicates GLib quark table initialization issue. May affect other glib2-dependent apps.

---

## Recent Session (18): glib2 (meson) + bitlbee (IRC gateway)

### glib2 2.80.0 — First meson cross-compilation!
**THE MESON BLOCKER IS DEAD.** glib2 was listed as blocked since session 7. Now built with raw meson commands (replacing `%meson`/`%meson_build` RPM macros).

Key fixes (many build iterations):
- **SGUG-RSE patch rebased** from 2.62.6 → 2.80.0 (42KB, 27 file diffs). Selective application — reverted ALL gsocket.c XPG5 hacks (our `_XOPEN5` mode already gives modern msghdr). Added fdopendir guard (`#if defined(AT_FDCWD) && !defined(__sgi)`).
- **Meson `-Werror` + `__mips` redefinition**: All meson compile checks fail because `__mips` redefinition warning + `-Werror`. Fix: `-Wno-macro-redefined` in `cross/meson-irix-cross.ini` c_args.
- **Meson cpp removal**: `cpp = 'irix-cc'` in cross-file caused .cpp link tests to fail (irix-cc is C-only). Removed `cpp` entry.
- **Python packaging module**: meson's uv-installed Python lacks `packaging`. Fix: `uv pip install --python /path/to/meson/bin/python packaging`.
- **gnulib IRIX frexp assumptions**: gnulib marks IRIX frexp as "broken beyond repair". Patched meson.build to mark as working.
- **strnlen compat**: New function (`compat/string/strnlen.c`), added to catalog + header.
- **NLS disabled cleanup**: Extensive spec_replacements to remove gettext, gi-tools, systemtap, man pages, girepository from %files + install_cleanup to remove files meson still installs.
- Explicit Provides: libglib-2.0.so.0, libgobject-2.0.so.0, libgio-2.0.so.0, libgmodule-2.0.so.0, libgthread-2.0.so.0

**Meson pattern** (reusable for future packages):
```yaml
spec_replacements:
  - pattern: "%meson ... %meson_build"
    replacement: |
      MOGRIX_CROSS=/path/to/cross/meson-irix-cross.ini
      meson setup _build --cross-file=$MOGRIX_CROSS --prefix=... [options]
      meson compile -C _build
  - pattern: "%meson_install"
    replacement: |
      DESTDIR=$RPM_BUILD_ROOT meson install -C _build --no-rebuild
```

### bitlbee 3.6 — IRC to chat networks gateway
Custom configure (not autotools, not meson). Disabled purple (needs dbus+gtk2), OTR (needs libotr), systemd.

Key fixes:
- **`%zu`/`%zd` format specifiers**: 10+ occurrences sed'd to `%u`/`%d` (IRIX libc crashes on %z). MIPS n32: size_t = unsigned int.
- **`sa_len` macro collision**: IRIX `<sys/socket.h>` defines `#define sa_len sa_union.sa_generic.sa_len2`. Local variable `sa_len` in dcc.c renamed to `saddr_len`.
- **GNU ld `-r` vs LLD incompatibility**: GNU ld `ld -r` produces symbols with binding 0 (STB_LOCAL) that LLD rejects as "invalid binding". Fix: use LLD for both `ld -r` and final link (`LD=ld.lld-irix-18`).
- **libgcrypt-config not in PATH**: bitlbee configure uses `$(libgcrypt-config --libs)` which silently fails. Fix: inject `-lgcrypt` into EFLAGS via Makefile.settings.
- **strcasestr compat**: IRIX libc lacks strcasestr. Added to `inject_compat_functions`.
- **bcond flipping for OTR**: `%bcond_without otr` → `%bcond_with otr` cleanly disables OTR.
- **Brace expansion bashisms**: Fixed in %prep (`touch`), %install (`mkdir`), %files (`%doc`).

### New compat function: strnlen
- `compat/string/strnlen.c`: Bounded string length (POSIX.1-2008)
- Added to `compat/catalog.yaml` and `compat/include/mogrix-compat/generic/string.h`

### Files created/modified
| File | Action |
|------|--------|
| `rules/packages/glib2.yaml` | REWRITTEN — full meson cross-compilation rules (178 lines) |
| `rules/packages/bitlbee.yaml` | CREATED — custom configure + systemd removal |
| `patches/packages/glib2/glib2.sgifixes.patch` | REWRITTEN — rebased from 2.62.6 to 2.80.0 |
| `cross/meson-irix-cross.ini` | MODIFIED — added -Wno-macro-redefined, removed cpp |
| `compat/string/strnlen.c` | CREATED — new compat function |
| `compat/catalog.yaml` | MODIFIED — added strnlen entry |
| `compat/include/mogrix-compat/generic/string.h` | MODIFIED — added strnlen declaration |

### bitlbee verified on IRIX
```
$ /opt/mogrix-apps/bin/bitlbee -V
BitlBee 3.6
API version 030600
Configure args: ...--jabber=1 --twitter=1 --purple=0 --otr=0 --pie=0

$ echo -e "NICK test\r\nUSER test 0 * :Test\r\n" | nc 192.168.0.81 6668
:localhost.localdomain 001 test :Welcome to the BitlBee gateway, test
```

### Session 18 continued: bitlbee-discord plugin

**discord.so plugin built and bundled.** Built manually from GitHub (sm00th/bitlbee-discord), not via mogrix SRPM flow.

Key discoveries and changes:
- **irix-ld updated**: `--export-dynamic` is now conditional via `MOGRIX_ALLOW_EXPORT_DYNAMIC` env var. Without the env var set, `--export-dynamic` is silently dropped (previous behavior). With `MOGRIX_ALLOW_EXPORT_DYNAMIC=1`, it passes through to the linker.
- **`--dynamic-list` for selective symbol export**: `--export-dynamic` with 887 symbols crashes rld SIGSEGV (known pattern: >468 entries). `--dynamic-list` with 48 listed symbols (365 total dynamic symbols) works fine. This is the correct approach for plugin dlopen support.
- **New files**: `cross/bitlbee-plugin-symbols.list` (dynamic-list format for bitlbee plugin symbols), `cross/bitlbee-plugin-symbols.ver` (unused version script alternative)
- **Bundle**: `bitlbee-3.6-14c-irix-bundle.tar.gz` with discord.so plugin included
- **Wrapper updated**: sed replaces `__MOGRIX_BUNDLE_DIR__` placeholder in `bitlbee.conf` for `PluginDir` at runtime, so plugins are found at the correct bundle-relative path

### Files created/modified (session 18 continued)
| File | Action |
|------|--------|
| `cross/bin/irix-ld` | MODIFIED — `--export-dynamic` now conditional on `MOGRIX_ALLOW_EXPORT_DYNAMIC` env var |
| `cross/bitlbee-plugin-symbols.list` | CREATED — dynamic-list of 48 symbols needed by discord.so |
| `cross/bitlbee-plugin-symbols.ver` | CREATED — version script alternative (unused) |

---

## Recent Session (17): st (suckless terminal) — X11 dependency chain

### 3-package build chain: libXrender → libXft → st
Built the complete Xft font rendering stack for IRIX:
1. **libXrender 0.9.11** — X Render Extension client library. Created handcrafted x11.pc, xext.pc, xproto.pc, renderproto.pc for IRIX native X11 libs. Installed xorgproto Render headers manually (xorgproto uses meson, can't RPM-build it).
2. **libXft 2.3.8** — X FreeType library. Depends on libXrender + fontconfig + freetype (all staged). Staged fontconfig-devel first.
3. **st 0.9** — suckless terminal emulator. Plain Makefile (not autotools). First terminal with Xft antialiased font rendering on IRIX.

### New compat functions: openpty + pselect
- **openpty()** (`compat/stdlib/openpty.c`): Wraps IRIX `_getpty()` which returns a pts path string (vs Linux openpty which returns fd pair). Creates master via open, slave via `_getpty()`.
- **pselect()** (`compat/stdlib/pselect.c`): Wraps `select()` + `sigprocmask()` for signal mask support.
- Header: `compat/include/mogrix-compat/generic/pty.h`

### IRIX X11R6.3 compatibility patterns (NEW)
IRIX ships X11R6.3. Modern X11R6.4+ APIs missing:
- `XICCallback` → use `XIMCallback` (sed global replace)
- `XSetIMValues()` → `#define XSetIMValues(...) 0` (no-op)
- `Xutf8TextListToTextProperty()` → `XmbTextListToTextProperty()` (compound text encoding)
- `XUTF8StringStyle` → `XCompoundTextStyle`
- `xicdestroy` callback: return type `int` → `void` (matches XIMProc signature)

### _XOPEN_SOURCE=600 hides IRIX definitions (NEW pattern)
On IRIX, `struct winsize`, `TIOCSWINSZ`, `TIOCGWINSZ` in `<sys/termios.h>` are gated by `(_NO_POSIX && _NO_XOPEN4 && _NO_XOPEN5) || _ABIAPI`. Setting `_XOPEN_SOURCE=600` makes `_NO_XOPEN5=0`, hiding them. Fix: remove `-D_XOPEN_SOURCE=600`; `_SGI_SOURCE` (from cross-compiler) provides all POSIX/XSI functions.

### Makefile builds need compat BEFORE make (NEW pattern)
For autotools packages, mogrix injects compat build between configure and make. For plain Makefile packages (st, figlet), compat must be built explicitly in the spec_replacement BEFORE the make command, and linked via LDFLAGS.

### PKG_CONFIG_SYSROOT_DIR for non-autotools builds
`rpmmacros.irix` exports `PKG_CONFIG_SYSROOT_DIR` inside `%configure`, but custom Makefile builds don't use `%configure`. Must export it explicitly in the spec_replacement.

### setenv/unsetenv NOT in IRIX libc
Confirmed via `llvm-nm -D`. Added to st's `inject_compat_functions`. Existing compat in `compat/stdlib/setenv.c` (wraps `putenv()`).

### Bundle delivered
`st-0.9-4a-irix-bundle.tar.gz` (5.9 MB). Includes 16 RPMs: st, libXft, libXrender, fontconfig, freetype, expat, bzip2, gettext family, libpng. Tested on IRIX — `DISPLAY=bogus:99 st` correctly prints "can't open display" and exits 1.

### Font bundling for X11 apps (NEW pattern)
st failed with "can't open font monospace" — IRIX has no TrueType fonts for Xft/fontconfig.

**Fix**: `bundle.py` now includes TTF fonts from `fonts/` directory in the mogrix project:
- Copies `.ttf` files to `share/fonts/` in bundle
- Adds `<dir prefix="relative">../../share/fonts</dir>` to bundle's `fonts.conf`
- Creates `conf.d/50-monospace.conf` mapping "monospace" → "Iosevka Nerd Font"
- Sets `FONTCONFIG_FILE="$dir/etc/fonts/fonts.conf"` in wrapper scripts

Font file: `fonts/IosevkaNerdFont-Regular.ttf` (13 MB, downloaded from [Nerd Fonts](https://github.com/ryanoasis/nerd-fonts))

Bundle `st-0.9-4b-irix-bundle.tar.gz` (10.2 MB) — tested on IRIX, Iosevka renders via Xft.

### Known issue: RPM-installed st stderr silent
RPM-installed `st` produces no stderr output (die() / vfprintf silent failure). Bundle version works. Likely dlmalloc symbol export interfering with IRIX libc stdio. Cosmetic — binary is functional.

---

## Session (16): Bundle versioning + documentation

### Bundle alphabetic revision suffixes
Bundle names now include an auto-incrementing letter suffix: `weechat-4.2.1-2a-irix-bundle`. When rebuilding, the suffix advances (a → b → c). Old bundle directories and tarballs for the same version are cleaned up automatically. Implemented in `bundle.py` `create_bundle()`.

### Bundle architecture documentation
Created `rules/methods/bundles.md` — explains the three-layer script architecture (trampolines → wrappers → binaries), dependency resolution via ELF NEEDED scanning, bundle optimization, app-specific customization points, and user installation.

### Rebuilt weechat bundle
Rebuilt with all accumulated fixes. Bundle `weechat-4.2.1-2a-irix-bundle.tar.gz` (21.7 MB) includes: plugin autoload via WEECHAT_EXTRA_LIBDIR, CA cert auto-config via `-r`, SSL_CERT_FILE for OpenSSL apps, uninstall `rm -rf` hint, alphabetic revision suffix.

---

## Session (15): TLS fixes + bundle polish

### gnutls _Thread_local crash (CRITICAL FIX)
`__tls_get_addr` is an unresolvable symbol on IRIX (rld has no TLS support). gnutls `lib/gthreads.h` maps `_Thread_local` to `__thread`, creating TLS variables in `random.c` and `fips.c`.

**Fix**: `prep_commands` sed to redefine `_Thread_local` to empty in `gthreads.h` — thread-local becomes plain static. Safe: IRIX is single-threaded for our purposes.

**Gotcha**: `-D_Thread_local=` in CFLAGS does NOT work — `#define` in source headers overrides `-D` from command line. Must patch the header directly.

**Gotcha**: `extra_cflags` rule is validated but NOT implemented in the engine — dead code. Used `prep_commands` instead.

### weechat plugin loading (WEECHAT_EXTRA_LIBDIR)
Weechat has `/usr/sgug/lib32/weechat` hardcoded as plugin search path. Bundle plugins are at `_lib32/weechat/plugins/`. Without `WEECHAT_EXTRA_LIBDIR`, the IRC plugin doesn't load — only core commands available (no `/server`).

**Fix**: Bundle wrappers set `WEECHAT_EXTRA_LIBDIR="$dir/_lib32/weechat"`.

### CA certificate auto-configuration
gnutls ignores `SSL_CERT_FILE` (that's OpenSSL). Weechat uses `weechat.network.gnutls_ca_user` setting.

**Fix**:
1. `mogrix bundle` auto-includes build host CA certs (`_include_ca_bundle()`)
2. Weechat wrapper passes `-r "/set weechat.network.gnutls_ca_user $dir/etc/pki/tls/certs/ca-bundle.crt"`
3. `-r` is ephemeral (not saved), acts as default without overriding user config

### Bundle infrastructure improvements (`bundle.py`)
- `{extra_env_block}` in wrapper templates — per-bundle env vars (WEECHAT_EXTRA_LIBDIR, SSL_CERT_FILE)
- `{extra_args}` in wrapper templates — per-binary CLI args (weechat's `-r` for CA setup)
- `_include_ca_bundle()` — auto-detects and copies build host CA bundle
- Detection is data-driven: weechat plugins dir → WEECHAT_EXTRA_LIBDIR; ca-bundle.crt → SSL_CERT_FILE + gnutls_ca_user

### weechat VERIFIED on Libera.Chat IRC
- TLS connection to irc.libera.chat:6697 — **working**
- Certificate verification via bundled CA certs — **working**
- IRC plugin autoloading via WEECHAT_EXTRA_LIBDIR — **working**
- All 54 trampolines functional

## Session (14): gnutls + weechat + bundle

### gnutls 3.8.3 — bcond flipping pattern
Complex spec with inline `%if/%else/%endif` inside `%configure` line continuations. The `configure_flags: remove` engine can't handle these (leaves orphaned `\` on `%endif` lines → `%else with no %if` error).

**Solution: bcond flipping** — Replace `%bcond_without X` with `%bcond_with X` via spec_replacements. RPM evaluates `%if %{with X}` to FALSE, keeping only `%else` branches. Much cleaner than trying to strip individual flags.

### weechat 4.2.1 — cmake cross-compilation
Replaced entire `%cmake3` block with raw cmake invocation. Disabled ALL scripting languages, spell, docs, headless, NLS, cJSON. Key: `ICONV_INCLUDE_PATH`/`ICONV_FOUND` for FindIconv.cmake, select() duplicate symbol fix.

### libcurl OpenSSL soname fix
Rebuilt curl → NEEDED now correctly shows `libssl.so.3` (not `libssl.so.0.9.7`). Root cause: old build predated irix-ld staging `-L` ordering fix.

---

## Session (13): ncurses Fix + Bundle Restructure

### ncurses ext-colors terminfo corruption (CRITICAL FIX)
- **Symptom**: `nano` (interactive) → Bus error on IRIX. `nano --version` works.
- **Root cause**: ncurses 6.4 ABI 6 auto-enables `--enable-ext-colors` → `NCURSES_INT2=int` (32-bit). Terminfo reader interprets legacy 16-bit number fields as 32-bit: `cols=80` becomes `(80<<16)|8 = 5242888`. SIGBUS on MIPS from absurd buffer allocation.
- **Fix**: `--disable-ext-colors` spec_replacement in `rules/packages/ncurses.yaml`. Rebuilt + installed.
- **Verified**: `infocmp -C vt100` shows `co#80`, nano interactive works.

### Bundle restructure — Flatpak-inspired install model
- One `bin/` directory with trampoline scripts pointing into bundles
- Trampoline: 4-line shell script that resolves own location via `dirname "$0"` and uses relative path
- Install/uninstall scripts; tested on IRIX with nano + groff coexisting

### Bundle optimization — 92% size reduction
- Lib pruning, terminfo trim, doc strip, staging dedup
- nano bundle 48MB → 3.9MB (tarball 13MB → 0.9MB)

---

## Session (11): groff + C++ Milestone

### groff 1.23.0 — First C++ package!
Built with `irix-cxx` (clang++ + GCC 9 libstdc++). Key fixes: c++config.h, wchar_t keyword, doc generation override, bashisms, update-alternatives drop, subpackage drops.

Also completed: libxslt, giflib, libstrophe (sockaddr_storage fix).

### Sessions 7-10 Summary
- **Session 10**: C++ cross-compilation infrastructure
- **Session 9**: batch-build of 68 packages, 8 new packages built
- **Session 8**: nano, rsync, unzip, zip. Long double crash pattern
- **Session 7**: `mogrix batch-build` command implemented

### Sessions 4-6 Summary
- **Session 6**: tdnf Error(1602) root cause (IRIX pre-C99 vsnprintf)
- **Session 5**: OpenSSH 9.6p1 (R_MIPS_REL32 dispatch pattern)
- **Session 4**: aterm (first X11 app), `-rdynamic` filter, tdnf repo setup

---

## Package Status

**Total: 88 source packages (240+ RPMs)**

### Phases 1-4c: 41 packages (ALL INSTALLED)
Bootstrap (14) + system libs (2) + build tools (6) + crypto (7) + text tools (3) + utilities (9).

### Phase 5+: 42 packages

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
| nettle | 3.9.1 | INSTALLED | --enable-mini-gmp |
| libtasn1 | 4.19.0 | INSTALLED | GTKDOCIZE=true |
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
| **gnutls** | **3.8.3** | **INSTALLED** | **bcond flipping, PRIdMAX compat, _Thread_local fix, drop crypto-policies** |
| **weechat** | **4.2.1** | **BUNDLED+VERIFIED** | **Connected to Libera.Chat! cmake cross-build, plugin autoload, CA certs** |
| **libXrender** | **0.9.11** | **INSTALLED** | **X Render Extension. Handcrafted .pc files for IRIX native X11** |
| **libXft** | **2.3.8** | **INSTALLED** | **Xft font rendering. Depends on libXrender + fontconfig + freetype** |
| **st** | **0.9** | **BUNDLED+VERIFIED** | **Suckless terminal. X11R6.3 compat, openpty/pselect compat, Xft + Iosevka Nerd Font** |
| **glib2** | **2.80.0** | **INSTALLED** | **First meson package! SGUG-RSE patch rebased, gnulib frexp fix, strnlen compat** |
| **bitlbee** | **3.6** | **BUNDLED+VERIFIED** | **IRC gateway. %zu fix, sa_len macro, LLD ld -r, gcrypt linkage, strcasestr** |

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
| Bundles | `~/mogrix_outputs/bundles/` |

### Workflow
```bash
uv run mogrix fetch <package> -y
uv run mogrix convert ~/mogrix_inputs/SRPMS/<pkg>-*.src.rpm
uv run mogrix build ~/mogrix_outputs/SRPMS/<pkg>-*-converted/<pkg>-*.src.rpm --cross
uv run mogrix stage ~/mogrix_outputs/RPMS/<pkg>*.rpm
# Install on IRIX via MCP: irix_copy_to + irix_exec "rpm -Uvh"
# Bundle: uv run mogrix bundle <package>
```

---

## Known Issues

- **`%zu` format specifier** crashes IRIX libc — use `%u` for size_t
- **Volatile function pointer static initializers** crash on IRIX rld — use dispatch functions
- **fopencookie** crashes — use funopen instead
- **`--export-dynamic`** crashes IRIX rld with large symbol tables (>468 entries). Use `--dynamic-list` for selective export (e.g., plugin dlopen). irix-ld now gates `--export-dynamic` behind `MOGRIX_ALLOW_EXPORT_DYNAMIC` env var
- **R_MIPS_REL32 relocations** from function pointers in static data — use switch/strcmp dispatch
- **Long double (128-bit)** not supported on MIPS n32 — `__extenddftf2` crashes. Fix: `ac_cv_type_long_double_wider: "no"`
- **openssh fork-mode broken** — use `-d` (debug/no-fork) in a loop
- **Man pages not compressed** — brp scripts disabled; use `*.1*` not `*.1.gz` in %files
- **Explicit Provides required** — `AutoProv: no` in rpmmacros.irix; every .so needs explicit Provides
- **C++ c++config.h patches** — staging only, not in mogrix source. Must disable `_GLIBCXX_USE_C99_MATH_TR1` and `_GLIBCXX_USE_STD_SPEC_FUNCS`
- **autoreconf undoes prep_commands** — patches to configure in %prep get overwritten. Use spec_replacements after autoreconf
- **sockaddr_storage** hidden by `_XOPEN_SOURCE=500` in dicl-clang-compat — defined explicitly in overlay
- **ncurses ext-colors terminfo corruption** — ABI 6 auto-enables ext-colors, reads 16-bit terminfo numbers as 32-bit. Fix: `--disable-ext-colors`
- **configure_flags: remove vs inline conditionals** — Engine can't handle `%if/%else/%endif` inside `%configure` continuations. Use bcond flipping instead.
- **select() duplicate symbol** — dicl-clang-compat `extern select()` vs IRIX `static select()` when `_XOPEN_SOURCE` set. Fixed in `cross/include/dicl-clang-compat/sys/select.h`
- **IRIX X11R6.3 missing APIs** — `XICCallback`, `XSetIMValues`, `Xutf8TextListToTextProperty`, `XUTF8StringStyle` not available. Substitute with X11R6.3 equivalents (XIMCallback, no-op macro, Xmb variant, XCompoundTextStyle)
- **`_XOPEN_SOURCE=600` hides struct winsize** — On IRIX, winsize/TIOCSWINSZ gated by `_NO_XOPEN5`. Remove `_XOPEN_SOURCE`; `_SGI_SOURCE` provides everything.
- **setenv/unsetenv not in IRIX libc** — Must use compat (wraps `putenv()`). Add to `inject_compat_functions`.
- **dlmalloc symbol export** — Statically linked dlmalloc exports malloc/free in dynamic symbol table. IRIX rld resolves libc stdio's malloc calls to dlmalloc, can cause stdio failures. Bundle `LD_LIBRARYN32_PATH` works around this.
- **__tls_get_addr / _Thread_local** — IRIX rld has no TLS. Any `__thread` or `_Thread_local` variables cause rld Fatal Error. Fix: patch source headers to remove TLS keywords. `-D` from CFLAGS won't work (source `#define` overrides). See gnutls.yaml `prep_commands`.
- **gnutls-utils libtool wrappers** — `make install` installs libtool wrapper scripts instead of real binaries. Needs fix-libtool-irix.sh or chrpath.
- **IRIX `sa_len` macro in `<sys/socket.h>`** — `#define sa_len sa_union.sa_generic.sa_len2` breaks local variables named `sa_len`. Rename to `saddr_len` or similar.
- **GNU ld `-r` + LLD incompatibility** — GNU ld's partial-link output has STB_LOCAL symbols that LLD rejects. Use LLD for both `ld -r` and final link.
- **GLib quark table warnings** — `g_hash_table_lookup: assertion 'hash_table != NULL' failed` on IRIX startup. Non-fatal but indicates .init constructor ordering issue. May need investigation for future glib2-dependent apps.
- **fdopendir missing on IRIX** — compat provides AT_FDCWD but not fdopendir. Code guarded by `#ifdef AT_FDCWD` must add `&& !defined(__sgi)`.

---

## What Worked (Sessions 14-18)

- **`--dynamic-list` for plugin symbol export**: `--export-dynamic` exports ALL symbols (887) → rld SIGSEGV. `--dynamic-list` exports only the 48 symbols the plugin needs (365 total) → works. Create .list file: `readelf -sW plugin.so | grep UND` to find needed symbols.
- **`MOGRIX_ALLOW_EXPORT_DYNAMIC` env var in irix-ld**: Makes `--export-dynamic` opt-in instead of silently dropped. Safe default (filtered) with escape hatch when needed.
- **`__MOGRIX_BUNDLE_DIR__` placeholder in config files**: Bundle wrapper seds the placeholder to actual path at runtime. Avoids baking absolute paths into config.
- **bcond flipping**: Cleanly disables features in specs with inline `%if/%else/%endif` inside `%configure`.
- **PRIdMAX format macros**: Added to mogrix-compat/generic/inttypes.h.
- **cmake cross-compilation for weechat**: Full raw cmake invocation replacing `%cmake3` macro.
- **OpenSSL soname fix**: Rebuilt libcurl → correct `libssl.so.3` NEEDED.
- **_Thread_local → empty via prep_commands sed**: Only reliable way to disable TLS keywords (CFLAGS `-D` gets overridden by `#define`).
- **WEECHAT_EXTRA_LIBDIR**: Solved plugin dlopen path for bundle installs.
- **Bundle CA cert auto-inclusion**: `_include_ca_bundle()` + weechat `-r` auto-config. Users get working TLS out of the box.
- **Bundle wrapper `{extra_env_block}` + `{extra_args}`**: Generic infrastructure for per-app env vars and CLI args.
- **Meson raw commands**: Replace `%meson` macros with `meson setup _build --cross-file=... && meson compile -C _build`. Avoids needing meson RPM macros. Pattern in glib2.yaml.
- **SGUG-RSE patch selective application**: Don't blindly apply old patches. Many are wrong for our `_XOPEN5` mode (e.g., gsocket.c XPG5 hacks). Analyze each hunk individually.
- **LLD for `ld -r` partial linking**: GNU ld's `-r` output has symbols with binding 0 that LLD rejects. Using LLD for both `ld -r` and final link avoids incompatibility.
- **Inject into Makefile.settings post-configure**: For non-autotools builds with custom configure, append to Makefile.settings after configure runs (e.g., `echo "EFLAGS += ..." >> Makefile.settings`).

## What Failed / Gotchas (Sessions 14-18)

- **configure_flags: remove on inline conditionals**: Leaves orphaned `\` on `%endif` lines. Solution: bcond flipping.
- **Symlink hack for OpenSSL sonames**: Wrong approach — fixed root cause by rebuilding.
- **`-D_Thread_local=` in CFLAGS**: Source `#define _Thread_local __thread` OVERRIDES `-D` from command line. Must patch the header directly.
- **`extra_cflags` rule is dead code**: Validated in validator.py but never applied in engine.py. Use `export_vars` or `prep_commands` instead.
- **`mogrix stage` doesn't overwrite**: cpio extraction respects timestamps, may skip files. Delete old files first or use `-u` flag.
- **`__tls_get_addr` is NOT just lazy binding**: It crashes on any actual TLS use (e.g., connecting to IRC with TLS). Previous session incorrectly assumed it was harmless.
- **gnutls_ca_file renamed in weechat 4.x**: Now `gnutls_ca_system` and `gnutls_ca_user` (not `gnutls_ca_file`).
- **SSL_CERT_FILE is OpenSSL-only**: gnutls ignores it. Must use app-specific mechanisms (weechat `-r`).
- **SGUG-RSE gsocket.c XPG5 hacks**: Our dicl-clang-compat forces `_XOPEN5` mode where `struct msghdr` already has modern fields (msg_control/msg_controllen). SGUG-RSE's `struct xpg5_msghdr` hacks are WRONG for this mode. Reverted ALL gsocket.c changes.
- **Meson cpp entry for C-only packages**: Setting `cpp = 'irix-cc'` in meson cross-file causes meson to use irix-cc for C++ link tests (.cpp extension). irix-cc can't handle .cpp files. Fix: remove `cpp` entry for C-only packages.
- **gnulib IRIX frexp assumptions**: gnulib's meson.build explicitly marks IRIX frexp/frexpl as "broken beyond repair". This is wrong for IRIX 6.5. Must patch the meson.build to add IRIX as working.
- **fdopendir not available on IRIX**: Our compat provides `AT_FDCWD` (via openat) but NOT `fdopendir`. Code using `#ifdef AT_FDCWD` as proxy for fdopendir must also check `!defined(__sgi)`.
- **IRIX `sa_len` macro**: `<sys/socket.h>` defines `#define sa_len sa_union.sa_generic.sa_len2`. Any local variable named `sa_len` gets expanded by preprocessor. Rename to avoid collision.
- **GNU ld `-r` + LLD final link**: GNU ld's relocatable output contains STB_LOCAL symbols with binding 0 that LLD rejects as "invalid binding". Must use same linker for both `-r` and final link.
- **libgcrypt-config not in PATH**: Cross-compilation staging bin isn't in PATH. Configure scripts using `$(libgcrypt-config --libs)` silently get empty string. Must inject `-lgcrypt` manually.

---

## Reference Documents

| Document | Purpose |
|----------|---------|
| `rules/INDEX.md` | Rules reference, pattern catalog, invariants |
| `rules/methods/before-you-start.md` | Checklist before debugging |
| `rules/generic.yaml` | Universal rules for all packages |
| `compat/catalog.yaml` | Compat function registry |
| `plan.md` | Project plan and architecture |
| `packages_plan.md` | Distribution strategy, killer app targets, build tiers |
| `mogrix/bundle.py` | App bundle generator |
| `rules/methods/bundles.md` | Bundle architecture documentation |
