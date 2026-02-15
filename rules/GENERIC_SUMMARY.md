# Generic Rules Summary

**Before adding rules to a package YAML**, check what `rules/generic.yaml` already provides.
Generic rules are applied to EVERY package automatically. Do NOT duplicate them in package files.

**generic.yaml already handles:**

| What | Mechanism | Details |
|------|-----------|---------|
| Test suite | `skip_check: true` | Can't run MIPS binaries on build host |
| Linux-only build deps | `drop_buildrequires` | systemd, systemd-devel, libselinux-devel, kernel-headers, systemtap-sdt-devel, libcap-devel, libcap-ng-devel, libacl-devel, libattr-devel, audit-libs-devel, alsa-lib-devel, gnupg2, gcc-c++, libdicl, libdicl-devel |
| Linux-only runtime deps | `drop_requires` | libdicl, libdicl-devel |
| libdicl removal | `remove_lines` | CPPFLAGS/LIBS exports, %gpgverify, %ldconfig_scriptlets, systemd scriptlets |
| Common compat functions | `inject_compat_functions` | strdup, strndup, strnlen, getline, getopt_long, asprintf, vasprintf (dlmalloc NOT injected — linked by irix-ld for executables only) |
| malloc(0) cross-detect | `ac_cv_overrides` | ac_cv_func_malloc_0_nonnull, ac_cv_func_realloc_0_nonnull, gl_cv_func_select_* |
| SGUG prefix paths | `rpm_macros` | _prefix=/usr/sgug, _libdir=/usr/sgug/lib32, _bindir, _includedir, etc. |
| Path translation | `rewrite_paths` | /usr/lib64 → /usr/sgug/lib32, /usr/lib → /usr/sgug/lib32, /usr/include → /usr/sgug/include |
| Linux-only features | `configure_disable` | selinux, systemd, udev, inotify, epoll, fanotify, timerfd, libcap, audit |
| Header stubs | `header_overlays: generic` | execinfo.h, malloc.h, error.h, etc. |
| Install cleanup | `install_cleanup` | Fix /usr/bin paths, rm *.la, rm infodir/dir, rm locale |
| Skip %find_lang | `skip_find_lang: true` | Comments out %find_lang, strips `-f *.lang` from %files (locale files always removed) |
| Subpackage bloat | `drop_subpackages` | debuginfo, debugsource, langpack-* |

**Only add to a package YAML** when the package needs something beyond the above.
