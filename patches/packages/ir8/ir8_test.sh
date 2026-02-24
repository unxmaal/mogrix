#!/bin/sh
# ir8_test.sh — automated test harness for ir8 browser
# Usage: ir8_test.sh [bundle_dir]
# Outputs: ir8_test_results.txt and ir8_test_errors.txt in current directory
#
# Requires DISPLAY to be set for GUI tests (tests 2+).
# Set MOGRIX_DIAG=1 and MOGRIX_CRASH_DIR=/path for diagnostic logging.
#
# IRIX /bin/sh compatible: backtick substitution, expr arithmetic, no $().

BUNDLE_DIR="${1:-/usr/people/edodd/apps/ir8-1.0-1-irix-bundle.0223262343}"
IR8="$BUNDLE_DIR/ir8"
TIMEOUT=30
RESULTS="ir8_test_results.txt"
ERRLOG="ir8_test_errors.txt"
ERRDIR="ir8_test_errfiles"

# Verify binary exists
if [ ! -f "$IR8" ]; then
    echo "ERROR: ir8 not found: $IR8"
    exit 1
fi

# Clean previous results
rm -f "$RESULTS" "$ERRLOG"
rm -rf "$ERRDIR"
mkdir -p "$ERRDIR"

# Header
_date=`date`
echo "# ir8 test results — $_date" > "$RESULTS"
echo "# Bundle: $BUNDLE_DIR" >> "$RESULTS"
echo "# Format: TEST|name|status|exit_code|time|error_count" >> "$RESULTS"

echo "# ir8 error summary — $_date" > "$ERRLOG"
echo "# Bundle: $BUNDLE_DIR" >> "$ERRLOG"

# Timeout wrapper — IRIX /bin/sh compatible
# Runs command with a watchdog that kills it after N seconds.
run_with_timeout() {
    _timeout=$1; shift
    _errfile=$1; shift
    "$@" 2>"$_errfile" &
    _pid=$!
    ( sleep $_timeout; kill $_pid 2>/dev/null ) &
    _watchdog=$!
    wait $_pid 2>/dev/null
    _rc=$?
    kill $_watchdog 2>/dev/null 2>&1
    wait $_watchdog 2>/dev/null 2>&1
    return $_rc
}

# Classify exit status into a result string.
# Sets global _STATUS variable (IRIX sh can't capture function output easily).
classify_result() {
    _crc=$1
    if [ "$_crc" -eq 0 ]; then
        _STATUS="PASS"
    elif [ "$_crc" -eq 137 ] || [ "$_crc" -eq 143 ]; then
        _STATUS="TIMEOUT"
    elif [ "$_crc" -eq 1 ]; then
        _STATUS="CRASH"
    elif [ "$_crc" -ge 128 ]; then
        _STATUS="CRASH"
    else
        _STATUS="FAIL"
    fi
}

# Count lines in a file. Sets global _ERRCNT.
count_errors() {
    if [ -f "$1" ]; then
        _ERRCNT=`wc -l < "$1" | tr -d ' '`
    else
        _ERRCNT=0
    fi
}

# Run a single test
# Usage: run_test <name> <timeout> <command> [args...]
run_test() {
    _name=$1; shift
    _tmout=$1; shift
    _errfile="$ERRDIR/${_name}.stderr"

    echo "  Running: $_name ..."

    # IRIX date lacks %s — extract seconds from HH:MM:SS via expr
    _ts=`date +%H:%M:%S`
    _sh=`echo "$_ts" | cut -d: -f1`
    _sm=`echo "$_ts" | cut -d: -f2`
    _ss=`echo "$_ts" | cut -d: -f3`
    _start=`expr $_sh \* 3600 + $_sm \* 60 + $_ss`

    run_with_timeout $_tmout "$_errfile" "$@"
    _rc=$?

    _te=`date +%H:%M:%S`
    _eh=`echo "$_te" | cut -d: -f1`
    _em=`echo "$_te" | cut -d: -f2`
    _es=`echo "$_te" | cut -d: -f3`
    _end=`expr $_eh \* 3600 + $_em \* 60 + $_es`
    _elapsed=`expr $_end - $_start`
    # Handle midnight wraparound
    if [ "$_elapsed" -lt 0 ]; then
        _elapsed=`expr $_elapsed + 86400`
    fi

    classify_result $_rc
    count_errors "$_errfile"

    echo "TEST|${_name}|${_STATUS}|${_rc}|${_elapsed}s|${_ERRCNT} errors" >> "$RESULTS"
    echo "    $_STATUS (rc=$_rc, ${_elapsed}s, $_ERRCNT error lines)"

    # Append errors to summary log
    if [ "$_ERRCNT" -gt 0 ] 2>/dev/null; then
        echo "" >> "$ERRLOG"
        echo "=== $_name (rc=$_rc, $_STATUS) ===" >> "$ERRLOG"
        cat "$_errfile" >> "$ERRLOG"
    fi
}

# ---- Error categorization ----
# Safe count: use egrep (supports | alternation unlike IRIX grep),
# write count to temp file to avoid backtick + || echo double-output bug.
_countfile="/tmp/ir8_count.$$"
safe_count() {
    # Usage: safe_count 'pattern' [grep_flags]
    # Sets _CNT to the match count
    cat "$ERRDIR"/*.stderr 2>/dev/null | egrep -c $2 "$1" > "$_countfile" 2>/dev/null
    _CNT=`cat "$_countfile" 2>/dev/null`
    case "$_CNT" in
        *[!0-9]*) _CNT=0 ;;
        "") _CNT=0 ;;
    esac
}

categorize_errors() {
    echo ""
    echo "=== ERROR CATEGORIES ===" >> "$ERRLOG"
    echo ""
    echo "--- Error Category Summary ---"

    # BLOCKER: missing library symbols
    safe_count 'rld:.*[Uu]nresolved'
    if [ "$_CNT" -gt 0 ]; then
        echo "  BLOCKER: $_CNT rld unresolved symbol errors"
        echo "CATEGORY|BLOCKER|rld_unresolved|$_CNT" >> "$ERRLOG"
        egrep 'rld:.*[Uu]nresolved' "$ERRDIR"/*.stderr >> "$ERRLOG" 2>/dev/null
    fi

    # BLOCKER: crash signals
    safe_count 'SIGSEGV|SIGBUS|SIGABRT'
    if [ "$_CNT" -gt 0 ]; then
        echo "  BLOCKER: $_CNT crash signals"
        echo "CATEGORY|BLOCKER|crash_signal|$_CNT" >> "$ERRLOG"
        egrep 'SIGSEGV|SIGBUS|SIGABRT' "$ERRDIR"/*.stderr >> "$ERRLOG" 2>/dev/null
    fi

    # BUG: WebProcess crashes
    safe_count 'WebProcess.*(crash|terminat)|web process crash'
    if [ "$_CNT" -gt 0 ]; then
        echo "  BUG:     $_CNT WebProcess crash messages"
        echo "CATEGORY|BUG|webprocess_crash|$_CNT" >> "$ERRLOG"
        egrep 'WebProcess.*(crash|terminat)|web process crash' "$ERRDIR"/*.stderr >> "$ERRLOG" 2>/dev/null
    fi

    # BUG: TLS issues
    safe_count 'tls|certificate|ssl' '-i'
    if [ "$_CNT" -gt 0 ]; then
        echo "  BUG:     $_CNT TLS/certificate messages"
        echo "CATEGORY|BUG|tls_issue|$_CNT" >> "$ERRLOG"
    fi

    # EXPECTED: JS errors
    safe_count 'JavaScript.*error|TypeError|ReferenceError|SyntaxError'
    if [ "$_CNT" -gt 0 ]; then
        echo "  EXPECTED: $_CNT JavaScript errors"
        echo "CATEGORY|EXPECTED|js_error|$_CNT" >> "$ERRLOG"
    fi

    # INFO: GTK/GLib warnings and criticals
    safe_count 'Gtk-(WARNING|CRITICAL)|GLib-GObject-(WARNING|CRITICAL)|Pango-WARNING'
    if [ "$_CNT" -gt 0 ]; then
        echo "  INFO:    $_CNT GTK/GLib warnings/criticals"
        echo "CATEGORY|INFO|gtk_warning|$_CNT" >> "$ERRLOG"
    fi

    # INFO: IPC errors
    safe_count 'IPC.*(error|Broken pipe)|Error sending IPC'
    if [ "$_CNT" -gt 0 ]; then
        echo "  INFO:    $_CNT IPC errors"
        echo "CATEGORY|INFO|ipc_error|$_CNT" >> "$ERRLOG"
    fi

    # DEBUG: resource load failures
    safe_count '[Ff]ailed to load'
    if [ "$_CNT" -gt 0 ]; then
        echo "  DEBUG:   $_CNT resource load failures"
        echo "CATEGORY|DEBUG|load_failure|$_CNT" >> "$ERRLOG"
    fi

    rm -f "$_countfile"
    echo ""
}

# ================================================================
# TEST SUITE
# ================================================================

echo "ir8 Test Harness"
echo "================"
echo "Bundle: $BUNDLE_DIR"
echo "Timeout: ${TIMEOUT}s per test"
echo ""

# 1. Version check (no display needed)
echo "[1/10] Startup"
run_test "version" 5 "$IR8" --version

# 2-10 need DISPLAY
if [ -z "$DISPLAY" ]; then
    echo ""
    echo "WARNING: DISPLAY not set. Skipping GUI tests (2-10)."
    echo "Set DISPLAY=:0 to run full suite."
    for _skip_name in homepage about_ir8 about_blank data_uri http_example https_example https_hn https_google https_discourse; do
        echo "TEST|${_skip_name}|SKIP|0|0s|0 errors" >> "$RESULTS"
    done
else
    # 2. Homepage (custom URI)
    echo "[2/10] Homepage"
    run_test "homepage" $TIMEOUT "$IR8" --exit-after-load "ir8-about:home"

    # 3. About page (second custom URI)
    echo "[3/10] About page"
    run_test "about_ir8" $TIMEOUT "$IR8" --exit-after-load "ir8-about:ir8"

    # 4. Blank page (minimal engine startup)
    echo "[4/10] Blank page"
    run_test "about_blank" 15 "$IR8" --exit-after-load "about:blank"

    # 5. Data URI (inline HTML)
    echo "[5/10] Data URI"
    run_test "data_uri" 15 "$IR8" --exit-after-load "data:text/html,<h1>Test</h1>"

    # 6. HTTP
    echo "[6/10] HTTP"
    run_test "http_example" $TIMEOUT "$IR8" --exit-after-load "http://example.com"

    # 7. HTTPS simple
    echo "[7/10] HTTPS simple"
    run_test "https_example" $TIMEOUT "$IR8" --exit-after-load "https://example.com"

    # 8. HTTPS + light JS
    echo "[8/10] HTTPS + light JS (HN)"
    run_test "https_hn" $TIMEOUT "$IR8" --exit-after-load "https://news.ycombinator.com"

    # 9. HTTPS + heavy JS (Google) — longer timeout
    echo "[9/10] HTTPS + heavy JS (Google)"
    run_test "https_google" 45 "$IR8" --exit-after-load "https://www.google.com"

    # 10. HTTPS + heavy JS (Discourse) — longer timeout
    echo "[10/10] HTTPS + heavy JS (Discourse)"
    run_test "https_discourse" 45 "$IR8" --exit-after-load "https://forums.sgi.sh"
fi

# Categorize all collected errors
categorize_errors

# Summary
_pass=`grep '^TEST|' "$RESULTS" | grep '|PASS|' | wc -l | tr -d ' '`
_fail=`grep '^TEST|' "$RESULTS" | grep '|FAIL|' | wc -l | tr -d ' '`
_crash=`grep '^TEST|' "$RESULTS" | grep '|CRASH|' | wc -l | tr -d ' '`
_timeout=`grep '^TEST|' "$RESULTS" | grep '|TIMEOUT|' | wc -l | tr -d ' '`
_skip=`grep '^TEST|' "$RESULTS" | grep '|SKIP|' | wc -l | tr -d ' '`
_total=`grep '^TEST|' "$RESULTS" | wc -l | tr -d ' '`

echo "================"
echo "Results: $_pass pass, $_fail fail, $_crash crash, $_timeout timeout, $_skip skip (of $_total)"
echo ""
echo "Results written to: $RESULTS"
echo "Error details in:   $ERRLOG"
echo "Per-test stderr in: $ERRDIR/"

# Final summary line in results file
echo "" >> "$RESULTS"
echo "SUMMARY|pass=$_pass|fail=$_fail|crash=$_crash|timeout=$_timeout|skip=$_skip|total=$_total" >> "$RESULTS"
