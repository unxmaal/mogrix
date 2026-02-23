# WebKit Silent Blockers on IRIX

**Problem**: WebKit's NetworkProcess receives HTTP requests via IPC but never calls libsoup. No crash, no log, no error — the request silently vanishes. MiniBrowser shows a blank white page with the URL in the address bar.

**Root cause**: WebKit's multi-process security sandbox model. The UIProcess acts as a security broker, populating allowlists before the NP needs them. The NP enforces these with silent-fail-closed behavior. On IRIX (no sandbox), the allowlists are empty or incorrectly populated for IP-address URLs, so every security check silently blocks legitimate operations.

---

## Three Blocker Categories Found

### 1. MESSAGE_CHECK Macros (Cookie Domain Checks)

**Pattern**: `NETWORK_PROCESS_MESSAGE_CHECK(condition)` — if condition is false, logs via RELEASE_LOG_FAULT (invisible on IRIX), sends `TerminateWebProcess` to UIProcess, and silently returns.

**Where**: `NetworkConnectionToWebProcess.cpp` — 11 instances of `allowsFirstPartyForCookies()`, guarding: scheduleResourceLoad, cookiesForDOM, setCookiesFromDOM, cookieRequestHeaderFieldValue, getRawCookies, domCookiesForHost, deleteCookie, setCookiesFromHTTPResponse, registrableDomainsWithWebsiteData.

**Why it fails on IRIX**: The `m_allowedFirstPartiesForCookies` map is populated by UIProcess IPC messages (`AddAllowedFirstPartyForCookies`). On IRIX, the map either isn't populated for IP-address URLs, or the processIdentifier lookup returns `end()` (line 446) which triggers `ASSERT_NOT_REACHED()` and returns `false`.

**Fix**: Bypass ALL instances with perl regex in prep_commands. Two macro variants need separate regexes:
```yaml
# Single-arg variant
- "perl -i -pe 's{NETWORK_PROCESS_MESSAGE_CHECK\\(m_networkProcess->allowsFirstPartyForCookies\\(m_webProcessIdentifier, .*?\\)\\);}{/* IRIX: cookie check bypassed */}g' ..."
# Completion variant
- "perl -i -pe 's{NETWORK_PROCESS_MESSAGE_CHECK_COMPLETION\\(m_networkProcess->allowsFirstPartyForCookies\\(m_webProcessIdentifier, .*?\\), .*?\\);}{/* IRIX: cookie check bypassed */}g' ..."
```

**Search pattern for similar issues**:
```bash
grep -rn 'MESSAGE_CHECK' Source/WebKit/NetworkProcess/
grep -rn 'allowsFirstPartyForCookies' Source/WebKit/
```

### 2. Async Completion Gates (Service Worker Import)

**Pattern**: `if (!server.isImportCompleted()) { server.whenImportIsCompleted(callback); return; }` — defers the load until an async import finishes. If the import never completes, the load is deferred forever.

**Where**: `NetworkConnectionToWebProcess.cpp:571` inside `scheduleResourceLoad()`, guarded by `#if ENABLE(SERVICE_WORKER)`.

**Why it fails on IRIX**: `ENABLE(SERVICE_WORKER)` is ON for GTK builds. The `SWServer` constructor starts an async import of service worker registrations from SQLite. On IRIX, this import either never starts (missing filesystem paths) or never signals completion (async callback depends on GLib main loop integration that may not fire).

**Fix**: Set the condition to `false` so the gate is never entered.

**Search pattern for similar issues**:
```bash
grep -rn 'isImportCompleted\|whenImportIsCompleted' Source/WebKit/
grep -rn 'CompletionHandler.*&&' Source/WebKit/NetworkProcess/ | grep -v '.h:'
```

### 3. Conditional Compilation Load Routing (Service Worker)

**Pattern**: `#if ENABLE(FEATURE) path_with_extra_complexity(); #else simple_path(); #endif` — the enabled path has async dependencies, error handling, or infrastructure requirements that don't exist on IRIX.

**Where**: `NetworkConnectionToWebProcess.cpp:606-610` — `startWithServiceWorker()` vs `start()`.

**Why it fails on IRIX**: `startWithServiceWorker()` calls `createFetchTask()` (needs SW connection → returns nullptr), then `abortIfServiceWorkersOnly()` (may abort if serviceWorkersMode is `Only`), then content filtering. The simple `start()` goes directly to `startRequest(originalRequest())`.

**Fix**: Replace `startWithServiceWorker()` with `start()`.

**Search pattern for similar issues**:
```bash
grep -rn '#if ENABLE(SERVICE_WORKER)' Source/WebKit/NetworkProcess/
grep -rn '#if ENABLE(CONTENT_FILTERING)' Source/WebKit/NetworkProcess/
```

---

## Why These Fail Silently (Common Traits)

All three share these traits — use them to identify future blockers:

1. **No crash** — the code path is valid, just takes a wrong branch
2. **No stderr/log** — `RELEASE_LOG_FAULT` goes to Apple's unified logging, which doesn't exist on IRIX. No fallback to stderr.
3. **No error callback** — the caller never receives an error; the request simply vanishes
4. **No timeout** — nothing detects that the load stalled; the browser just shows a blank page
5. **Works on Linux/macOS** — the security infrastructure is properly initialized on those platforms

---

## What Is NOT a Blocker

These were investigated and found safe:

| Feature | Status | Why |
|---------|--------|-----|
| Content Filtering | Not compiled | `ENABLE(CONTENT_FILTERING)` is Apple/Cocoa only |
| Content Extensions | No-op | ON for GTK but no rules loaded by default |
| CORS Preflight | Not triggered | Simple GET navigations don't trigger OPTIONS |
| GLib Main Loop | Should work | Cross-compiled GLib provides async infrastructure |
| NetworkLoadChecker::check() | Safe for basic HTTP | Uses WeakPtr correctly, no CORS for same-origin |

---

## Debugging Methodology

1. **MOGRIX_DIAG tags**: Compile-time `fprintf(stderr, "MOGRIX_DIAG: tag\n")` at critical points. Output visible in crash handler logs or captured stderr.
2. **Step-mapping**: Start from known-working point (NP_scheduleResourceLoad), add DIAG at each subsequent step, find the gap where execution stops.
3. **grep for patterns**: When one blocker is found, search for the same pattern across all files in the same directory.
4. **Full audit doc**: `docs/webkit-silent-blocker-audit.md` has the complete analysis with prioritized fix lists.

---

## Files

| File | Purpose |
|------|---------|
| `rules/packages/webkitgtk.yaml` | All bypasses in prep_commands |
| `docs/webkit-silent-blocker-audit.md` | Full audit with root cause analysis |
| `docs/webkit-ipc-flow-map.md` | Step-by-step HTTP loading flow with status |
| `Source/WebKit/NetworkProcess/NetworkConnectionToWebProcess.cpp` | All 3 blockers in this file |
| `Source/WebKit/NetworkProcess/NetworkResourceLoader.cpp` | SW routing, startNetworkLoad |
| `Source/WebKit/NetworkProcess/soup/NetworkDataTaskSoup.cpp` | libsoup integration |
