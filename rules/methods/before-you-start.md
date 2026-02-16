# Before You Start

Check these resources before making significant changes or debugging build failures.

## 0. Will this package actually work on IRIX?

Before investing time porting a package, ask: **does this software require OS-specific support that IRIX doesn't have?**

Red flags that a package won't be useful:
- **Debuggers/profilers** that need OS-specific ptrace, /proc, or perf support (GDB 14.2 removed all IRIX native debugging — we burned multiple sessions before realizing this)
- **System daemons** that need systemd, D-Bus, cgroups, or Linux-specific IPC
- **Filesystem tools** that need Linux-specific syscalls (inotify, fanotify, io_uring)
- **Network tools** that need netfilter, eBPF, or /proc/net
- **Container/VM tools** that need namespaces, seccomp, or KVM

Quick checks:
1. Does the package have IRIX-specific code? `grep -r irix <source>` — if it was removed, that's a bad sign
2. Did SGUG-RSE build it? Check `/home/edodd/projects/github/sgug-rse/packages/<name>/`
3. Does it depend on Linux-only APIs? Check configure.ac for `linux`-only code paths
4. Is there an IRIX-native alternative? (dbx for debugging, IRIX syslog, etc.)

If the answer is "it'll compile but can't actually do its job," skip it.

## 1. Check rules first

Check the rules/INDEX.md first.
Start with generic / global rules for fixes, then package-specific rules.  

## 2. Check HANDOFF.md

The "What Failed" sections document approaches that don't work. Don't repeat them.

## 3. Check SGUG-RSE 

```bash
ls /home/edodd/projects/github/sgug-rse/packages/<package>/
cat /home/edodd/projects/github/sgug-rse/packages/<package>/*.sgifixes.patch
```

SGUG-RSE solved many IRIX problems already. Their patches are battle-tested.

HOWEVER:
- SGUG-RSE patches were for older FC31 packages and likely won't work as-is for newer packages.
- **Question SGUG-RSE assumptions.** Many SGUG-RSE decisions were made because they were bootstrapping ON IRIX, building packages in the absence of most reasonable dependencies. They skipped linking against system libraries, bundled things unnecessarily, and disabled features that couldn't be built yet. We are cross-compiling with a full sysroot — we have readline, ncurses, and other libraries available. Just because SGUG-RSE skipped a dependency doesn't mean we should. Prefer using system libraries over bundled copies when the dependency is available.

## 4. Check Git History

```bash
git log --all --oneline --grep="<keyword>"
git log --all --oneline -- '**/filename'
git show <commit>
```

You may have already solved this problem in a previous session.


## 5. Check rules/methods/

| Problem | Method file |
|---------|-------------|
| Linker issues | methods/linker-selection.md |
| Missing functions | methods/compat-functions.md |
| Build failures | methods/irix-quirks.md |
| Creating patches | methods/patch-creation.md |

## 6. Check lld-fixes/README.md

If anything involves linking or relocations, the answer is probably LLD 18 with patches.

## 7. Clean Build Environment

```bash
rm -rf ~/rpmbuild/BUILD/*
rm -rf ~/rpmbuild/BUILDROOT/*
```

Builds are tainted if done without cleaning first. Old object files, patched sources, or stale configs from previous attempts can silently pollute the new build.
