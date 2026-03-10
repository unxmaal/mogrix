# IRIX par invocation — correct flags

**Keywords:** irix,par,syscall,trace,flags,invocation,-o,-O,-s,-i,-SS
**Category:** irix

IRIX par is NOT like Linux strace. Correct flags:
- `-s` = collect syscall events
- `-i` = follow forks
- `-SS` = print full syscall trace (display mode)
- `-T` = show thread/process IDs
- `-o file` = write analysis output to file (lowercase o)
- `-O file` = write raw event data to file (uppercase O, for later replay)

Common invocation: `par -s -SS -i -o par_output.txt ./command args`
Two-step (collect then analyze): `par -s -i -O raw.par ./command` then `par -SS < raw.par`
NEVER use `-f` (doesn't exist), never guess Linux strace flags.
