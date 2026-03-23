#!/bin/sh
# Start termusic-server in background and check if it stays alive
BUNDLE=/usr/people/edodd/apps/termusic-0.12.1-1-irix-bundle.0322261856
cd $BUNDLE
./termusic-server ~/Music &
SERVER_PID=$!
sleep 4
if kill -0 $SERVER_PID 2>/dev/null; then
    echo "PASS: termusic-server running (pid $SERVER_PID)"
    # Check if socket exists
    ls -la /tmp/termusic.socket 2>/dev/null && echo "PASS: UDS socket exists"
    # Check log
    tail -5 /tmp/termusic-server.log 2>/dev/null
    # Leave it running for TUI test
else
    echo "FAIL: termusic-server died"
    cat /tmp/termusic-server.log 2>/dev/null | tail -20
    wait $SERVER_PID
    echo "exit code: $?"
fi
