# Use add_patch with git diff for complex text replacements containing shell metacharacters

**Keywords:** add_patch, git diff, patch, safepatch, quoting, shell metacharacters, YAML quoting, prep_commands, Makefile, perl, pipe
**Category:** methods

# Use add_patch for complex text replacements

## Problem
When a text replacement contains shell metacharacters ($, |, ', ", \t, $$, etc.), putting it in prep_commands via safepatch or inline perl leads to YAML+shell+perl quoting nightmares that are fragile and hard to debug.

## Rule
If the replacement text contains pipes, single quotes, dollar signs, or regex metacharacters, **use a git diff patch file** instead of safepatch/inline commands.

## How
1. Extract original source, make the change, generate diff:
   ```bash
   cp src/file.orig src/file && <make change> && diff -u src/file.orig src/file
   ```
2. Save as `patches/packages/<pkg>/<descriptive-name>.patch` with `a/` `b/` prefixes
3. In the YAML, add at top level (not under `rules:`):
   ```yaml
   add_patch:
     - descriptive-name.patch
   ```
4. Patches are applied with `patch -p1` during %prep, before prep_commands run

## Example: dash builtins.def Makefile fix
The replacement contained `$<`, `|`, `perl -ne '...'`, and `$$` — 4 iterations of safepatch quoting all failed. A one-line patch file worked immediately.

## When safepatch IS fine
- Simple `--insert-top` additions (no metacharacters in the inserted text)
- `--old/--new` where both strings are plain alphanumeric text
- Any replacement where the text doesn't contain |, ', $, or regex chars
