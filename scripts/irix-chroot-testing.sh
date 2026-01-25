#!/bin/sh
#
# irix-chroot-testing.sh - Extract bootstrap tarball into chroot for testing
#
# This script extracts the bootstrap tarball into an existing chroot.
# It does NOT delete or recreate the chroot - that's your responsibility.
#
# Run this on IRIX as root.
#
# Usage: ./irix-chroot-testing.sh [tarball-path] [chroot-dir]
#
# Default tarball: /tmp/irix-bootstrap.tar.gz
# Default chroot:  /opt/chroot
#

# Configuration
TARBALL="${1:-/tmp/irix-bootstrap.tar.gz}"
CHROOT_DIR="${2:-/opt/chroot}"

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

# Check tarball exists
check_tarball() {
    if [ ! -f "$TARBALL" ]; then
        # Try uncompressed version
        UNCOMPRESSED="`echo $TARBALL | sed 's/\.gz$//'`"
        if [ -f "$UNCOMPRESSED" ]; then
            TARBALL="$UNCOMPRESSED"
            log_info "Using uncompressed tarball: $TARBALL"
        else
            log_error "Tarball not found: $TARBALL"
            log_error "Copy it from Linux build host first"
            exit 1
        fi
    fi
}

# Check chroot exists
check_chroot() {
    if [ ! -d "$CHROOT_DIR" ]; then
        log_error "Chroot directory does not exist: $CHROOT_DIR"
        log_error "Create it first or specify a different path"
        exit 1
    fi
    log_info "Using existing chroot: $CHROOT_DIR"
}

# Extract tarball into chroot
extract_tarball() {
    log_info "Extracting tarball into chroot..."
    log_info "Tarball: $TARBALL"
    log_info "Chroot:  $CHROOT_DIR"

    cd "$CHROOT_DIR"

    case "$TARBALL" in
        *.gz)
            if command -v gzcat >/dev/null 2>&1; then
                gzcat "$TARBALL" | tar xf -
            elif command -v gunzip >/dev/null 2>&1; then
                gunzip -c "$TARBALL" | tar xf -
            else
                log_error "No gzip decompressor found (gzcat or gunzip)"
                exit 1
            fi
            ;;
        *)
            tar xf "$TARBALL"
            ;;
    esac

    log_success "Tarball extracted"
}

# Show what was extracted
show_contents() {
    log_info "Checking extracted contents..."
    echo ""

    if [ -d "$CHROOT_DIR/usr/sgug" ]; then
        echo "SGUG directories:"
        ls "$CHROOT_DIR/usr/sgug/" 2>/dev/null
        echo ""

        if [ -d "$CHROOT_DIR/usr/sgug/bin" ]; then
            echo "Key binaries:"
            for bin in rpm tdnf openssl file; do
                if [ -f "$CHROOT_DIR/usr/sgug/bin/$bin" ]; then
                    echo "  [OK] /usr/sgug/bin/$bin"
                else
                    echo "  [MISSING] /usr/sgug/bin/$bin"
                fi
            done
        fi

        echo ""
        if [ -d "$CHROOT_DIR/usr/sgug/lib32" ]; then
            echo "Key libraries:"
            for lib in librpm.so libssl.so libz.so liblua.so libmagic.so libsolv.so; do
                if [ -f "$CHROOT_DIR/usr/sgug/lib32/$lib" ]; then
                    echo "  [OK] /usr/sgug/lib32/$lib"
                else
                    echo "  [MISSING] /usr/sgug/lib32/$lib"
                fi
            done
        fi
    else
        log_warn "/usr/sgug not found in chroot"
    fi
}

# Create chroot shell script
create_chroot_shell() {
    SHELL_SCRIPT="$CHROOT_DIR/usr/sgug/bin/chroot-shell.sh"

    log_info "Creating chroot shell script: $SHELL_SCRIPT"

    cat > "$SHELL_SCRIPT" << 'SHELLEOF'
#!/bin/sh
#
# chroot-shell.sh - Set up environment for testing in chroot
#
# Source this script or run it to get a shell with proper paths:
#   . /usr/sgug/bin/chroot-shell.sh
# or:
#   /usr/sgug/bin/chroot-shell.sh
#

# Clear conflicting environment
unset PKG_CONFIG_PATH
unset LDFLAGS
unset CFLAGS
unset CPPFLAGS
unset LD_LIBRARY_PATH
unset LD_LIBRARYN32_PATH
unset LD_LIBRARYN64_PATH

SGUG_ROOT=/usr/sgug
SGUG_BIN=$SGUG_ROOT/bin
SGUG_LIB=$SGUG_ROOT/lib32

PATH=$SGUG_BIN:/usr/bin:/bin:/usr/sbin
export PATH

LD_LIBRARYN32_PATH=$SGUG_LIB:/usr/lib32:/lib32:/usr/lib:/lib
export LD_LIBRARYN32_PATH

LC_ALL=C
export LC_ALL

echo "SGUG chroot environment configured"
echo "  PATH=$PATH"
echo "  LD_LIBRARYN32_PATH=$LD_LIBRARYN32_PATH"
echo ""
echo "Test commands:"
echo "  rpm --version"
echo "  tdnf --version"
echo ""

# If called directly (not sourced), start a shell
case "$0" in
    *chroot-shell.sh)
        exec /bin/sh
        ;;
esac
SHELLEOF

    chmod +x "$SHELL_SCRIPT"
    log_success "Created $SHELL_SCRIPT"
}

# Print manual testing instructions
print_instructions() {
    echo ""
    echo "========================================"
    echo "Bootstrap Extraction Complete"
    echo "========================================"
    echo ""
    echo "Chroot location: $CHROOT_DIR"
    echo ""
    echo "To test manually:"
    echo ""
    echo "  # Enter chroot"
    echo "  chroot $CHROOT_DIR /bin/sh"
    echo ""
    echo "  # Then source the environment script:"
    echo "  . /usr/sgug/bin/chroot-shell.sh"
    echo ""
    echo "  # Or run it directly for a configured shell:"
    echo "  /usr/sgug/bin/chroot-shell.sh"
    echo ""
    echo "  # Test rpm:"
    echo "  rpm --version"
    echo ""
    echo "  # Test tdnf:"
    echo "  tdnf --version"
    echo ""
    echo "To clean up (if needed):"
    echo "  rm -rf $CHROOT_DIR/usr/sgug"
    echo ""
}

# Main
main() {
    echo "==================================="
    echo "IRIX Bootstrap Chroot Extraction"
    echo "==================================="
    echo ""

    check_root
    check_tarball
    check_chroot
    extract_tarball
    show_contents
    create_chroot_shell
    print_instructions
}

main "$@"
