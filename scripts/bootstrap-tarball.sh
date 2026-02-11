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
# Usage: ./scripts/bootstrap-tarball.sh [--repo-url URL]
#
# Options:
#   --repo-url URL   Override the tdnf repo baseurl (default: auto-detect build host IP)
#
# Output goes to: ./tmp/irix-bootstrap.tar.gz
#

set -euo pipefail

# Configuration - use project-local tmp directory
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
MIPS_RPM_DIR="${HOME}/mogrix_outputs/RPMS"
NOARCH_RPM_DIR="${HOME}/mogrix_outputs/RPMS"
TMP_DIR="${PROJECT_DIR}/tmp"
BOOTSTRAP_DIR="${TMP_DIR}/irix-bootstrap-work"
TARBALL="${TMP_DIR}/irix-bootstrap.tar"
REPO_URL=""  # Set via --repo-url or auto-detected
REPO_PORT=8090

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

    # Release/config packages (built as mips, not noarch, to get proper provides)
    "sgugrse-release-common-[0-9]*:mips"
    "sgugrse-release-[0-9]*:mips"

    # Development symlinks needed at runtime (libsolvext.so links against libz.so not libz.so.1)
    "zlib-ng-compat-devel-[0-9]*:mips"
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

    # Also include the RPM files themselves for rpm database registration
    log_info "Including RPM files for database registration..."
    mkdir -p "$BOOTSTRAP_DIR/tmp/bootstrap-rpms"
    for rpm in "${FOUND_RPMS[@]}"; do
        cp "$rpm" "$BOOTSTRAP_DIR/tmp/bootstrap-rpms/"
    done
    log_info "Copied ${#FOUND_RPMS[@]} RPM files to /tmp/bootstrap-rpms/"

    # Create missing directories that packages should own but may not in older builds
    log_info "Creating required directories..."
    mkdir -p "$BOOTSTRAP_DIR/usr/sgug/var/cache/tdnf"
    log_info "Created /usr/sgug/var/cache/tdnf"
}

# Install extra tools (sgugshell, sgug-exec) into bootstrap
install_extras() {
    log_info "Installing extra tools..."

    # sgugshell - interactive SGUG environment shell
    if [ -f "$PROJECT_DIR/tools/sgugshell" ]; then
        cp "$PROJECT_DIR/tools/sgugshell" "$BOOTSTRAP_DIR/usr/sgug/bin/sgugshell"
        chmod +x "$BOOTSTRAP_DIR/usr/sgug/bin/sgugshell"
        echo "  [OK] /usr/sgug/bin/sgugshell"
    else
        log_warn "tools/sgugshell not found - skipping"
    fi

    # sgug-exec - single-command wrapper
    if [ -f "$PROJECT_DIR/tools/sgug-exec" ]; then
        cp "$PROJECT_DIR/tools/sgug-exec" "$BOOTSTRAP_DIR/usr/sgug/bin/sgug-exec"
        chmod +x "$BOOTSTRAP_DIR/usr/sgug/bin/sgug-exec"
        echo "  [OK] /usr/sgug/bin/sgug-exec"
    else
        log_warn "tools/sgug-exec not found - skipping"
    fi
}

# Install tdnf repo config and tdnf.conf
generate_repo_config() {
    log_info "Installing tdnf repository configuration..."

    # Install mogrix.repo (public package server)
    mkdir -p "$BOOTSTRAP_DIR/usr/sgug/etc/yum.repos.d"
    cp "$PROJECT_DIR/configs/tdnf/mogrix.repo" "$BOOTSTRAP_DIR/usr/sgug/etc/yum.repos.d/mogrix.repo"

    # If --repo-url was provided, also create a local override repo
    if [ -n "$REPO_URL" ]; then
        log_info "Adding local repo override: $REPO_URL"
        cat > "$BOOTSTRAP_DIR/usr/sgug/etc/yum.repos.d/mogrix-local.repo" << EOF
[mogrix-local]
name=Mogrix Local Repository
baseurl=${REPO_URL}
enabled=1
gpgcheck=0
EOF
        echo "  [OK] /usr/sgug/etc/yum.repos.d/mogrix-local.repo (local override)"
    fi

    echo "  [OK] /usr/sgug/etc/yum.repos.d/mogrix.repo"

    # Write tdnf.conf (our version, not whatever the RPM shipped)
    mkdir -p "$BOOTSTRAP_DIR/usr/sgug/etc/tdnf"
    cp "$PROJECT_DIR/configs/tdnf/tdnf.conf" "$BOOTSTRAP_DIR/usr/sgug/etc/tdnf/tdnf.conf"
    echo "  [OK] /usr/sgug/etc/tdnf/tdnf.conf"
}

# Generate serve-repo.sh helper script
generate_serve_script() {
    log_info "Generating serve-repo.sh helper..."

    cat > "${TMP_DIR}/serve-repo.sh" << 'SERVEOF'
#!/bin/bash
#
# serve-repo.sh - Serve mogrix RPM repository for IRIX clients
#
# Copies all built RPMs to a temporary directory, generates repodata,
# and starts an HTTP server on port 8090.
#
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
RPM_SOURCE="${HOME}/mogrix_outputs/RPMS"
REPO_DIR="${SCRIPT_DIR}/mogrix-repo"

echo "==================================="
echo "Mogrix Repository Server"
echo "==================================="
echo ""

# Create/update repo directory
mkdir -p "$REPO_DIR"

echo "Copying RPMs from ${RPM_SOURCE}..."
cp "${RPM_SOURCE}"/*.rpm "$REPO_DIR/" 2>/dev/null || true
rpm_count=$(ls "$REPO_DIR"/*.rpm 2>/dev/null | wc -l)
echo "  ${rpm_count} RPMs in repository"
echo ""

echo "Generating repodata..."
python3 "${PROJECT_DIR}/scripts/create-repo.py" "$REPO_DIR"
echo ""

HOST_IP=$(hostname -I 2>/dev/null | awk '{print $1}')
echo "========================================"
echo "Serving repository at:"
echo "  http://${HOST_IP:-localhost}:8090/"
echo ""
echo "On IRIX, run:"
echo "  tdnf makecache"
echo "  tdnf list"
echo "========================================"
echo ""

cd "$REPO_DIR" && exec python3 -m http.server 8090
SERVEOF

    chmod +x "${TMP_DIR}/serve-repo.sh"
    log_info "Created: ${TMP_DIR}/serve-repo.sh"
}

# Verify critical files exist
verify_contents() {
    log_info "Verifying critical files..."

    local errors=0

    # Critical binaries
    for bin in rpm tdnf sgugshell sgug-exec; do
        if [ -f "$BOOTSTRAP_DIR/usr/sgug/bin/$bin" ]; then
            echo "  [OK] /usr/sgug/bin/$bin"
        else
            echo "  [MISSING] /usr/sgug/bin/$bin"
            errors=$((errors + 1))
        fi
    done

    # Critical libraries (use versioned names for ssl since that's the SONAME)
    for lib in librpm.so libtdnf.so libsolv.so libz.so libssl.so.3; do
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
    for cfg in /usr/sgug/etc/tdnf/tdnf.conf /usr/sgug/etc/rpm/macros.sqlite /usr/sgug/etc/yum.repos.d/mogrix.repo; do
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
    echo "The tarball is self-contained and includes:"
    echo "  - Extracted files under /usr/sgug/"
    echo "  - RPM files in /tmp/bootstrap-rpms/ for database registration"
    echo "  - sgugshell + sgug-exec environment wrappers"
    echo "  - tdnf repo config pointing to https://packages.mogrix.unxmaal.com/repo/"
    echo ""
    echo "Next steps:"
    echo ""
    echo "1. Start the repo server on this machine:"
    echo "   ./tmp/serve-repo.sh"
    echo ""
    echo "2. Copy tarball to IRIX and extract to chroot:"
    echo "   scp ${TARBALL}.gz root@<irix-host>:/tmp/"
    echo "   # On IRIX: cd /opt/chroot && gzcat /tmp/irix-bootstrap.tar.gz | tar xvf -"
    echo ""
    echo "3. Initialize rpm database and register packages:"
    echo "   chroot /opt/chroot /usr/sgug/bin/sgugshell"
    echo "   rpm --initdb"
    echo "   rpm -Uvh --nodeps /tmp/bootstrap-rpms/*.rpm"
    echo ""
    echo "4. Test tdnf (repo server must be running):"
    echo "   tdnf makecache"
    echo "   tdnf list"
    echo ""
}

# Parse arguments
parse_args() {
    while [ $# -gt 0 ]; do
        case "$1" in
            --repo-url)
                REPO_URL="$2"
                shift 2
                ;;
            --repo-url=*)
                REPO_URL="${1#*=}"
                shift
                ;;
            -h|--help)
                echo "Usage: $0 [--repo-url URL]"
                echo ""
                echo "Options:"
                echo "  --repo-url URL   Override the tdnf repo baseurl"
                echo "                   Default: auto-detect build host IP, port ${REPO_PORT}"
                exit 0
                ;;
            *)
                log_error "Unknown option: $1"
                exit 1
                ;;
        esac
    done
}

# Main
main() {
    parse_args "$@"

    echo "==================================="
    echo "IRIX Bootstrap Tarball Creator"
    echo "==================================="
    echo ""

    check_prerequisites
    validate_manifest
    extract_rpms
    install_extras
    generate_repo_config
    verify_contents
    show_contents
    create_tarball
    generate_serve_script
    cleanup
    print_next_steps
}

main "$@"
