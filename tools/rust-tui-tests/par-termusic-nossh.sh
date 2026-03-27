#!/bin/sh
# Run termusic under par non-interactively to capture the spin-loop syscall pattern
# Deploy to IRIX and run: /bin/sh /usr/people/edodd/par-termusic-nossh.sh
BUNDLE_DIR=/usr/people/edodd/termusic-0.12.1-1-irix-bundle.0322262040
BIN=$BUNDLE_DIR/_bin/termusic
LIB=$BUNDLE_DIR/_lib32
export LD_LIBRARYN32_PATH="$LIB:$LIB/engines-3:$LIB/ossl-modules:/usr/lib32"

# Start server
$BUNDLE_DIR/termusic-server /usr/people/edodd/Music > /dev/null 2>&1 &
sleep 2

# Par trace TUI for 5 seconds - just capture the syscall summary
par -s -i -T $BIN < /dev/null > /tmp/termusic-par-stdout.txt 2>&1 &
PAR_PID=$!
sleep 5
kill $PAR_PID 2>/dev/null
wait $PAR_PID 2>/dev/null

# Show the syscall summary (last section of par output)
echo "=== Par stdout ==="
cat /tmp/termusic-par-stdout.txt 2>/dev/null | tail -40

# Kill everything
pkill termusic 2>/dev/null
