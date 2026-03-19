#!/bin/bash
# deploy-test.sh — Deploy a mogrix bundle to IRIX, smoke test, collect diagnostics
#
# Usage:
#   ./tools/deploy-test.sh /path/to/bundle_name.run [binary_name]
#
# If binary_name is omitted, it's guessed from the bundle name
# (e.g., worker-4.12.1-3-irix-bundle.0318261419.run → worker)
#
# Steps:
#   1. scp .run to IRIX
#   2. Extract (run the .run installer)
#   3. Smoke test: run the binary with --version or --help
#   4. Run irix-diag.sh for full par trace + diagnostics
#   5. scp the diag tarball back to Linux
#
# NOTE: Uses /usr/sgug/bin/bash on IRIX to avoid csh quoting hell.

set -euo pipefail

IRIX_HOST="edodd@192.168.0.81"
IRIX_APPS_DIR="apps"
IRIX_BASH="/usr/sgug/bin/bash"
MOGRIX_DIR="$(cd "$(dirname "$0")/.." && pwd)"

# Helper: run a bash script on IRIX via heredoc (bypasses csh entirely).
# csh is the IRIX login shell and can't do VAR=val syntax, so we use env
# to bootstrap LD_LIBRARYN32_PATH so bash can find its shared libs.
irix_bash() {
    ssh "$IRIX_HOST" "env LD_LIBRARYN32_PATH=/usr/sgug/lib32:/usr/lib32:/lib32 $IRIX_BASH --norc"
}

# --- Parse arguments ---
if [ $# -lt 1 ]; then
    echo "Usage: $0 <bundle.run> [binary_name]"
    echo ""
    echo "Examples:"
    echo "  $0 ~/mogrix_outputs/bundles/worker-4.12.1-3-irix-bundle.0318261419.run"
    echo "  $0 ~/mogrix_outputs/bundles/worker-4.12.1-3-irix-bundle.0318261419.run worker"
    exit 1
fi

BUNDLE_PATH="$1"
BUNDLE_FILE="$(basename "$BUNDLE_PATH")"
BUNDLE_NAME="${BUNDLE_FILE%.run}"

# Guess binary name from bundle: take everything before the first dash-digit
if [ $# -ge 2 ]; then
    BINARY="$2"
else
    BINARY="$(echo "$BUNDLE_NAME" | sed 's/-[0-9].*//')"
    echo "Guessed binary name: $BINARY"
fi

if [ ! -f "$BUNDLE_PATH" ]; then
    echo "ERROR: $BUNDLE_PATH not found"
    exit 1
fi

echo "=== Deploy & Test ==="
echo "  Bundle:  $BUNDLE_FILE"
echo "  Binary:  $BINARY"
echo "  Target:  $IRIX_HOST:~/$IRIX_APPS_DIR/"
echo ""

# --- Step 1: Upload bundle ---
echo "[1/5] Uploading bundle to IRIX..."
scp "$BUNDLE_PATH" "${IRIX_HOST}:"
echo "  Done."
echo ""

# --- Step 2: Extract bundle ---
echo "[2/5] Extracting bundle on IRIX..."
irix_bash <<EOF
export PATH=/usr/sgug/bin:/usr/bin:/bin
export LD_LIBRARYN32_PATH=/usr/sgug/lib32:/usr/lib32:/lib32
cd "\$HOME"
rm -rf "${BUNDLE_NAME}"
/bin/sh "./${BUNDLE_FILE}"
EOF
echo "  Done."
echo ""

# .run extracts to $HOME/<bundle_name>/ and creates $HOME/bin/<binary> symlinks
BUNDLE_DIR_REMOTE="\$HOME/${BUNDLE_NAME}"

# --- Step 3: Smoke test ---
echo "[3/5] Smoke test..."
SMOKE_OUTPUT=$(irix_bash <<EOF
export PATH=/usr/sgug/bin:/usr/bin:/bin
export LD_LIBRARYN32_PATH=/usr/sgug/lib32:/usr/lib32:/lib32

WRAPPER="\$HOME/${BUNDLE_NAME}/${BINARY}"
if [ ! -f "\$WRAPPER" ]; then
    WRAPPER="\$HOME/${BUNDLE_NAME}/_bin/${BINARY}"
fi
if [ ! -f "\$WRAPPER" ]; then
    echo "ERROR: Cannot find ${BINARY} in bundle"
    echo "Bundle contents:"
    ls "\$HOME/${BUNDLE_NAME}/" 2>/dev/null
    exit 1
fi
"\$WRAPPER" --version 2>&1 | head -5 || "\$WRAPPER" --help 2>&1 | head -5 || echo "No --version or --help output"
EOF
) || true
echo "  Smoke output:"
echo "$SMOKE_OUTPUT" | sed 's/^/    /'
echo ""

# --- Step 4: Run diagnostics ---
echo "[4/5] Running irix-diag.sh (par trace + full diagnostics)..."

# Upload the diag script
scp "$MOGRIX_DIR/tools/irix-diag.sh" "${IRIX_HOST}:irix-diag.sh"

# Run the diag script and capture the output tarball name
DIAG_OUTPUT=$(irix_bash <<EOF
export PATH=/usr/sgug/bin:/usr/bin:/bin:/usr/sbin
export LD_LIBRARYN32_PATH=/usr/sgug/lib32:/usr/lib32:/lib32
export DISPLAY=:0

/bin/sh "\$HOME/irix-diag.sh" "\$HOME/${BUNDLE_NAME}" "${BINARY}"
EOF
) || true
echo "$DIAG_OUTPUT" | sed 's/^/    /'
echo ""

# Extract the diag tarball path from the output
DIAG_FILE=$(echo "$DIAG_OUTPUT" | grep '\.tar\.gz' | grep 'diag_' | head -1 | sed 's/.*\(diag_[^ ]*\.tar\.gz\).*/\1/')
if [ -z "$DIAG_FILE" ]; then
    # Try to find it by listing recent files on IRIX
    DIAG_FILE=$(irix_bash <<EOF
ls -t \$HOME/diag_${BUNDLE_NAME}_*.tar.gz 2>/dev/null | head -1
EOF
    )
fi

if [ -z "$DIAG_FILE" ]; then
    echo "WARNING: Could not find diag tarball on IRIX"
    echo "  Check IRIX ~/diag_*.tar.gz manually"
    exit 1
fi

# Normalize — strip any leading path, we'll fetch from ~
DIAG_BASENAME="$(basename "$DIAG_FILE")"

# --- Step 5: Fetch diagnostics ---
echo "[5/5] Fetching diag tarball..."
scp "${IRIX_HOST}:${DIAG_BASENAME}" "${MOGRIX_DIR}/"
echo "  Saved: ${MOGRIX_DIR}/${DIAG_BASENAME}"
echo ""

# --- Extract locally for convenience ---
echo "Extracting locally..."
cd "$MOGRIX_DIR"
tar xzf "$DIAG_BASENAME" 2>/dev/null || gzip -d < "$DIAG_BASENAME" | tar xf - 2>/dev/null || true
DIAG_DIR="${DIAG_BASENAME%.tar.gz}"
if [ -d "$DIAG_DIR" ]; then
    echo "  Extracted: ${MOGRIX_DIR}/${DIAG_DIR}/"
    echo ""
    echo "=== Quick Summary ==="
    if [ -f "$DIAG_DIR/par_trace.txt" ]; then
        if grep -q "received signal \(SIGSEGV\|SIGBUS\|SIGABRT\)" "$DIAG_DIR/par_trace.txt"; then
            echo "  CRASH DETECTED in par trace"
            grep -E "received signal (SIGSEGV|SIGBUS|SIGABRT)" "$DIAG_DIR/par_trace.txt" | head -3 | sed 's/^/    /'
        else
            echo "  No crash signals in par trace — binary survived 10s"
        fi
    fi
    if [ -f "$DIAG_DIR/soname_check.txt" ]; then
        MISSING=$(grep -c 'NOT FOUND' "$DIAG_DIR/soname_check.txt" 2>/dev/null || echo 0)
        echo "  Missing sonames: $MISSING"
    fi
else
    echo "  (could not extract locally)"
fi

echo ""
echo "Done."
