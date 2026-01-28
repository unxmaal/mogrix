#!/bin/bash
# Start HTTP server for IRIX RPM repository
#
# Usage: ./start-repo-server.sh [port]
#
# Default port is 8080

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
MOGRIX_DIR="$(dirname "$SCRIPT_DIR")"
REPO_DIR="${HOME}/irix-repo"
PORT="${1:-8080}"

# Create repo directory if needed
mkdir -p "$REPO_DIR/mips"

# Check if there are RPMs to serve
if [ ! -f "$REPO_DIR/mips/repodata/repomd.xml" ]; then
    echo "No repository metadata found. Run update-repo.sh first."
    exit 1
fi

# Get local IP
LOCAL_IP=$(hostname -I | awk '{print $1}')

echo "Starting IRIX RPM repository server..."
echo "  URL: http://${LOCAL_IP}:${PORT}/mips/"
echo "  Repo dir: ${REPO_DIR}"
echo ""
echo "To use on IRIX, add to /etc/yum.repos.d/irix-local.repo:"
echo "  baseurl=http://${LOCAL_IP}:${PORT}/mips/"
echo ""
echo "Press Ctrl+C to stop."
echo ""

cd "$REPO_DIR"
exec python3 -m http.server "$PORT"
