#!/bin/bash
#
# bootstrap-tarball.sh - Create IRIX bootstrap tarball on Linux build host
#
# This script extracts all cross-compiled MIPS RPMs into a single tarball
# that can be extracted on IRIX without needing rpm/cpio on the target.
#
# Usage: ./scripts/bootstrap-tarball.sh
#
# Output goes to: ./tmp/irix-bootstrap.tar.gz
#

set -euo pipefail

# Configuration - use project-local tmp directory
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
RPM_DIR="${HOME}/rpmbuild/RPMS/mips"
TMP_DIR="${PROJECT_DIR}/tmp"
BOOTSTRAP_DIR="${TMP_DIR}/irix-bootstrap-work"
TARBALL="${TMP_DIR}/irix-bootstrap.tar"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

log_info() {
    echo -e "${GREEN}[INFO]${NC} $*"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $*"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $*"
}

# Check prerequisites
check_prerequisites() {
    if ! command -v rpm2cpio &> /dev/null; then
        log_error "rpm2cpio not found. Install rpm-tools or rpm package."
        exit 1
    fi

    if ! command -v cpio &> /dev/null; then
        log_error "cpio not found. Install cpio package."
        exit 1
    fi

    if [ ! -d "$RPM_DIR" ]; then
        log_error "RPM directory not found: $RPM_DIR"
        log_error "Build packages first with: mogrix build ... --cross"
        exit 1
    fi

    local rpm_count
    rpm_count=$(find "$RPM_DIR" -name "*.mips.rpm" 2>/dev/null | wc -l)
    if [ "$rpm_count" -eq 0 ]; then
        log_error "No .mips.rpm files found in $RPM_DIR"
        exit 1
    fi
    log_info "Found $rpm_count MIPS RPM files"

    # Ensure tmp directory exists
    mkdir -p "$TMP_DIR"
}

# Extract ALL RPMs to bootstrap directory
extract_rpms() {
    log_info "Creating bootstrap directory: $BOOTSTRAP_DIR"
    rm -rf "$BOOTSTRAP_DIR"
    mkdir -p "$BOOTSTRAP_DIR"

    log_info "Extracting ALL MIPS RPMs..."
    cd "$BOOTSTRAP_DIR"

    local extracted=0

    for rpm in "$RPM_DIR"/*.mips.rpm; do
        if [ -f "$rpm" ]; then
            local basename
            basename=$(basename "$rpm")
            echo "  Extracting: $basename"
            rpm2cpio "$rpm" | cpio -idm 2>/dev/null
            extracted=$((extracted + 1))
        fi
    done

    log_info "Extracted $extracted RPMs"
}

# Show what was extracted
show_contents() {
    log_info "Bootstrap directory contents:"
    echo ""
    echo "Directories:"
    find "$BOOTSTRAP_DIR" -type d | sed "s|$BOOTSTRAP_DIR||" | grep -v '^$' | sort | head -20
    echo ""
    echo "Key binaries:"
    find "$BOOTSTRAP_DIR" -type f \( -name "rpm" -o -name "tdnf" -o -name "openssl" \) 2>/dev/null | sed "s|$BOOTSTRAP_DIR||"
    echo ""
    echo "RPM libraries:"
    find "$BOOTSTRAP_DIR" -name "librpm*.so*" 2>/dev/null | sed "s|$BOOTSTRAP_DIR||"
    echo ""
    echo "Total shared libraries:"
    find "$BOOTSTRAP_DIR" -name "*.so*" -type f 2>/dev/null | wc -l
    echo " .so files found"
    echo ""

    local total_size
    total_size=$(du -sh "$BOOTSTRAP_DIR" | cut -f1)
    log_info "Total extracted size: $total_size"
}

# Create the tarball
create_tarball() {
    log_info "Creating tarball: $TARBALL"

    # Remove old tarball if exists
    rm -f "$TARBALL" "${TARBALL}.gz"

    # Create tarball (uncompressed for maximum IRIX compatibility)
    tar cf "$TARBALL" -C "$BOOTSTRAP_DIR" .

    local size
    size=$(du -sh "$TARBALL" | cut -f1)
    log_info "Tarball created: $TARBALL ($size)"

    # Also create compressed version
    log_info "Creating compressed version..."
    gzip -k "$TARBALL"
    local gz_size
    gz_size=$(du -sh "${TARBALL}.gz" | cut -f1)
    log_info "Compressed: ${TARBALL}.gz ($gz_size)"
}

# Cleanup
cleanup() {
    log_info "Cleaning up temporary directory..."
    rm -rf "$BOOTSTRAP_DIR"
}

# Print next steps
print_next_steps() {
    echo ""
    echo "========================================"
    echo "Bootstrap tarball created successfully!"
    echo "========================================"
    echo ""
    echo "Output files:"
    echo "  ${TARBALL}"
    echo "  ${TARBALL}.gz"
    echo ""
    echo "Next steps:"
    echo ""
    echo "1. Copy files to IRIX:"
    echo "   scp ${TARBALL}.gz edodd@192.168.0.81:/tmp/"
    echo "   scp scripts/irix-chroot-testing.sh scripts/irix-bootstrap.sh edodd@192.168.0.81:/tmp/"
    echo "   ssh edodd@192.168.0.81 'mkdir -p /tmp/rpms'"
    echo "   scp ~/rpmbuild/RPMS/mips/*.mips.rpm edodd@192.168.0.81:/tmp/rpms/"
    echo ""
    echo "2. On IRIX as root:"
    echo "   cd /tmp"
    echo "   chmod +x irix-chroot-testing.sh irix-bootstrap.sh"
    echo "   ./irix-chroot-testing.sh"
    echo ""
}

# Main
main() {
    echo "==================================="
    echo "IRIX Bootstrap Tarball Creator"
    echo "==================================="
    echo ""

    check_prerequisites
    extract_rpms
    show_contents
    create_tarball
    cleanup
    print_next_steps
}

main "$@"
