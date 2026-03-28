# Never guess IRIX bundle paths — list first

**Keywords:** irix,bundle,path,mogrix-apps,ls,directory,guess
**Category:** irix

Bundle directories on IRIX include version strings and timestamps (e.g., `tmux-3.5a-1-irix-bundle.0306260212`), not just the package name. ALWAYS run `ls ~/apps/` first to find the actual directory name before accessing files inside bundles. Never guess paths like `~/apps/tmux/bin/tmux`.
