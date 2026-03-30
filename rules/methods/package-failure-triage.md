# Package Failure Triage Protocol

> **When to use:** Every time a package fails during `mogrix rebuild-all` or
> `mogrix convert && mogrix build`. Follow this EXACTLY. Do not skip steps.

## Step 1: Read the error

Delegate to a Haiku sub-agent if the log is >200 lines. Get back a concise
summary: the failing command, the error message, the file/line if applicable.

## Step 2: MCP search

Call `report_error` with the error message. This searches rules, knowledge,
errors, and the compat catalog in one call. If it returns a documented fix,
apply it to `rules/packages/<pkg>.yaml` and rebuild. Done.

## Step 3: Classify the failure

### A) Package-specific build issue
The error is in the package's own source code, configure script, or Makefile.
Examples: missing function, wrong argument count, incompatible type in package code.

**Action:** Fix in `rules/packages/<pkg>.yaml` (spec_replacements, configure_flags,
patches) or `patches/packages/<pkg>/`. Rebuild. If it passes, `add_rule` immediately.

### B) Missing dependency
The package needs a library or header that isn't in staging yet.

**Action:** Check if the dependency is in the build order. If yes, it should have
built already — check its gate-result. If no, add it to the build order and build
it first. Fix in `rules/packages/<pkg>.yaml` (add_requires, build_after).

### C) Possible platform gap
The error references compat headers, compiler wrappers, linker behavior, or
IRIX system headers. Examples: "undefined reference" to a POSIX function not in
the compat library, conflicts between compat headers and IRIX system headers,
linker errors from rld/LLD incompatibilities.

**Action:**
1. **DO NOT modify any file under `cross/`, `compat/include/`, or `rules/generic.yaml`.**
2. Add `skip: true` to the package's YAML.
3. Add a comment: `# SKIP: <one-line description of the platform gap>`
4. Call `add_knowledge` with category "platform_gap" and full details.
5. Continue the rebuild. The platform gap will be triaged after the rebuild.

### D) Two failed fix attempts
You tried twice and the package still fails.

**Action:**
1. Add `skip: true` to the package's YAML.
2. Add a comment with what you tried and why it failed.
3. Call `add_knowledge` with your findings.
4. Continue the rebuild. Do not spend more time on this package.

### E) Unclear
You can't classify the failure.

**Action:** Same as D. Skip, log, continue.

## What is NEVER correct during a package porting session

- Modifying `cross/bin/irix-cc` or `cross/bin/irix-cxx*` to fix one package
- Modifying `cross/include/dicl-clang-compat/*.h` to fix one package
- Modifying `rules/generic.yaml` to fix one package
- Modifying `mogrix/rebuild.py` to fix one package
- Modifying `rpmmacros.irix` to fix one package
- Reverting a migration (libc++, LLVM version, prefix) to fix one package

These changes affect ALL 254+ packages. If one package needs them, the correct
action is to skip that package and flag the issue for human review.

Infrastructure changes happen in dedicated sessions on dedicated branches,
not as side effects of package porting.
