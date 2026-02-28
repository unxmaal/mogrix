#!/bin/bash
#
# Mogrix Build VM Cleanup Script
#
# Resets the staging environment to a clean state for validation builds.
# Run this before starting a fresh build chain.

set -e

MOGRIX_DIR="/home/edodd/projects/github/unxmaal/mogrix"
STAGING="/opt/sgug-staging/usr/sgug"

echo "=== Mogrix Build VM Cleanup ==="
echo ""

# Clean staging directories
echo "[1/5] Cleaning staging directories..."
rm -rf "${STAGING}/lib32/"*
rm -rf "${STAGING}/include/"*
echo "      Done."

# Restore compat headers
echo "[2/5] Restoring compat headers..."
cp -r "${MOGRIX_DIR}/cross/include/"* "${STAGING}/include/"
cp -r "${MOGRIX_DIR}/compat/include/"* "${STAGING}/include/"
echo "      Done."

# Build and install ALL runtime objects
echo "[3/5] Building runtime objects..."
"${MOGRIX_DIR}/scripts/build-runtime-objects.sh"
echo "      Done."

# Clean rpmbuild directories
echo "[4/5] Cleaning rpmbuild directories..."
rm -rf ~/rpmbuild/BUILD/*
rm -rf ~/rpmbuild/BUILDROOT/*
rm -rf ~/rpmbuild/RPMS/mips/*
echo "      Done."

# Verify
echo "[5/5] Verifying cleanup..."
echo ""
echo "      Staging lib32:"
ls "${STAGING}/lib32/" 2>/dev/null || echo "        (empty)"
echo ""
echo "      Staging include:"
ls "${STAGING}/include/" 2>/dev/null || echo "        (empty)"
echo ""
echo "      RPMS/mips:"
ls ~/rpmbuild/RPMS/mips/ 2>/dev/null || echo "        (empty)"
echo ""

echo "=== Cleanup Complete ==="
echo ""
echo "Ready to start validation build chain."
