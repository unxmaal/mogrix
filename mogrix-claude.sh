#!/bin/bash
# ~/bin/mogrix-claude.sh
SESSION="${1:-$(claude sessions list --json 2>/dev/null | jq -r '.[0].id // empty')}"
RESUME_FLAG=""
[[ -n "$SESSION" ]] && RESUME_FLAG="--resume $SESSION"

cd /home/edodd/projects/github/unxmaal/mogrix || exit 1
exec systemd-run --user --pty \
  --property=ProtectSystem=strict \
  --property=ProtectHome=no \
  --property=ReadWritePaths=/src \
  --property=ReadWritePaths=/opt/cross \
  --property=ReadWritePaths=/opt/libdicl \
  --property=ReadWritePaths=/opt/sgug-staging \
  --property=ReadWritePaths=/tmp \
  --property=PrivateDevices=no \
  --property=ProtectKernelTunables=yes \
  --property=ProtectKernelModules=yes \
  --property=ProtectControlGroups=yes \
  --property=LockPersonality=yes \
  --property=MemoryDenyWriteExecute=no \
  --property=RestrictRealtime=yes \
  --property=SystemCallArchitectures=native \
  --property=BindPaths=/opt/faketemp:/tmp \
  --property=UMask=077 \
  --property=WorkingDirectory="$(pwd)" \
  --setenv=HOME="$HOME" \
  claude --dangerously-skip-permissions $RESUME_FLAG
