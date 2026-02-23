# WebKit Silent Blocker Audit — IRIX

**Created**: 2026-02-23 (session 114)
**Scope**: Systematic analysis of WHY the three known blockers fail, and a sweep for identical patterns across the HTTP loading path.

---

## Part 1: Root Cause Analysis — The Three Known Blockers

### Blocker 1: Cookie Domain Check (BYPASSED)

**Location**: `NetworkConnectionToWebProcess.cpp:504` (original line; our line ~565 after DIAG inserts)

```cpp
NETWORK_PROCESS_MESSAGE_CHECK(m_networkProcess->allowsFirstPartyForCookies(
    m_webProcessIdentifier, loadParameters.request.firstPartyForCookies()));
```

**What the macro does** (lines 116-125):
```cpp
#define NETWORK_PROCESS_MESSAGE_CHECK(assertion) ...
    if (UNLIKELY(!(assertion))) {
        RELEASE_LOG_FAULT(IPC, "Invalid message dispatched");
        m_networkProcess->parentProcessConnection()->send(
            Messages::NetworkProcessProxy::TerminateWebProcess(m_webProcessIdentifier), 0);
        return;  // ← SILENT RETURN
    }
```

**WHY it fails on IRIX**:
1. `m_allowedFirstPartiesForCookies` is a `HashMap<ProcessIdentifier, pair<LoadedWebArchive, HashSet<RegistrableDomain>>>` in `NetworkProcess.h`
2. It's populated by `addAllowedFirstPartyForCookies()` which is called by the UIProcess via IPC message `AddAllowedFirstPartyForCookies`
3. On IRIX, the UIProcess sends this message but the `RegistrableDomain` for the HTTP URL (`192.168.0.1`) may not be in the set, OR the processIdentifier lookup returns `end()` (line 446), which triggers `ASSERT_NOT_REACHED()` and returns `false`
4. The ASSERT doesn't crash in release builds — it returns `false`, which makes the macro send `TerminateWebProcess` and silently return

**Root cause**: The security sandbox cookie-domain allowlist is designed for sandboxed browser environments where the UIProcess carefully controls which domains each WebProcess can access. On IRIX, there's no sandbox, and the allowlist may not be populated correctly for bare IP URLs.

**Fix applied**: Replace the check with a comment (session 112).

---

### Blocker 2: Service Worker Import Gate (BYPASSED)

**Location**: `NetworkConnectionToWebProcess.cpp:571` (after DIAG inserts)

```cpp
if (auto& server = session->ensureSWServer(); !server.isImportCompleted()) {
    server.whenImportIsCompleted([...] { scheduleResourceLoad(...); });
    return;  // ← DEFERRED FOREVER
}
```

**WHY it fails on IRIX**:
1. `ensureSWServer()` creates a `SWServer` on first call
2. `SWServer` constructor starts an async import of SW registrations from an SQLite database
3. `isImportCompleted()` returns `true` only after the database import finishes
4. On IRIX, the SQLite import either never starts (missing filesystem paths) or starts but never signals completion (async callback depends on GLib main loop integration that may not fire)
5. The `whenImportIsCompleted()` callback is stored in a vector and only called when `importCompleted()` is invoked — which never happens

**Root cause**: Service Workers are a web platform feature that requires background database infrastructure. IRIX doesn't need SW support at all, but `ENABLE(SERVICE_WORKER)` is `ON` for GTK builds and there's no cmake flag to disable it without breaking other things.

**Fix applied**: Changed condition to `false` so the gate is never entered (session 112).

---

### Blocker 3: Service Worker Load Routing (BYPASSED)

**Location**: `NetworkConnectionToWebProcess.cpp:607` (after DIAG inserts)

```cpp
#if ENABLE(SERVICE_WORKER)
    loader->startWithServiceWorker();
#else
    loader->start();
#endif
```

**WHY it fails on IRIX**:
1. `startWithServiceWorker()` (NetworkResourceLoader.cpp:1938) does:
   - `startContentFiltering(newRequest)` if `ENABLE(CONTENT_FILTERING)` — can silently return false
   - `m_connection->createFetchTask(*this, newRequest)` — tries to find a SW to handle the request
   - If no SW found, calls `abortIfServiceWorkersOnly()` — which may abort if `serviceWorkersMode == Only`
   - Only then falls through to `startRequest(newRequest)`
2. On IRIX, `createFetchTask()` calls `swConnection()` which returns `nullptr` (no SW connection), so `createFetchTask()` returns `nullptr`
3. But `abortIfServiceWorkersOnly()` checks `m_parameters.serviceWorkersMode` — if this is `Only` (set by the requesting page), it aborts the load entirely
4. Even if it falls through, there's a content filtering check that could silently block

**Root cause**: The SW routing path adds unnecessary async complexity and failure modes for a platform that will never have Service Workers.

**Fix applied**: Changed to `start()` directly (session 113).

---

### Common Pattern: Why These Fail Silently

All three share the same failure mode:
1. **No crash** — the condition check passes compilation, the runtime path just takes a wrong branch
2. **No log** — `RELEASE_LOG_FAULT` output goes to Apple's unified logging (which doesn't exist on IRIX), not stderr
3. **No error callback** — the caller never receives an error; the request simply vanishes
4. **No timeout** — nothing detects that the load stalled; the browser just shows a blank page

---

## Part 2: Systematic Scan — Remaining Blockers on the HTTP Path

### Category A: More `allowsFirstPartyForCookies` Checks

These are **identical** to Blocker 1. They will fail on IRIX for the same reason.

| # | File | Line | Function | On HTTP Path? | Severity |
|---|------|------|----------|---------------|----------|
| A1 | `NetworkConnectionToWebProcess.cpp` | 504 | `scheduleResourceLoad` | **YES** | **BYPASSED** |
| A2 | `NetworkConnectionToWebProcess.cpp` | 788 | `cookiesForDOM` | Cookie read | MEDIUM |
| A3 | `NetworkConnectionToWebProcess.cpp` | 805 | `setCookiesFromDOM` | Cookie write | MEDIUM |
| A4 | `NetworkConnectionToWebProcess.cpp` | 821 | `cookieRequestHeaderFieldValue` | Cookie header | MEDIUM |
| A5 | `NetworkConnectionToWebProcess.cpp` | 832 | `getRawCookies` | Cookie enumeration | LOW |
| A6 | `NetworkConnectionToWebProcess.cpp` | 861 | `domCookiesForHost` | Cookie by host | LOW |
| A7 | `NetworkConnectionToWebProcess.cpp` | 878 | `deleteCookie` | Cookie delete | LOW |
| A8 | `NetworkConnectionToWebProcess.cpp` | 898 | `setCookiesFromHTTPResponse` (with `isValidValue(host)`) | Cookie from HTTP | **HIGH** |
| A9 | `NetworkConnectionToWebProcess.cpp` | 1338 | `registrableDomainsWithWebsiteData` | Data listing | LOW |
| A10 | `WebSWServerConnection.cpp` | 335 | `scheduleJobInServer` | SW registration | LOW |
| A11 | `WebSharedWorkerServerConnection.cpp` | 88 | `requestSharedWorker` | SharedWorker | LOW |
| A12 | `NetworkSession.cpp` | 753 | (inline check, not MESSAGE_CHECK) | Session validation | MEDIUM |

**A8 is critical**: After HTTP succeeds and the server sends `Set-Cookie` headers, the NP will try to store them. If `setCookiesFromHTTPResponse` silently fails, the browser can't maintain session state (login, CSRF tokens). This will cause follow-up requests to fail.

**Recommended fix for all**: Same as Blocker 1 — bypass the `allowsFirstPartyForCookies` check entirely. A single sed/perl command can replace all instances in `NetworkConnectionToWebProcess.cpp`:

```yaml
# Bypass ALL allowsFirstPartyForCookies checks in NP connection handler.
# On IRIX there's no sandbox — the cookie domain allowlist isn't properly
# populated for direct IP URLs. These checks silently drop HTTP operations.
- "perl -i -pe 's{NETWORK_PROCESS_MESSAGE_CHECK\\(m_networkProcess->allowsFirstPartyForCookies\\(m_webProcessIdentifier, .*?\\)\\);}{/* IRIX: cookie check bypassed */}' Source/WebKit/NetworkProcess/NetworkConnectionToWebProcess.cpp"
- "perl -i -pe 's{NETWORK_PROCESS_MESSAGE_CHECK_COMPLETION\\(m_networkProcess->allowsFirstPartyForCookies\\(m_webProcessIdentifier, .*?\\), .*?\\);}{/* IRIX: cookie check bypassed */}' Source/WebKit/NetworkProcess/NetworkConnectionToWebProcess.cpp"
```

---

### Category B: Service Worker Gates (Beyond the Three Already Bypassed)

| # | File | Line | Pattern | On HTTP Path? | Severity |
|---|------|------|---------|---------------|----------|
| B1 | `NetworkResourceLoader.cpp` | 1370 | `createFetchTask` in `continueWillSendRequest` | Redirect handling | **HIGH** |
| B2 | `NetworkResourceLoader.cpp` | 1955 | `abortIfServiceWorkersOnly` in `startWithServiceWorker` | Load routing | **BYPASSED** (via B3 fix) |
| B3 | `NetworkResourceLoader.cpp` | 1977 | `abortIfServiceWorkersOnly` in `serviceWorkerDidNotHandle` | SW fallback | LOW (unreachable after bypass) |

**B1 is critical**: After the initial request succeeds and a redirect happens (`willSendRedirectedRequest` → `continueWillSendRequest`), it tries `createFetchTask` again for the redirected URL. If the SW connection is broken, it returns `nullptr`, which is fine — but this code path is only entered if `ENABLE(SERVICE_WORKER)`, and the `createFetchTask` call itself could trigger side effects.

**Recommended fix**: Already handled by the `start()` bypass. But B1 should be watched during redirect testing.

---

### Category C: Content Filtering (ENABLE_CONTENT_FILTERING)

`ENABLE(CONTENT_FILTERING)` is **Apple-only** (Cocoa platform). The GTK build does NOT enable it. Verified: no cmake option for it in `OptionsGTK.cmake`. These are NOT blockers.

| # | File | Lines | Impact |
|---|------|-------|--------|
| C1 | `NetworkResourceLoader.cpp` | 254-267 | `startContentFiltering` — NOT compiled |
| C2 | `NetworkResourceLoader.cpp` | 1943-1946 | In `startWithServiceWorker` — NOT compiled |

**No action needed.**

---

### Category D: Content Extensions (ENABLE_CONTENT_EXTENSIONS = ON for GTK)

Content extensions are **ON** for GTK. These run in `NetworkLoadChecker::check()`, which is on the critical path.

| # | Location | What It Does | Risk |
|---|----------|-------------|------|
| D1 | `NetworkLoadChecker.cpp:268-285` | `processContentRuleListsForLoad` async callback | LOW — uses WeakPtr, safe |
| D2 | `NetworkLoadChecker.cpp:496-505` | `contentExtensionsBackend` async callback | LOW — uses WeakPtr, safe |

**Actual risk**: Content extensions depend on the UIProcess sending rule lists. If no content extensions are configured (default), this code path is a no-op. Not a blocker.

---

### Category E: CORS Preflight (in NetworkLoadChecker)

| # | Location | What It Does | Risk |
|---|----------|-------------|------|
| E1 | `NetworkLoadChecker.cpp:417-461` | `checkCORSRequestWithPreflight` — async OPTIONS request | **HIGH if triggered** |
| E2 | `NetworkLoadChecker.cpp:445` | Callback uses raw `this`, no WeakPtr | Use-after-free risk |

**When is CORS triggered?** Only for cross-origin requests with specific conditions (non-simple methods, custom headers). A simple `GET http://192.168.0.1/` from MiniBrowser will NOT trigger CORS preflight.

**Risk**: LOW for basic HTTP testing. HIGH if WebKit pages make cross-origin XHR/fetch calls. The preflight has **no timeout** — if the OPTIONS request hangs, the load blocks forever.

**Recommended fix**: Not needed for initial HTTP testing. If CORS becomes relevant later, add a timeout to the preflight checker or disable CORS enforcement entirely.

---

### Category F: Async Completion Chains in startRequest()

The path `start()` → `startRequest()` → `NetworkLoadChecker::check()` callback:

```cpp
m_networkLoadChecker->check(ResourceRequest{newRequest}, this,
    [this, weakThis = WeakPtr{*this}](auto&& result) {
        if (!weakThis) return;
        WTF::switchOn(result,
            [this](ResourceError& error) { ... },
            [this](RedirectionTriplet& triplet) { ... },
            [this](ResourceRequest& request) {
                if (this->canUseCache(request)) {
                    this->retrieveCacheEntry(request);  // Cache path
                    return;
                }
                this->startNetworkLoad(WTFMove(request), FirstLoad::Yes);  // Network path
            }
        );
    });
```

**Risk assessment**:
1. The `weakThis` check is correct — no use-after-free
2. `canUseCache()` returns `false` on first load (no HTTP cache on IRIX) — falls through to `startNetworkLoad()`
3. The callback fires synchronously if no CORS preflight is needed (our case)

**No action needed for basic HTTP.**

---

### Category G: NetworkLoad → NetworkDataTaskSoup Path

Once `startNetworkLoad()` runs successfully:

| # | Step | File | Risk |
|---|------|------|------|
| G1 | `NetworkLoad` constructor | `NetworkLoad.cpp` | LOW — just stores parameters |
| G2 | `startWithScheduling()` | `NetworkLoad.cpp` | LOW — scheduler queues, then calls `resume()` |
| G3 | `NetworkDataTaskSoup` constructor | `NetworkDataTaskSoup.cpp` | **MEDIUM** — creates soup message |
| G4 | `createSoupMessage()` | `NetworkDataTaskSoup.cpp` | **MEDIUM** — if URL parsing fails |
| G5 | `resume()` | `NetworkDataTaskSoup.cpp:290` | **HIGH** — must have valid `m_soupMessage` AND `!m_cancellable` |
| G6 | `soup_session_send_async()` | `NetworkDataTaskSoup.cpp:314` | **CRITICAL** — the actual HTTP call |

**G5 potential silent failure**:
```cpp
void NetworkDataTaskSoup::resume() {
    if (m_state == State::Canceling || m_state == State::Completed)
        return;  // ← SILENT RETURN if state is wrong
    m_state = State::Running;
    if (m_soupMessage && !m_cancellable) {
        m_cancellable = adoptGRef(g_cancellable_new());
        soup_session_send_async(...);
        return;
    }
    // Falls through if m_soupMessage is null → NO HTTP REQUEST, NO ERROR
}
```

**If `m_soupMessage` is null** (createSoupMessage failed silently), `resume()` falls through without making any HTTP request and without reporting any error. This is a potential blocker.

**Our DIAG tags cover this**: `NP_resume_soupMsg` and `NP_resume_cancellable` will tell us the state at resume time.

---

### Category H: GLib Main Loop Integration

**This is the BIGGEST systemic risk** but is NOT a silent-blocker-pattern issue — it's an infrastructure question.

WebKit's NetworkProcess on GTK uses GLib's `GMainLoop` for all async operations:
- `soup_session_send_async()` dispatches to GMainLoop
- `sendRequestCallback` fires when GMainLoop processes the soup response
- `g_input_stream_read_async()` fires when data arrives

**On IRIX**: We're using the GLib that ships with our cross-compiled stack. If `g_main_loop_run()` is executing in the NP's main thread (which it must be, since that's how GTK auxiliary processes work), then GLib async operations should work.

**Verification**: If `NP_soup_send_async` fires but `sendRequestCallback` never fires, the GMainLoop is broken. But this is a different class of problem from the three silent blockers.

---

## Part 3: Prioritized Fix List

### Immediate (add to next rebuild)

| # | Fix | Complexity | Impact |
|---|-----|-----------|--------|
| 1 | Bypass ALL `allowsFirstPartyForCookies` checks in `NetworkConnectionToWebProcess.cpp` | LOW (2 perl commands) | Prevents cookie operations from silently failing |
| 2 | Already done: SW import gate bypass | — | DONE |
| 3 | Already done: SW load routing bypass | — | DONE |

### After HTTP Works (if needed)

| # | Fix | When | Impact |
|---|-----|------|--------|
| 4 | Bypass `allowsFirstPartyForCookies` in `WebSWServerConnection.cpp` | If SW registration fails | LOW |
| 5 | Bypass `allowsFirstPartyForCookies` in `WebSharedWorkerServerConnection.cpp` | If SharedWorkers fail | LOW |
| 6 | Add timeout to CORS preflight in `NetworkLoadChecker.cpp` | If cross-origin requests hang | MEDIUM |
| 7 | Watch `createFetchTask` in redirect path (`NetworkResourceLoader.cpp:1370`) | If HTTP redirects fail | HIGH |

### Probably Never Needed

| # | Fix | Why |
|---|-----|-----|
| 8 | Content Filtering | Not compiled for GTK |
| 9 | Content Extensions rule processing | No rules configured by default |
| 10 | NetworkBroadcastChannelRegistry checks | Only for BroadcastChannel API |
| 11 | SharedWorker connection checks | Only for SharedWorker API |

---

## Part 4: Search Patterns for Future Blocker Hunting

These grep patterns find the blocker categories above:

```bash
# Category A: Cookie domain checks that silently kill operations
grep -rn 'allowsFirstPartyForCookies' Source/WebKit/NetworkProcess/

# Category B: Service Worker gates
grep -rn 'isImportCompleted\|whenImportIsCompleted\|startWithServiceWorker\|createFetchTask\|abortIfServiceWorkersOnly' Source/WebKit/NetworkProcess/

# Category C/D: Conditional compilation gates
grep -rn '#if ENABLE(SERVICE_WORKER)\|#if ENABLE(CONTENT_FILTERING)\|#if ENABLE(CONTENT_EXTENSIONS)' Source/WebKit/NetworkProcess/

# All MESSAGE_CHECK variants (silent return on failure)
grep -rn 'MESSAGE_CHECK\|MESSAGE_CHECK_COMPLETION\|MESSAGE_CHECK_WITH_RETURN_VALUE' Source/WebKit/NetworkProcess/

# Async callbacks with no timeout (potential infinite wait)
grep -rn 'CompletionHandler.*&&\|whenImportIsCompleted\|_async(' Source/WebKit/NetworkProcess/

# Silent return without error propagation
grep -rn 'return;.*//.*silent\|return;.*//.*bypass\|return;.*//.*skip' Source/WebKit/NetworkProcess/
```

---

## Part 5: Architectural Insight

The three blockers share a common cause: **WebKit's security sandbox model assumes a controlled environment**.

On a modern Linux/macOS system:
- The UIProcess acts as a security broker
- It carefully populates allowlists (cookie domains, SW registrations) before the NP needs them
- The NP enforces these allowlists with silent-fail-closed behavior (the MESSAGE_CHECK pattern)
- This is correct security design — fail closed, don't crash, terminate the offending process

On IRIX:
- There's no sandbox
- The UIProcess may not correctly populate all allowlists (especially for IP-address URLs)
- The NP's fail-closed checks then silently block legitimate operations
- No crash, no log (on this platform), no error callback — just silence

**The fix principle**: On IRIX, bypass all NP-side security checks that enforce sandbox policies. These are defense-in-depth measures for sandboxed environments; on an unsandboxed single-user system they only cause harm.
