#!/bin/bash
#
# Mogrix Build VM Cleanup Script
#
# Resets the staging environment to a clean state for validation builds.
# Run this before starting a fresh build chain.

set -e

MOGRIX_DIR="/home/edodd/projects/github/unxmaal/mogrix"
STAGING="/opt/sgug-staging/usr/sgug"
CROSS="/opt/cross/bin"

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

# Build and install runtime libraries
echo "[3/5] Building runtime libraries..."
TMPDIR=$(mktemp -d)

# libsoft_float_stubs.a
"${STAGING}/bin/irix-cc" -c "${MOGRIX_DIR}/compat/runtime/soft_float_stubs.c" -o "${TMPDIR}/soft_float_stubs.o" 2>/dev/null
"${CROSS}/llvm-ar" rcs "${STAGING}/lib32/libsoft_float_stubs.a" "${TMPDIR}/soft_float_stubs.o"

# libatomic_stub.a (if source exists)
if [[ -f "${MOGRIX_DIR}/compat/runtime/libatomic_stub.c" ]]; then
    "${STAGING}/bin/irix-cc" -c "${MOGRIX_DIR}/compat/runtime/libatomic_stub.c" -o "${TMPDIR}/libatomic_stub.o" 2>/dev/null
    "${CROSS}/llvm-ar" rcs "${STAGING}/lib32/libatomic.a" "${TMPDIR}/libatomic_stub.o"
fi

rm -rf "${TMPDIR}"
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
