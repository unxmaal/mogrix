#!/bin/sh
# Test termusic-server stays alive and termusic TUI starts
BUNDLE=/usr/people/edodd/apps/termusic-0.12.1-1-irix-bundle.0322261856

# Kill any old instances
pkill -f termusic-server 2>/dev/null
rm -f /tmp/termusic.socket

# Start server via wrapper (sets LD_LIBRARYN32_PATH)
$BUNDLE/termusic-server ~/Music &
SERVER_PID=$!
sleep 4

if kill -0 $SERVER_PID 2>/dev/null; then
    echo "PASS: termusic-server alive (pid $SERVER_PID)"
else
    echo "FAIL: termusic-server died"
    tail -10 /tmp/termusic-server.log 2>/dev/null
    exit 1
fi

# Check socket
if test -S /tmp/termusic.socket; then
    echo "PASS: UDS socket exists"
else
    echo "FAIL: no UDS socket"
fi

# Show server log
echo "--- server log ---"
tail -5 /tmp/termusic-server.log 2>/dev/null

# Try TUI with timeout (will fail without tty, but tests loading)
echo "--- TUI load test (3s timeout, expect FAIL on non-tty) ---"
timeout 3 $BUNDLE/termusic 2>/tmp/termusic-tui-err.txt
TUI_RC=$?
echo "TUI exit code: $TUI_RC"
cat /tmp/termusic-tui-err.txt 2>/dev/null

# Cleanup
kill $SERVER_PID 2>/dev/null
