#!/bin/sh
#
# create-irix-sysroot.sh - Capture IRIX system headers and libraries
#
# Run this ON an IRIX machine to create a sysroot tarball for cross-compilation.
# The output tarball is extracted to /opt/irix-sysroot/ on the Linux build host.
#
# Usage: sh create-irix-sysroot.sh [output_file]
#
# Default output: /tmp/irix-sysroot.tar.gz
#
# What it captures:
#   /usr/include/  - System headers (for --sysroot with clang)
#   /usr/lib32/    - N32 ABI shared libraries and link stubs
#   /lib32/        - Core N32 runtime (rld, libc.so.1)
#   /usr/lib64/    - N64 ABI shared libraries (if present)
#   /lib64/        - Core N64 runtime (if present)
#
# On the Linux build host:
#   mkdir -p /opt/irix-sysroot
#   cd /opt/irix-sysroot && gzcat irix-sysroot.tar.gz | tar xf -
#

OUTPUT="${1:-/tmp/irix-sysroot.tar.gz}"

echo "==================================="
echo "IRIX Sysroot Creator"
echo "==================================="
echo ""

# Verify we're on IRIX
if [ "`uname -s`" != "IRIX" ] && [ "`uname -s`" != "IRIX64" ]; then
    echo "Error: this script must be run on IRIX." >&2
    exit 1
fi

echo "System: `uname -a`"
echo "Machine: `hinv 2>/dev/null | head -1`"
echo ""

# Verify required directories exist
errors=0
for dir in /usr/include /usr/lib32 /lib32; do
    if [ -d "$dir" ]; then
        count=`ls "$dir" | wc -l`
        size=`du -sk "$dir" | awk '{printf "%.0f MB", $1/1024}'`
        echo "  [OK] $dir ($count entries, $size)"
    else
        echo "  [MISSING] $dir"
        errors=`expr $errors + 1`
    fi
done

# N64 directories are optional (not all installs have them)
OPTIONAL_DIRS=""
for dir in /usr/lib64 /lib64; do
    if [ -d "$dir" ]; then
        count=`ls "$dir" | wc -l`
        size=`du -sk "$dir" | awk '{printf "%.0f MB", $1/1024}'`
        echo "  [OK] $dir ($count entries, $size)"
        OPTIONAL_DIRS="$OPTIONAL_DIRS `echo $dir | sed 's|^/||'`"
    else
        echo "  [--] $dir (not present, skipping)"
    fi
done
echo ""

if [ $errors -gt 0 ]; then
    echo "Error: $errors required directory(ies) missing." >&2
    exit 1
fi

# Collect symlink targets from /usr/include that point outside the tree.
# IRIX has several subsystems installed under /usr/ with symlinks in
# /usr/include (e.g., /usr/include/Xm -> ../Motif-1.2/include/Xm).
# Without the targets, the sysroot has dangling symlinks.
EXTRA_DIRS=""
echo "Checking for symlink targets in /usr/include..."
for link in /usr/include/*; do
    if [ -L "$link" ]; then
        target=`ls -l "$link" | awk '{print $NF}'`
        # Resolve relative paths
        case "$target" in
            /*) resolved="$target" ;;
            *)  resolved="/usr/include/$target" ;;
        esac
        # Normalize (remove /../)
        resolved=`cd "\`dirname $resolved\`" 2>/dev/null && echo "\`pwd\`/\`basename $resolved\`"`
        # Check if the resolved path is outside /usr/include
        case "$resolved" in
            /usr/include/*) ;;
            *)
                # Strip leading / for tar
                dir=`echo "$resolved" | sed 's|^/||'`
                if [ -d "$resolved" ]; then
                    echo "  [SYMLINK] $link -> $target (adding $dir)"
                    EXTRA_DIRS="$EXTRA_DIRS $dir"
                fi
                ;;
        esac
    fi
done
echo ""

# Create the tarball
# Use tar with full paths so extraction preserves the directory structure:
#   usr/include/...
#   usr/lib32/...
#   lib32/...
#   usr/lib64/...        (if present)
#   lib64/...            (if present)
#   usr/Motif-1.2/...    (if symlinked from /usr/include/Xm)
#   etc.
echo "Creating tarball: $OUTPUT"
echo "This may take several minutes on slower hardware..."
echo ""

cd / && tar cf - usr/include usr/lib32 lib32 $OPTIONAL_DIRS $EXTRA_DIRS | gzip > "$OUTPUT"
status=$?

if [ $status -ne 0 ]; then
    echo "Error: tarball creation failed." >&2
    rm -f "$OUTPUT"
    exit 1
fi

size=`ls -l "$OUTPUT" | awk '{mb = $5 / 1048576; printf "%.0f MB", mb}'`
echo ""
echo "==================================="
echo "Sysroot tarball created: $OUTPUT ($size)"
echo "==================================="
echo ""
echo "Transfer to Linux build host:"
echo "  scp $OUTPUT buildhost:/tmp/"
echo ""
echo "Extract on Linux:"
echo "  mkdir -p /opt/irix-sysroot"
echo "  cd /opt/irix-sysroot && tar xzf /tmp/`basename $OUTPUT`"
echo ""
