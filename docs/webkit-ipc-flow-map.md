# WebKit HTTP Loading — Step Flow Map

**Updated**: 2026-02-22 (session 113)
**Purpose**: Map every step from known-working territory through the dark zone to the failure symptom.

## Context

WebKit MiniBrowser on IRIX: `file://` renders, HTTP never renders.

**Session 110 hypothesis** (WRONG): brokered WP↔NP IPC connection is dead.
**Session 111 finding**: brokered IPC works perfectly. NP receives the HTTP request but never invokes libsoup.
**Session 112 finding**: TWO silent blockers in `scheduleResourceLoad()` — cookie check + SW import gate.
**Session 113 finding**: THIRD blocker — `startWithServiceWorker()` silently blocks. Bypass applied, pending test.

## Process Map

| PID | Process | Inherited fd (→UI) | Brokered fd (WP↔NP) |
|-----|---------|-------------------|---------------------|
| 242097 | UIProcess (MiniBrowser) | fd=15 (→WP), fd=20 (→NP) | — |
| 240872 | WebProcess | fd=19 (→UI) | fd=25 (→NP) |
| 240639 | NetworkProcess | fd=14 (→UI) | fd=31 (→WP) |

---

## Phase A: Inherited Connections — CONFIRMED WORKING

| # | Step | Status | Evidence |
|---|------|--------|----------|
| 1 | UIProcess creates socketpairs, forks WP + NP | **CONFIRMED** | Processes start, fds inherited |
| 2 | NP `platformInitialize(fd=14)` → GSocket OK | **CONFIRMED** | `g_socket_new_from_fd_OK`, `g_socket_is_closed=0`, `condition_check=0` |
| 3 | NP `platformOpen(fd=14)` → GSocketMonitor starts | **CONFIRMED** | `platformOpen_fd=14`, `platformOpen_gsock_closed=0` |
| 4 | NP monitorCB fires repeatedly for fd=14 | **CONFIRMED** | Multiple `monitorCB_fd=14, cond=1` → `recvmsg_ok` → `monitorCB_CONTINUE` |
| 5 | WP `platformInitialize(fd=19)` → GSocket OK | **CONFIRMED** | Same DIAG pattern as NP fd=14 |
| 6 | WP monitorCB fires repeatedly for fd=19 | **CONFIRMED** | Multiple bidirectional send/recv cycles |
| 7 | UIProcess fd=15 and fd=20 bidirectional IPC | **CONFIRMED** | Sends and receives on both fds throughout |

## Phase B: Brokered Connection Setup — CONFIRMED WORKING

| # | Step | Status | Evidence |
|---|------|--------|----------|
| 8 | WP `ensureNetworkProcessConnection()` | **CONFIRMED** | `ensureNPConn_needed` → `ensureNPConn_creating` |
| 9 | UIProcess relays fd via SCM_RIGHTS to both sides | **CONFIRMED** | WP gets fd=25, NP gets fd=31 |
| 10 | NP `platformInitialize(fd=31)` → GSocket OK | **CONFIRMED** | `g_socket_new_from_fd_OK`, `condition_check=0` |
| 11 | NP `platformOpen(fd=31)` → GSocketMonitor starts | **CONFIRMED** | `platformOpen_fd=31`, `platformOpen_gsock_closed=0` |
| 12 | WP `platformInitialize(fd=25)` → GSocket OK | **CONFIRMED** | `g_socket_new_from_fd_OK`, `condition_check=1` (data waiting) |
| 13 | WP `platformOpen(fd=25)` → GSocketMonitor starts | **CONFIRMED** | `platformOpen_fd=25`, `platformOpen_gsock_closed=0` |

## Phase C: Brokered IPC Data Flow — CONFIRMED WORKING

| # | Step | Status | Evidence |
|---|------|--------|----------|
| 14 | NP→WP handshake: NP sends 29+29+28 bytes on fd=31 | **CONFIRMED** | `sendmsg_ok_fd=31, bytes=29/29/28` |
| 15 | WP receives handshake on fd=25 | **CONFIRMED** | `recvmsg_ok_fd=25, bytes=86` |
| 16 | WP→NP: sends 54 bytes on fd=25 | **CONFIRMED** | `sendmsg_ok_fd=25, bytes=54` |
| 17 | NP receives 54 bytes on fd=31 | **CONFIRMED** | `recvmsg_ok_fd=31, bytes=54` (monitorCB fires) |
| 18 | WP→NP: more messages (40, 29, 60, 37 bytes) | **CONFIRMED** | All received by NP on fd=31 |
| 19 | NP GSocketMonitor fires 5+ times for fd=31 | **CONFIRMED** | Multiple `monitorCB_fd=31, cond=1` → `monitorCB_CONTINUE` |

## Phase D: HTTP Request Dispatch — CONFIRMED WP SIDE

| # | Step | Status | Evidence |
|---|------|--------|----------|
| 20 | WP `scheduleLoad_entry` | **CONFIRMED** | DIAG fires |
| 21 | WP `scheduleLoad_tryURLScheme` (not intercepted) | **CONFIRMED** | DIAG fires, falls through |
| 22 | WP `scheduleLoad_to_NP` (decides to use NetworkProcess) | **CONFIRMED** | DIAG fires |
| 23 | WP `scheduleLoadFromNP_entry` + `ensureNPConn_needed` | **CONFIRMED** | DIAG fires |
| 24 | WP `scheduleLoadFromNP_ipc_OK` — IPC message sent | **CONFIRMED** | DIAG fires |
| 25 | WP `sendmsg_ok_fd=25, bytes=894` — the HTTP request | **CONFIRMED** | 894-byte message sent to NP |
| 26 | NP `recvmsg_ok_fd=31, bytes=894` — NP receives it | **CONFIRMED** | 894 bytes received via GSocketMonitor |

## Phase E: NP HTTP Handler Chain — THREE SILENT BLOCKERS FOUND

After NP receives the 894-byte message, it should:
1. Decode → dispatch to `NetworkConnectionToWebProcess::didReceiveMessage()`
2. Route to `scheduleResourceLoad()` → cookie check → SW import gate
3. Create `NetworkResourceLoader` → `startWithServiceWorker()` or `start()`
4. `startNetworkLoad()` → get `NetworkSession` → create `NetworkLoad` + `NetworkDataTaskSoup`
5. `resume()` → `soup_session_send_async()` → libsoup does the HTTP fetch

### Session 112-113 DIAG results: Three blockers identified and bypassed

| # | Step | Status | DIAG Tag / Evidence |
|---|------|--------|---------------------|
| 27 | NP `processIncomingMessage()` → decode 894-byte buffer | **CONFIRMED** | `readyRead_entry_fd=31, recvmsg_ok_bytes=894` |
| 28 | NP `NetworkConnectionToWebProcess::didReceiveMessage()` | **CONFIRMED** | `NP_connToWP_didRecvMsg` (fires 6 times) |
| 29 | NP message router: match ScheduleResourceLoad | **CONFIRMED** | (NP-3→NP-5 bracket) |
| 30 | NP `scheduleResourceLoad()` entry | **CONFIRMED** | `NP_scheduleResourceLoad` fires |
| **30.5** | **Blocker 1: Cookie check** | **BYPASSED** | `NETWORK_PROCESS_MESSAGE_CHECK(allowsFirstPartyForCookies())` silently returned. Fix: replaced with comment. |
| **30.6** | **Blocker 2: SW import gate** | **BYPASSED** | `!server.isImportCompleted()` was always true → load deferred forever. Fix: condition set to `false`. |
| 30.7 | NP creates `NetworkResourceLoader` | **CONFIRMED** | Execution continues past line 604 |
| **30.8** | **Blocker 3: SW load routing** | **BYPASSED (pending test)** | `startWithServiceWorker()` tries `createFetchTask()` + content filtering before `startRequest()`. On IRIX, this path silently blocks. Fix: changed to `start()`. Bundle `0222262335` has this fix. |
| 31 | NP `NetworkResourceLoader::start()` → `startRequest()` | **PENDING** | `NP_startNetworkLoad_hasSession` — should fire with bundle 0222262335 |
| 32 | NP `startNetworkLoad()` — get NetworkSession | **PENDING** | |
| 33 | NP create `NetworkLoad` + `NetworkDataTaskSoup` | **PENDING** | `NP_createNetworkLoad` |
| 34 | NP `createSoupMessage()` | **PENDING** | `NP_createSoupMsg_ENTER`, `NP_soupMsg_ptr` |
| 35 | NP `resume()` — state check | **PENDING** | `NP_resume_soupMsg`, `NP_resume_cancellable` |
| 36 | NP `soup_session_send_async()` | **PENDING** | `NP_soup_send_async` — **THE GATEWAY** |
| 37 | libsoup does HTTP fetch | **PENDING** | SOUP_DEBUG=2 should produce output |
| 38 | NP `sendRequestCallback()` — result | **PENDING** | `NP_sendReqCB_OK` / `NP_sendReqCB_FAIL` |

## Phase F: NP Process Init — CONFIRMED WORKING

| # | Step | Status | DIAG Tag |
|---|------|--------|----------|
| 39 | NP `initializeNetworkProcess()` → `platformInitializeNetworkProcess()` | **CONFIRMED** | `NP_platformInit_ENTER` + `NP_platformInit_OK` |
| 40 | NP `addWebsiteDataStore()` → creates `NetworkSessionSoup` | **CONFIRMED** | `NP_addWebsiteDataStores_ENTER` |

## Phase G: Teardown — CONFIRMED (pre-existing bugs)

| # | Step | Status | Evidence |
|---|------|--------|----------|
| 41 | UIProcess exits (killed by timeout or user close) | **CONFIRMED** | Process exits normally |
| 42 | WP `connectionDidClose` on inherited fd | **CONFIRMED** | `connectionDidClose`, `platformInvalidate` |
| 43 | WP SIGSEGV at 0xdc (NULL+220) during cleanup | **CONFIRMED** | Crash at same PC in both WP and NP (pre-existing) |
| 44 | NP `connectionDidClose` on inherited fd | **CONFIRMED** | Same pattern |
| 45 | NP brokered fd also closes | **CONFIRMED** | `connectionDidClose`, `platformInvalidate` |
| 46 | NP SIGSEGV at 0xdc (NULL+220) | **CONFIRMED** | Same crash as WP — cleanup bug, not root cause |

---

## Coverage Summary

```
Phase A (steps 1-7):    Inherited connections     — CONFIRMED ✓
Phase B (steps 8-13):   Brokered setup            — CONFIRMED ✓
Phase C (steps 14-19):  Brokered data flow        — CONFIRMED ✓
Phase D (steps 20-26):  WP HTTP request dispatch   — CONFIRMED ✓
Phase E (steps 27-30):  NP message decode/route    — CONFIRMED ✓
Phase E (steps 30.5-8): THREE SILENT BLOCKERS      — ALL BYPASSED ✓ (step 30.8 pending test)
Phase E (steps 31-38):  NP → libsoup chain         — ★ PENDING ★ (need test with bundle 0222262335)
Phase F (steps 39-40):  NP process init            — CONFIRMED ✓
Phase G (steps 41-46):  Teardown                   — CONFIRMED ✓ (crash is pre-existing)
```

## Visual Progress

```
MiniBrowser window on IRIX:
  ┌──────────────────────────────────────┐
  │ WebKitGTK MiniBrowser                │
  │ ← → ⟳  http://192.168.0.1/    🔍 ≡ │
  ├──────────────────────────────────────┤
  │                                      │
  │         (white / blank page)         │
  │                                      │
  │     Browser opens, URL in bar,       │
  │     but content never loads.         │
  │     No SOUP_DEBUG output.            │
  │                                      │
  └──────────────────────────────────────┘

  Status: Waiting for bundle 0222262335 test
  (has all 3 bypasses: cookie + SW import + SW routing)
```

## Three Silent Blockers — Summary

| # | Blocker | Location | How It Fails | Fix |
|---|---------|----------|-------------|-----|
| 1 | **Cookie domain check** | `NetworkConnectionToWebProcess.cpp:565` | `NETWORK_PROCESS_MESSAGE_CHECK` macro silently returns + sends TerminateWebProcess | Replace check with comment |
| 2 | **SW import gate** | `NetworkConnectionToWebProcess.cpp:571` | `!server.isImportCompleted()` always true → load deferred forever via callback that never fires | Set condition to `false` |
| 3 | **SW load routing** | `NetworkConnectionToWebProcess.cpp:607` | `startWithServiceWorker()` tries createFetchTask + content filtering before startRequest; silently blocks | Change to `start()` |

All three are in the same file. All three fail silently — no crash, no log, no error.

## Diagnostic Decision Table (for bundle 0222262335)

| If last DIAG that fires is... | Meaning | Next Step |
|-------------------------------|---------|-----------|
| `NP_scheduleResourceLoad` only | SW routing bypass didn't compile | Check build log |
| `NP_startNetworkLoad_hasSession=0` | **NetworkSession is NULL** — init failed | Debug NP session creation |
| `NP_startNetworkLoad_hasSession=1` + `NP_createNetworkLoad` | NetworkLoad created, check soup | Look for soup DIAG |
| `NP_soupMsg_ptr=0` | `createSoupMessage()` failed — URL issue | Debug soup message creation |
| `NP_soup_send_async` but no callback | libsoup hangs — DNS? socket? GMainLoop? | par_trace the NP |
| `NP_sendReqCB_FAIL` | libsoup returned error | Check error details |
| `NP_sendReqCB_OK` + SOUP_DEBUG output | **HTTP WORKS** — check WP rendering | Success! Debug rendering if blank |
