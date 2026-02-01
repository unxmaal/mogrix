# Package Design Rules

> **For AI agents**: These rules govern how packages should be structured.
> RPMs must be self-contained. Never hack missing resources into external scripts.

---

## Resource Ownership Rule

**The lowest-level dependency that might encounter the need for a resource owns that resource.**

When multiple packages could potentially need a file or directory, the package lowest in the dependency chain (installed earliest) owns it.

### Example: /var/run

Both rpm and tdnf need `/var/run` for lockfiles. The dependency chain is:

```
zlib → bzip2 → popt → ... → rpm → libsolv → tdnf
```

Since rpm is lower than tdnf in the chain, **rpm owns /var/run**.

### Example: /usr/sgug/etc/yum.repos.d

Both tdnf and sgugrse-repos could provide this directory. tdnf needs it to function, but sgugrse-repos provides the actual .repo files.

Since tdnf is a dependency of anything using repos, **tdnf owns the directory**.
sgugrse-repos owns the .repo files within it.

---

## Self-Containment Rule

**RPMs must contain everything needed for their functionality.**

Never rely on:
- Bootstrap scripts to create directories
- Manual post-install fixups
- External configuration not in a package

If a package needs a directory, it creates it in %install and lists it in %files.
If a package needs a config file, it ships one with sane defaults.
If a package needs a symlink, it creates it in %post or ships it in %files.

---

## IRIX-Specific Defaults

For IRIX packages, certain defaults differ from Linux:

| Setting | Linux Default | IRIX Default | Reason |
|---------|---------------|--------------|--------|
| gpgcheck | 1 | 0 | No GPG infrastructure on IRIX |
| _db_backend | varies | sqlite | BerkeleyDB problematic on IRIX |

These should be set in the package's config files, not external scripts.

---

## Directory Ownership Examples

| Directory | Owner Package | Reason |
|-----------|---------------|--------|
| `/var/run` | rpm | Lowest package needing lockfiles |
| `/var/lib/rpm` | rpm | rpm's database location (symlink to /usr/sgug/lib32/sysimage/rpm) |
| `/usr/sgug/etc/rpm` | rpm | rpm config directory |
| `/usr/sgug/etc/rpm/macros.sqlite` | rpm | Sets sqlite as default db backend |
| `/usr/sgug/etc/yum.repos.d` | tdnf | tdnf needs it to function |
| `/usr/sgug/etc/yum.repos.d/mogrix.repo` | tdnf | Default bootstrap repo (points to /tmp/mogrix-repo) |
| `/usr/sgug/var/cache/tdnf` | tdnf | tdnf's cache |
| `/usr/sgug/etc/pki/rpm-gpg` | sgugrse-repos | GPG key storage |
