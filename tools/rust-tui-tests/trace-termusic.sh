#!/bin/sh
# Trace termusic TUI under par to see what syscalls it makes while "stuck"
# Run this ON the IRIX machine in a real terminal (not over SSH test harness)
BUNDLE=$HOME/termusic-0.12.1-1-irix-bundle.0322262040

# Start server first
$HOME/bin/termusic-server ~/Music &
sleep 2

# Run TUI under par — capture 15 seconds of syscalls
# -s = syscall events, -SS = full trace, -i = follow forks, -T = thread IDs
echo "Starting termusic TUI under par (15s capture)..."
echo "Press arrow-down when the TUI appears, then wait."
par -s -SS -i -T -o /tmp/termusic-par.txt $HOME/bin/termusic &
PAR_PID=$!
sleep 15
kill $PAR_PID 2>/dev/null
kill %1 2>/dev/null

echo "Done. Par trace at /tmp/termusic-par.txt"
echo "Last 50 lines:"
tail -50 /tmp/termusic-par.txt
