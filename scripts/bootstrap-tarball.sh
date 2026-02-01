#!/bin/bash
#
# bootstrap-tarball.sh - Create IRIX bootstrap tarball on Linux build host
#
# This script extracts REQUIRED cross-compiled RPMs into a single tarball
# that can be extracted on IRIX without needing rpm/cpio on the target.
#
# The manifest of required packages is defined in this script. If any
# required package is missing, the script fails loudly.
#
# Usage: ./scripts/bootstrap-tarball.sh
#
# Output goes to: ./tmp/irix-bootstrap.tar.gz
#

set -euo pipefail

# Configuration - use project-local tmp directory
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
MIPS_RPM_DIR="${HOME}/rpmbuild/RPMS/mips"
NOARCH_RPM_DIR="${HOME}/rpmbuild/RPMS/noarch"
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

# =============================================================================
# BOOTSTRAP MANIFEST
# =============================================================================
# These are the REQUIRED packages for a working tdnf bootstrap.
# Order matters - dependencies are listed before dependents.
# Format: "package-pattern:arch" where arch is "mips" or "noarch"
#
# If adding a new package, add it in dependency order.
# =============================================================================

REQUIRED_PACKAGES=(
    # Base libraries (no deps on other mogrix packages)
    "zlib-ng-compat-[0-9]*:mips"
    "bzip2-libs-[0-9]*:mips"
    "xz-libs-[0-9]*:mips"
    "popt-[0-9]*:mips"
    "openssl-libs-[0-9]*:mips"
    "lua-libs-[0-9]*:mips"
    "file-libs-[0-9]*:mips"
    "sqlite-libs-[0-9]*:mips"

    # Mid-level (depend on base)
    "libxml2-[0-9]*:mips"
    "libcurl-[0-9]*:mips"

    # RPM and package management
    "rpm-libs-[0-9]*:mips"
    "rpm-[0-9]*:mips"
    "libsolv-[0-9]*:mips"
    "tdnf-cli-libs-[0-9]*:mips"
    "tdnf-[0-9]*:mips"

    # Release/config packages
    "sgugrse-release-[0-9]*:noarch"
)

# =============================================================================

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

    # Ensure tmp directory exists
    mkdir -p "$TMP_DIR"
}

# Find RPM matching pattern
find_rpm() {
    local pattern="$1"
    local arch="$2"
    local rpm_dir
    local extension

    if [ "$arch" = "mips" ]; then
        rpm_dir="$MIPS_RPM_DIR"
        extension="mips.rpm"
    else
        rpm_dir="$NOARCH_RPM_DIR"
        extension="noarch.rpm"
    fi

    # Find matching RPM (use ls to expand glob, take first match)
    local found
    found=$(ls "$rpm_dir"/${pattern}.${extension} 2>/dev/null | head -1)

    if [ -n "$found" ] && [ -f "$found" ]; then
        echo "$found"
        return 0
    fi

    return 1
}

# Validate all required packages exist
validate_manifest() {
    log_info "Validating required packages..."

    local missing=0
    local found_rpms=()

    for entry in "${REQUIRED_PACKAGES[@]}"; do
        local pattern="${entry%:*}"
        local arch="${entry#*:}"

        local rpm_path
        if rpm_path=$(find_rpm "$pattern" "$arch"); then
            local basename
            basename=$(basename "$rpm_path")
            echo "  [OK] $basename"
            found_rpms+=("$rpm_path")
        else
            echo "  [MISSING] $pattern ($arch)"
            missing=$((missing + 1))
        fi
    done

    echo ""

    if [ "$missing" -gt 0 ]; then
        log_error "$missing required package(s) missing!"
        log_error "Build missing packages with: mogrix build <package> --cross"
        exit 1
    fi

    log_info "All ${#REQUIRED_PACKAGES[@]} required packages found"

    # Store found RPMs for extraction
    FOUND_RPMS=("${found_rpms[@]}")
}

# Extract required RPMs to bootstrap directory
extract_rpms() {
    log_info "Creating bootstrap directory: $BOOTSTRAP_DIR"
    rm -rf "$BOOTSTRAP_DIR"
    mkdir -p "$BOOTSTRAP_DIR"

    log_info "Extracting ${#FOUND_RPMS[@]} required RPMs..."
    cd "$BOOTSTRAP_DIR"

    local extracted=0

    for rpm in "${FOUND_RPMS[@]}"; do
        local basename
        basename=$(basename "$rpm")
        echo "  Extracting: $basename"
        rpm2cpio "$rpm" | cpio -idm 2>/dev/null
        extracted=$((extracted + 1))
    done

    log_info "Extracted $extracted RPMs"
}

# Verify critical files exist
verify_contents() {
    log_info "Verifying critical files..."

    local errors=0

    # Critical binaries
    for bin in rpm tdnf; do
        if [ -f "$BOOTSTRAP_DIR/usr/sgug/bin/$bin" ]; then
            echo "  [OK] /usr/sgug/bin/$bin"
        else
            echo "  [MISSING] /usr/sgug/bin/$bin"
            errors=$((errors + 1))
        fi
    done

    # Critical libraries
    for lib in librpm.so libtdnf.so libsolv.so libz.so libssl.so; do
        if [ -f "$BOOTSTRAP_DIR/usr/sgug/lib32/$lib" ]; then
            echo "  [OK] /usr/sgug/lib32/$lib"
        else
            echo "  [MISSING] /usr/sgug/lib32/$lib"
            errors=$((errors + 1))
        fi
    done

    # Critical directories (created by packages per package-rules.md)
    for dir in /var/run /usr/sgug/etc/yum.repos.d /usr/sgug/var/cache/tdnf; do
        if [ -d "$BOOTSTRAP_DIR$dir" ]; then
            echo "  [OK] $dir"
        else
            echo "  [MISSING] $dir"
            errors=$((errors + 1))
        fi
    done

    # Critical config files
    for cfg in /usr/sgug/etc/tdnf/tdnf.conf /usr/sgug/etc/rpm/macros.sqlite; do
        if [ -f "$BOOTSTRAP_DIR$cfg" ]; then
            echo "  [OK] $cfg"
        else
            echo "  [MISSING] $cfg"
            errors=$((errors + 1))
        fi
    done

    # Critical symlinks
    if [ -L "$BOOTSTRAP_DIR/var/lib/rpm" ]; then
        echo "  [OK] /var/lib/rpm (symlink)"
    else
        echo "  [MISSING] /var/lib/rpm symlink"
        errors=$((errors + 1))
    fi

    # libz.so symlink (needed by libsolvext)
    if [ -L "$BOOTSTRAP_DIR/usr/sgug/lib32/libz.so" ] || [ -f "$BOOTSTRAP_DIR/usr/sgug/lib32/libz.so" ]; then
        echo "  [OK] /usr/sgug/lib32/libz.so"
    else
        echo "  [MISSING] /usr/sgug/lib32/libz.so"
        errors=$((errors + 1))
    fi

    echo ""

    if [ "$errors" -gt 0 ]; then
        log_error "$errors critical file(s) missing!"
        log_error "The packages may need to be rebuilt with updated rules."
        log_error "Check rules/methods/package-rules.md for ownership requirements."
        exit 1
    fi

    log_info "All critical files verified"
}

# Show summary of contents
show_contents() {
    log_info "Bootstrap directory summary:"
    echo ""

    echo "Key binaries:"
    find "$BOOTSTRAP_DIR" -type f \( -name "rpm" -o -name "tdnf" -o -name "openssl" \) 2>/dev/null | sed "s|$BOOTSTRAP_DIR||" | sort
    echo ""

    echo "Shared libraries:"
    find "$BOOTSTRAP_DIR" -name "*.so*" -type f 2>/dev/null | wc -l | tr -d '\n'
    echo " .so files"
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
    echo "1. Copy to IRIX:"
    echo "   scp ${TARBALL}.gz root@192.168.0.81:/tmp/"
    echo ""
    echo "2. On IRIX as root, extract to chroot:"
    echo "   cd /opt/chroot"
    echo "   gzcat /tmp/irix-bootstrap.tar.gz | tar xvf -"
    echo ""
    echo "3. Initialize rpm database:"
    echo "   chroot /opt/chroot /bin/sh"
    echo "   export LD_LIBRARYN32_PATH=/usr/sgug/lib32"
    echo "   /usr/sgug/bin/rpm --initdb"
    echo ""
    echo "4. Test tdnf:"
    echo "   /usr/sgug/bin/sgug-exec /usr/sgug/bin/tdnf repolist"
    echo ""
}

# Main
main() {
    echo "==================================="
    echo "IRIX Bootstrap Tarball Creator"
    echo "==================================="
    echo ""

    check_prerequisites
    validate_manifest
    extract_rpms
    verify_contents
    show_contents
    create_tarball
    cleanup
    print_next_steps
}

main "$@"
