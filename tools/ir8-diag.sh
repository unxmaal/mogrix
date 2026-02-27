#!/bin/sh
# ir8-diag.sh — Diagnostic script for ir8 browser crashes
# Run from the ir8 bundle directory (where the 'ir8' wrapper script lives)
# Usage: sh ir8-diag.sh > ir8-diag.log 2>&1

echo "=== ir8 Diagnostic Report ==="
echo "Date: `date`"
echo "User: `id`"
echo "Hostname: `hostname`"
echo "Uname: `uname -a`"
echo ""

echo "=== 1. Bundle directory ==="
dir=`/bin/dirname "$0"`
case "$dir" in
    /*) ;;
    *)  dir="`/bin/pwd`/$dir" ;;
esac
echo "Script location: $dir"
ls -la "$dir/ir8" 2>&1
ls -la "$dir/_bin/ir8" 2>&1
echo ""

echo "=== 2. WebKit subprocess executables ==="
ls -la "$dir/libexec/webkit2gtk-4.0/" 2>&1
echo ""

echo "=== 3. Key directories ==="
echo "HOME=$HOME"
echo "XDG_RUNTIME_DIR=$XDG_RUNTIME_DIR"
ls -lad "$HOME/.cache" 2>&1
ls -lad "$HOME/.local" 2>&1
ls -lad "$HOME/.local/share" 2>&1
ls -lad "$HOME/.config/ir8" 2>&1
ls -lad /tmp/.xdg-runtime-* 2>&1
echo ""

echo "=== 4. /tmp writable + free space ==="
touch /tmp/.ir8-diag-test 2>&1 && echo "/tmp is writable" && rm /tmp/.ir8-diag-test || echo "/tmp is NOT writable"
df -k /tmp
echo ""

echo "=== 5. Shared memory files ==="
ls -la /tmp/.shm_* 2>&1
echo ""

echo "=== 6. Wrapper script contents ==="
echo "--- first 60 lines of ir8 wrapper ---"
head -60 "$dir/ir8"
echo ""

echo "=== 7. Library path check ==="
echo "LD_LIBRARYN32_PATH=$LD_LIBRARYN32_PATH"
ls -la "$dir/_lib32/" 2>&1 | head -20
echo "..."
ls "$dir/_lib32/" 2>&1 | wc -l
echo "total files in _lib32"
echo ""

echo "=== 8. GSettings schemas ==="
ls -la "$dir/share/glib-2.0/schemas/gschemas.compiled" 2>&1
echo ""

echo "=== 9. GIO modules ==="
ls -la "$dir/_lib32/gio/modules/" 2>&1
echo ""

echo "=== 10. Fontconfig ==="
ls -la "$dir/share/fontconfig/" 2>&1
ls -la "$HOME/.cache/fontconfig/" 2>&1
echo ""

echo "=== 11. Test run with WEBKIT_DEBUG ==="
echo "Running ir8 --version..."
"$dir/ir8" --version 2>&1
echo "Exit code: $?"
echo ""

echo "=== 12. Full debug launch (5 second timeout) ==="
echo "Launching ir8 with debug env. Will kill after 5 seconds."
echo "If you have X11 (DISPLAY set), this will try to open a window."
echo "DISPLAY=$DISPLAY"
if [ -z "$DISPLAY" ]; then
    echo "WARNING: DISPLAY not set, skipping GUI launch"
else
    # Redirect WebProcess stderr to a file so we can capture subprocess crashes
    WEBKIT_SUBPROCESS_STDERR="$dir/webprocess_stderr.txt"
    export G_MESSAGES_DEBUG=all
    export WEBKIT_DEBUG=Process

    # Launch ir8 in background, capture all output
    "$dir/ir8" "ir8-about:home" > "$dir/ir8_debug_stdout.txt" 2> "$dir/ir8_debug_stderr.txt" &
    IR8_PID=$!
    echo "ir8 PID: $IR8_PID"

    # Wait 5 seconds then kill
    sleep 5
    kill $IR8_PID 2>/dev/null
    sleep 1
    kill -9 $IR8_PID 2>/dev/null

    echo ""
    echo "--- ir8 stdout ---"
    cat "$dir/ir8_debug_stdout.txt" 2>/dev/null
    echo ""
    echo "--- ir8 stderr ---"
    cat "$dir/ir8_debug_stderr.txt" 2>/dev/null
    echo ""

    # Check for WebProcess crash artifacts
    echo "--- WebProcess stderr (if captured) ---"
    cat "$WEBKIT_SUBPROCESS_STDERR" 2>/dev/null || echo "(not captured)"
    echo ""

    # Check for core files
    echo "--- Recent core files ---"
    ls -lt /tmp/core* core* "$HOME/core"* 2>/dev/null | head -5
fi

echo ""
echo "=== 13. Socket path length check ==="
RUNTIME="/tmp/.xdg-runtime-`id -un`"
echo "XDG_RUNTIME_DIR would be: $RUNTIME"
echo "Path length: `echo $RUNTIME | wc -c` bytes (max ~108 for AF_UNIX)"
echo ""

echo "=== Done ==="
echo "Please send this entire log to edodd."
