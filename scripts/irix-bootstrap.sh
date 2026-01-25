#!/bin/sh
#
# irix-bootstrap.sh - Install bootstrap packages to real IRIX filesystem
#
# WARNING: This script modifies the real /usr/sgug filesystem!
#          Run irix-chroot-testing.sh first to verify binaries work.
#
# This script:
# 1. Extracts the bootstrap tarball to / (creates /usr/sgug/...)
# 2. Initializes the RPM database
# 3. Properly installs packages via rpm (for database tracking)
#
# Usage: ./irix-bootstrap.sh [tarball] [rpm-dir]
#
# Default tarball: /tmp/irix-bootstrap.tar.gz
# Default rpm-dir: /tmp/rpms
#

# Configuration
TARBALL="${1:-/tmp/irix-bootstrap.tar.gz}"
RPM_DIR="${2:-/tmp/rpms}"
SGUG_PREFIX="/usr/sgug"
RPM_DBPATH="$SGUG_PREFIX/lib32/sysimage/rpm"

# Colors
log_info() {
    echo "[INFO] $*"
}

log_warn() {
    echo "[WARN] $*"
}

log_error() {
    echo "[ERROR] $*"
}

log_success() {
    echo "[OK] $*"
}

# Check if running as root
check_root() {
    if [ "`id -u`" != "0" ]; then
        log_error "This script must be run as root"
        exit 1
    fi
}

# Confirm with user
confirm_install() {
    echo "========================================"
    echo "WARNING: REAL FILESYSTEM MODIFICATION"
    echo "========================================"
    echo ""
    echo "This script will:"
    echo "  1. Extract tarball to / (creates $SGUG_PREFIX/...)"
    echo "  2. Initialize RPM database at $RPM_DBPATH"
    echo "  3. Install RPMs from $RPM_DIR"
    echo ""
    echo "Files will be written to the REAL filesystem!"
    echo ""
    echo "Did you run irix-chroot-testing.sh first? (Recommended)"
    echo ""
    printf "Continue? (yes/no): "
    read -r answer
    if [ "$answer" != "yes" ]; then
        echo "Aborted."
        exit 0
    fi
}

# Check prerequisites
check_prerequisites() {
    if [ ! -f "$TARBALL" ]; then
        # Try uncompressed
        if [ -f "${TARBALL%.gz}" ]; then
            TARBALL="${TARBALL%.gz}"
        else
            log_error "Tarball not found: $TARBALL"
            exit 1
        fi
    fi
    log_info "Tarball: $TARBALL"

    if [ ! -d "$RPM_DIR" ]; then
        log_warn "RPM directory not found: $RPM_DIR"
        log_warn "Proper RPM installation will be skipped."
        log_warn "Create it with: mkdir -p $RPM_DIR"
        log_warn "Copy RPMs with: scp ~/rpmbuild/RPMS/mips/*.mips.rpm irix:$RPM_DIR/"
    else
        rpm_count=`find "$RPM_DIR" -name "*.mips.rpm" 2>/dev/null | wc -l`
        log_info "Found $rpm_count RPM files in $RPM_DIR"
    fi
}

# Extract bootstrap tarball
extract_bootstrap() {
    log_info "Extracting bootstrap tarball to /..."

    cd /

    case "$TARBALL" in
        *.gz)
            if command -v gzcat >/dev/null 2>&1; then
                gzcat "$TARBALL" | tar xf -
            elif command -v gunzip >/dev/null 2>&1; then
                gunzip -c "$TARBALL" | tar xf -
            else
                log_error "No gzip decompressor found"
                exit 1
            fi
            ;;
        *)
            tar xf "$TARBALL"
            ;;
    esac

    log_success "Bootstrap files extracted to $SGUG_PREFIX"
}

# Set up environment
setup_environment() {
    log_info "Setting up environment..."

    # Export paths for this session
    export LD_LIBRARY_PATH="$SGUG_PREFIX/lib32:$LD_LIBRARY_PATH"
    export PATH="$SGUG_PREFIX/bin:$PATH"

    log_info "LD_LIBRARY_PATH=$LD_LIBRARY_PATH"
    log_info "PATH=$PATH"
}

# Verify rpm works
verify_rpm() {
    log_info "Verifying rpm binary..."

    RPM_BIN="$SGUG_PREFIX/bin/rpm"

    if [ ! -f "$RPM_BIN" ]; then
        log_error "rpm binary not found at $RPM_BIN"
        exit 1
    fi

    if "$RPM_BIN" --version; then
        log_success "rpm binary works"
    else
        log_error "rpm binary failed to run"
        log_error "Check library dependencies"
        exit 1
    fi
}

# Initialize RPM database
init_rpm_database() {
    log_info "Initializing RPM database..."

    # Create database directory
    mkdir -p "$RPM_DBPATH"

    RPM_BIN="$SGUG_PREFIX/bin/rpm"

    # Initialize database
    if "$RPM_BIN" --initdb --dbpath "$RPM_DBPATH"; then
        log_success "RPM database initialized at $RPM_DBPATH"
    else
        log_error "Failed to initialize RPM database"
        exit 1
    fi

    # Show database contents
    echo ""
    echo "Database files:"
    ls -la "$RPM_DBPATH/"
}

# Create symlink for rpm database (as rpm's %pre script does)
create_db_symlink() {
    log_info "Creating RPM database symlink..."

    # rpm's %pre script creates: /var/lib/rpm -> /usr/sgug/lib32/sysimage/rpm
    # This is for compatibility with tools that expect /var/lib/rpm

    mkdir -p /var/lib

    if [ -L /var/lib/rpm ]; then
        log_info "Symlink /var/lib/rpm already exists"
        ls -la /var/lib/rpm
    elif [ -d /var/lib/rpm ]; then
        log_warn "/var/lib/rpm is a directory, not creating symlink"
    else
        ln -s "$RPM_DBPATH" /var/lib/rpm
        log_success "Created symlink: /var/lib/rpm -> $RPM_DBPATH"
    fi
}

# Install RPMs properly (for database tracking)
install_rpms() {
    if [ ! -d "$RPM_DIR" ]; then
        log_warn "Skipping RPM installation (no RPM directory)"
        return
    fi

    log_info "Installing RPMs for proper database tracking..."

    RPM_BIN="$SGUG_PREFIX/bin/rpm"

    # Install order matters - dependencies first
    # Using --nodeps because files are already in place from tarball
    # Using --force to update database for already-installed files

    INSTALL_ORDER="
        zlib-ng-compat-*.mips.rpm
        bzip2-libs-*.mips.rpm
        xz-libs-*.mips.rpm
        popt-1*.mips.rpm
        openssl-libs-*.mips.rpm
        libxml2-2*.mips.rpm
        lua-libs-*.mips.rpm
        file-libs-*.mips.rpm
        libcurl-8*.mips.rpm
        rpm-libs-*.mips.rpm
        rpm-4*.mips.rpm
        libsolv-0*.mips.rpm
        tdnf-cli-libs-*.mips.rpm
        tdnf-3*.mips.rpm
    "

    cd "$RPM_DIR"

    for pattern in $INSTALL_ORDER; do
        for rpm in $pattern; do
            if [ -f "$rpm" ]; then
                echo "Installing: $rpm"
                "$RPM_BIN" --dbpath "$RPM_DBPATH" -ivh --nodeps --force "$rpm" 2>&1 || {
                    log_warn "Failed to install $rpm (may be ok if already registered)"
                }
            fi
        done
    done

    echo ""
    log_info "Installed packages:"
    "$RPM_BIN" --dbpath "$RPM_DBPATH" -qa | sort
}

# Test final installation
test_installation() {
    log_info "Testing installation..."
    echo ""

    RPM_BIN="$SGUG_PREFIX/bin/rpm"
    TDNF_BIN="$SGUG_PREFIX/bin/tdnf"

    echo "rpm version:"
    "$RPM_BIN" --version

    echo ""
    echo "rpm query:"
    "$RPM_BIN" --dbpath "$RPM_DBPATH" -qa | wc -l
    echo " packages in database"

    if [ -f "$TDNF_BIN" ]; then
        echo ""
        echo "tdnf version:"
        "$TDNF_BIN" --version 2>&1 || log_warn "tdnf may need configuration"
    fi
}

# Print completion message
print_completion() {
    echo ""
    echo "========================================"
    echo "Bootstrap Complete!"
    echo "========================================"
    echo ""
    echo "The following are now available:"
    echo "  - rpm:  $SGUG_PREFIX/bin/rpm"
    echo "  - tdnf: $SGUG_PREFIX/bin/tdnf"
    echo ""
    echo "RPM database: $RPM_DBPATH"
    echo ""
    echo "To use in future sessions, add to your shell startup:"
    echo ""
    echo "  # For bash/sh:"
    echo "  export LD_LIBRARY_PATH=$SGUG_PREFIX/lib32:\$LD_LIBRARY_PATH"
    echo "  export PATH=$SGUG_PREFIX/bin:\$PATH"
    echo ""
    echo "  # For csh/tcsh:"
    echo "  setenv LD_LIBRARY_PATH $SGUG_PREFIX/lib32:\$LD_LIBRARY_PATH"
    echo "  setenv PATH $SGUG_PREFIX/bin:\$PATH"
    echo ""
    echo "Or use the sgugshell wrapper if available:"
    echo "  $SGUG_PREFIX/bin/sgugshell"
    echo ""
    echo "Next steps:"
    echo "  1. Configure tdnf repositories"
    echo "  2. Build/install additional packages"
    echo ""
}

# Main
main() {
    echo "==================================="
    echo "IRIX Bootstrap Installation"
    echo "==================================="
    echo ""

    check_root
    check_prerequisites
    confirm_install
    extract_bootstrap
    setup_environment
    verify_rpm
    init_rpm_database
    create_db_symlink
    install_rpms
    test_installation
    print_completion
}

main "$@"
