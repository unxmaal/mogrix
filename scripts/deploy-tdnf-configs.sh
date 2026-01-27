#!/bin/bash
# Deploy tdnf configuration files to IRIX chroot
#
# Usage: ./deploy-tdnf-configs.sh [irix_host] [chroot_path]
#
# Defaults:
#   irix_host: edodd@192.168.0.81
#   chroot_path: /opt/chroot

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
MOGRIX_DIR="$(dirname "$SCRIPT_DIR")"
CONFIG_DIR="$MOGRIX_DIR/configs/tdnf"

IRIX_HOST="${1:-edodd@192.168.0.81}"
CHROOT_PATH="${2:-/opt/chroot}"

# Get local IP for repo URL
LOCAL_IP=$(hostname -I | awk '{print $1}')
REPO_PORT=8080

echo "Deploying tdnf configs to IRIX..."
echo "  IRIX host: $IRIX_HOST"
echo "  Chroot: $CHROOT_PATH"
echo "  Repo URL: http://${LOCAL_IP}:${REPO_PORT}/mips/"
echo ""

# Create temp files with correct repo URL
TEMP_DIR=$(mktemp -d)
cp "$CONFIG_DIR/tdnf.conf" "$TEMP_DIR/"
sed "s|baseurl=.*|baseurl=http://${LOCAL_IP}:${REPO_PORT}/mips/|" "$CONFIG_DIR/irix-local.repo" > "$TEMP_DIR/irix-local.repo"

echo "Creating directories on IRIX..."
ssh "$IRIX_HOST" "mkdir -p ${CHROOT_PATH}/etc/tdnf ${CHROOT_PATH}/etc/yum.repos.d ${CHROOT_PATH}/var/cache/tdnf"

echo "Copying configuration files..."
scp "$TEMP_DIR/tdnf.conf" "${IRIX_HOST}:${CHROOT_PATH}/etc/tdnf/"
scp "$TEMP_DIR/irix-local.repo" "${IRIX_HOST}:${CHROOT_PATH}/etc/yum.repos.d/"

rm -rf "$TEMP_DIR"

echo ""
echo "Configuration deployed. To test on IRIX:"
echo ""
echo "  # Enter chroot"
echo "  chroot $CHROOT_PATH /bin/sh"
echo ""
echo "  # Set up environment"
echo "  export LD_LIBRARY_PATH=/usr/sgug/lib32"
echo "  export PATH=/usr/sgug/bin:\$PATH"
echo ""
echo "  # Test tdnf"
echo "  tdnf repolist"
echo "  tdnf makecache"
echo "  tdnf list available"
