#!/bin/sh
# Fallback for MCP irix_exec — same chroot-safe SSH pattern.
# Usage: tools/irix-exec.sh "command to run in chroot"
IRIX_HOST="${IRIX_HOST:-192.168.0.81}"
IRIX_USER="${IRIX_USER:-root}"
IRIX_CHROOT="${IRIX_CHROOT:-/opt/chroot}"

cmd="$1"
if [ -z "$cmd" ]; then
    echo "Usage: $0 'command'" >&2
    exit 1
fi

escaped_cmd=$(printf '%s' "$cmd" | sed "s/'/'\\\\''/g")
ssh -o ConnectTimeout=10 -o BatchMode=yes -o StrictHostKeyChecking=no \
    "${IRIX_USER}@${IRIX_HOST}" \
    "chroot ${IRIX_CHROOT} /usr/sgug/bin/sgug-exec /bin/sh -c '${escaped_cmd}'"
