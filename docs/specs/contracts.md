# Interface Contracts — respec

## 1. `scan` — `src/respec/scan.py:145`

**Purpose.** Walk a repository directory, classify its files, and return a structured inventory used by all downstream pipeline stages.

**Signature.**
```python
def scan(repo_path: Path | str) -> dict[str, Any]
```

**Preconditions.**
- `repo_path` must resolve to an existing directory.
- The caller must have read permission for the tree (unreadable files are silently skipped via `OSError` guards on individual reads, but the root must be listable).

**Postconditions.**
The returned dict always contains these keys:

| Key | Type | Notes |
|-----|------|-------|
| `root` | `str` | Absolute resolved path |
| `languages` | `list[str]` | Sorted; drawn from `LANG_BY_EXT` extension map |
| `file_count` | `int` | Count of readable, non-binary, non-skipped files |
| `total_lines` | `int` | Sum of newline counts across included files |
| `structure` | `dict[str, {"files": int, "lines": int}]` | Keyed by top-level directory (or `"(root)"`) |
| `existing_docs` | `list[str]` | Sorted relative paths with `.md/.rst/.txt/.adoc` extensions |
| `ci_config` | `str \| None` | First matching CI candidate relative path |
| `test_framework` | `str \| None` | Detected framework name or command |
| `build_system` | `str \| None` | Detected build system name |
| `entry_points` | `list[str]` | Sorted relative paths matching `ENTRY_POINT_NAMES` |
| `interesting_files` | `dict[str, list[str]]` | Keys: `todos`, `hacks`, `fixmes`, `workarounds`, `criticals`, `safeties`, `invariants` |
| `skipped` | `list[str]` | Files omitted due to size > 50 KB or binary heuristic |

Files under directories in `ALWAYS_IGNORE` (e.g. `.git`, `.venv`, `node_modules`) and files matched by root `.gitignore` are excluded before any counting.

**Error behavior.** Raises `ValueError("not a directory: <path>")` if `repo_path` does not point to an existing directory. No exception is raised for individual unreadable or oversized files; they are recorded in `skipped`.

**Thread safety.** Reads files; writes no shared state. Safe to call concurrently on distinct paths.

---

## 2. `load_rules` — `src/respec/rules.py:102`

**Purpose.** Load and merge the four-layer ruleset (builtin → user generic → language → project) into a single `EffectiveRuleset`.

**Signature.**
```python
def load_rules(
    rules_dir: Path | str | None,
    *,
    project: str | None,
    language: str | None,
) -> EffectiveRuleset
```

**Preconditions.**
- `rules_dir`, if provided, must be a readable directory. A non-existent path is silently treated as absent (layers 2–4 are skipped).
- All YAML files referenced by `methods` entries and `prompt_overrides` values must exist relative to their layer's directory.
- Every YAML file's top-level value must be a mapping (not a list or scalar).

**Postconditions.**
Returns an `EffectiveRuleset` whose fields reflect the merged result of all present layers. The `sources` list records which files were actually loaded, in layer order. If `rules_dir` is `None` and no builtin `generic.yaml` exists, every field is empty/default.

Merge semantics per field kind:
- `dict_merge` keys: later layers overwrite individual keys.
- `list_append` keys: items from later layers are appended.
- `dict_merge_files` (`prompt_overrides`): values are file contents read at load time; later layers overwrite individual keys.
- `methods`: collected as relative paths, then resolved once and concatenated in order.

**Error behavior.**
- `ValueError` if any loaded YAML file's top-level value is not a mapping (`src/respec/rules.py:61`).
- `FileNotFoundError` for any `methods` file path that does not exist (`src/respec/rules.py:96`).
- `FileNotFoundError` for any `prompt_overrides` target path that does not exist (`src/respec/rules.py:88`).

**Thread safety.** Reads files; writes no shared state. Safe to call concurrently.

---

## 3. `EffectiveRuleset` — `src/respec/rules.py:41`

**Purpose.** Immutable-in-practice dataclass that carries merged rule configuration across every stage of the pipeline (`scan` → `analyze_*` → `assemble_context_*` → `load_prompt` → `generate_doc` → `redact`).

**Fields consumed by downstream modules.**

| Field | Type | Consumed by |
|-------|------|------------|
| `glossary` | `dict[str, str]` | `build_system_message` — injected into system prompt |
| `must_cite` | `list[str]` | `build_system_message` — citation instructions |
| `redaction_extra` | `list[str]` | `redact` — additional regex patterns |
| `include_globs` / `exclude_globs` | `list[str]` | Available for future use; not consumed in current code |
| `prompt_prefix` / `prompt_suffix` | `dict[str, str]` | `load_prompt` — per-doc-type wrapping |
| `prompt_overrides` | `dict[str, str]` | `load_prompt` — replaces entire bundled prompt body |
| `additional_doc_types` | `list[dict[str, str]]` | `analyze_*` — extends the set of doc types |
| `disable_doc_types` | `list[str]` | `analyze_*` — removes doc types from generation |
| `methods_text` | `str` | `build_system_message` — methodology block |
| `sources` | `list[str]` | `main._write_manifest` — provenance tracking |

**Contract note.** This dataclass is produced only by `load_rules`. Callers must not construct it manually; the merge semantics (especially `methods_text` resolution) are internal to `load_rules`.

---

## 4. `analyze_code` — `src/respec/analyze.py:89`

**Purpose.** Determine which specification documents are missing or partially covered in an existing codebase.

**Signature.**
```python
def analyze_code(inventory: dict, rules: EffectiveRuleset) -> list[Gap]
```

**Preconditions.**
- `inventory` must be the dict returned by `scan()` — specifically it must contain `root` (a resolvable path string) and `existing_docs` (a list of relative path strings).
- `rules` must come from `load_rules()`.

**Postconditions.**
Returns a `list[Gap]` sorted by `(PRIORITY_ORDER[status], doc_type)` — missing docs sort before partial, partial before existing. The set of doc types is determined by `EffectiveRuleset.additional_doc_types` and `disable_doc_types`. A doc is classified `"partial"` if any existing doc file contains a heading alias for that doc type with ≥ 500 characters of body text; otherwise `"missing"`.

**Error behavior.** No explicit exceptions. `OSError` from unreadable existing docs is swallowed per `try/except OSError: continue` in `_classify_code` (`analyze.py:63`).

**Thread safety.** Reads files; writes no shared state.

---

## 5. `analyze_plan` — `src/respec/analyze.py:100`

**Purpose.** Produce the generation work-list when operating from a design proposal rather than existing code.

**Signature.**
```python
def analyze_plan(manifest: dict, rules: EffectiveRuleset) -> list[Gap]
```

**Preconditions.**
- `manifest` must be the dict returned by `ingest_plan()`.
- `rules` must come from `load_rules()`.

**Postconditions.** Returns a `list[Gap]` where every entry has `status="missing"`. The LLM is responsible for marking sections "Underspecified" where the plan is silent. The set of doc types honours `additional_doc_types` and `disable_doc_types` in `rules`.

**Error behavior.** None; no I/O.

---

## 6. `Gap` — `src/respec/analyze.py:36`

**Purpose.** Frozen dataclass representing one documentation gap; the unit of work consumed by `_run` in `main.py`.

**Fields.**

| Field | Type | Values |
|-------|------|--------|
| `doc_type` | `str` | `"architecture"`, `"contracts"`, `"constraints"`, `"testing"`, `"decisions"`, `"onboarding"`, or any name from `additional_doc_types` |
| `status` | `str` | `"missing"` \| `"partial"` \| `"exists"` |
| `description` | `str` | Human-readable sentence describing the document |

**Contract note.** `doc_type` is used as a filename stem (`{doc_type}.md`) and as the key for prompt lookup, context dispatch, and manifest entries. Callers adding custom doc types via `additional_doc_types` must ensure no bundled prompt exists with that name unless they also supply a `prompt_overrides` entry.

---

## 7. `ingest_plan` — `src/respec/ingest_plan.py:182`

**Purpose.** Parse a Markdown plan document (or directory of `.md` files) into a typed intent manifest consumed by `analyze_plan` and `assemble_context_plan`.

**Signature.**
```python
def ingest_plan(plan_path: Path | str) -> dict[str, Any]
```

**Preconditions.**
- `plan_path` must exist as a file or directory.
- Files must be UTF-8 readable Markdown; binary files are not filtered here.

**Postconditions.**
Returns a dict with the following keys, populated by merging all discovered `.md` files in lexicographic order:

| Key | Type | Source |
|-----|------|--------|
| `source_files` | `list[str]` | Relative paths of parsed files |
| `title` | `str` | First H1 heading found |
| `stated_goal` | `str` | First paragraph of `goal/goals/purpose/overview/summary` section |
| `proposed_components` | `list[{"name", "section", "description"}]` | Bullets from `components/architecture/design/system/modules` sections |
| `stated_constraints` | `list[str]` | Bullets from `constraints/requirements/must/invariants` sections, plus must/shall sentences promoted from prose |
| `stated_non_goals` | `list[str]` | Bullets from `non-goals/out of scope` sections |
| `open_questions` | `list[str]` | Bullets ending with `?` from `open questions/tbd/unknowns/questions` |
| `external_dependencies` | `list[str]` | Bullets from `dependencies/external dependencies` sections |
| `decisions_made` | `list[str]` | Bullets from `decisions/decided` sections |
| `decisions_deferred` | `list[str]` | Bullets containing `TBD` from open-questions sections |
| `raw_text` | `str` | Concatenated full text of all files |

**Error behavior.**
- `FileNotFoundError` if `plan_path` does not exist.
- `ValueError("no markdown files found in <path>")` if a directory contains no `.md` files.

**Thread safety.** Reads files; writes no shared state.

---

## 8. `load_prompt` — `src/respec/extract.py:48`

**Purpose.** Return the assembled user-prompt template (prefix + body + suffix) for a given doc type and mode.

**Signature.**
```python
def load_prompt(
    doc_type: str,
    *,
    mode: str,
    rules: EffectiveRuleset,
    abstract: bool,
) -> str
```

**Preconditions.**
- `mode` must be `"from_code"` or `"from_plan"`.
- Either `rules.prompt_overrides[doc_type]` must exist, or a bundled prompt at `src/respec/prompts/{mode}/{doc_type}.md` must exist.

**Postconditions.** Returns a single string. The body is the override content if present, else the bundled file content. `rules.prompt_prefix[doc_type]` and `rules.prompt_suffix[doc_type]`, if non-empty, are prepended/appended with double newlines. The string contains the literal `{context}` placeholder which `generate_doc` replaces at call time.

**Error behavior.**
- `ValueError("unknown mode: <mode>")` for any `mode` not in `VALID_MODES`.
- `FileNotFoundError` if no override exists and the bundled prompt file is absent.

---

## 9. `assemble_context_code` — `src/respec/extract.py:170`

**Purpose.** Build the context string (inventory summary + conditionally included source samples) that is substituted into the `{context}` placeholder of the user prompt.

**Signature.**
```python
def assemble_context_code(
    inventory: dict,
    doc_type: str,
    rules: EffectiveRuleset,
) -> str
```

**Preconditions.**
- `inventory` must be the dict returned by `scan()`.
- `doc_type` must match a known or rule-supplied doc type (`"architecture"`, `"contracts"`, `"constraints"`, `"testing"`, `"decisions"`, or custom).
- `inventory["root"]` must be a valid directory path.

**Postconditions.** Returns a string of at most `CONTEXT_CHAR_BUDGET` characters (120,000 chars / ≈30 K tokens). The string always includes the inventory summary, top-level structure, and the first README-like file plus build config found. Additional sections are included conditionally by `doc_type`:
- `"constraints"` / `"decisions"`: adds interesting markers (TODO/HACK/FIXME/SAFETY/INVARIANT).
- `"architecture"` / `"contracts"`: adds up to 12 largest non-test source files (4 000 chars each).
- `"testing"`: adds a list of test files and the CI config.

**Error behavior.** None (individual file reads are wrapped in `try/except OSError`).

---

## 10. `assemble_context_plan` — `src/respec/extract.py:227`

**Purpose.** Build the context string from a plan manifest for use in `from_plan` mode.

**Signature.**
```python
def assemble_context_plan(
    manifest: dict,
    doc_type: str,
    rules: EffectiveRuleset,
) -> str
```

**Preconditions.** `manifest` must be the dict returned by `ingest_plan()`.

**Postconditions.** Returns a string of at most `CONTEXT_CHAR_BUDGET` characters containing the manifest as formatted JSON (excluding `raw_text`) followed by the raw plan text. `doc_type` and `rules` are accepted for signature uniformity but are not used in the current implementation.

---

## 11. `generate_doc` — `src/respec/extract.py:293`

**Purpose.** Invoke the configured LLM backend and return the generated specification document text.

**Signature.**
```python
def generate_doc(
    doc_type: str,
    mode: str,
    context: str,
    prompt_template: str,
    model: str,
    rules: EffectiveRuleset,
    *,
    abstract: bool,
    backend: str = "litellm",
) -> str
```

**Preconditions.**
- `mode` must be in `VALID_MODES` (`{"from_code", "from_plan"}`).
- `backend` must be in `VALID_BACKENDS` (`{"litellm", "claude-cli"}`).
- `prompt_template` must contain the literal substring `{context}`; the call performs a plain `str.replace`.
- For `backend="litellm"`: `model` must be a model string accepted by litellm (e.g. `"anthropic/claude-sonnet-4-20250514"`).
- For `backend="claude-cli"`: the `claude` binary must be on `PATH`; `model` must be a short alias accepted by `claude -p --model` (e.g. `"sonnet"`).

**Postconditions.** Returns the raw text produced by the LLM. For `claude-cli`, any leading non-heading preamble is stripped (`_strip_preamble`). No further transformation is applied; the caller (`_run` in `main.py`) is responsible for redaction if `abstract=True`.

**Error behavior.**
- `ValueError("unknown mode: …")` — mode not in `VALID_MODES`.
- `ValueError("unknown backend: …")` — backend not in `VALID_BACKENDS`.
- `RuntimeError("claude CLI not found in PATH …")` — `claude-cli` backend, binary absent.
- `RuntimeError("claude CLI exited <N>: …")` — `claude-cli` backend, non-zero exit.
- litellm exceptions propagate unmodified for the `litellm` backend.

**Thread safety.** Makes network or subprocess calls. Concurrent calls are safe in isolation but may trigger API rate limits.

---

## 12. `redact` — `src/respec/redact.py:24`

**Purpose.** Strip precise technical identifiers (file paths, versions, URLs) from a generated document for abstract-mode output.

**Signature.**
```python
def redact(text: str, rules: EffectiveRuleset) -> tuple[str, list[str]]
```

**Preconditions.**
- `rules` must come from `load_rules()` (uses `rules.redaction_extra`, which may be empty).
- `text` is expected to be the output of `generate_doc`.

**Postconditions.** Returns `(redacted_text, audit_log)`.
- `redacted_text`: all fenced code blocks replaced with `‹example elided›` (except `mermaid` blocks, which are preserved); URLs replaced with `‹url›`; `file.ext:lineno` patterns with `‹code location›`; bare source file paths with `‹source file›`; semver strings with `‹version›`; any patterns in `rules.redaction_extra` with `‹redacted›`.
- `audit_log`: one entry per substitution in the form `"<category>: <matched_text>"`. Code block entries use `"code_block: elided (<N> chars, lang=<lang>)"`.

Patterns run in the order defined in `DEFAULT_PATTERNS` (url → code_location → source_file → version), followed by `redaction_extra` patterns. More-specific patterns running first prevents over-matching.

**Error behavior.** No exceptions; all operations are in-memory regex substitutions.

**Thread safety.** Stateless; safe to call concurrently.

