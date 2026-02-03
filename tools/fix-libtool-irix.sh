#!/bin/bash
# fix-libtool-irix.sh - Fix libtool for IRIX shared library building
#
# Libtool often gets confused by cross-compilation and defaults to:
#   build_libtool_libs=no (no shared libraries)
#   deplibs_check_method="unknown" (fails to link)
#   version_type=none (no versioned shared libraries)
#
# This script fixes these settings for IRIX N32 builds.
#
# Usage: fix-libtool-irix.sh [libtool-file]
#        If no file specified, defaults to ./libtool
#
# Exit codes:
#   0 - Success
#   1-6 - safepatch error
#   10 - Script error (file not found, etc.)

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
SAFEPATCH="${SCRIPT_DIR}/safepatch"
LIBTOOL="${1:-./libtool}"

if [ ! -f "$LIBTOOL" ]; then
    echo "ERROR: libtool file not found: $LIBTOOL" >&2
    echo "Run this after ./configure" >&2
    exit 10
fi

if [ ! -x "$SAFEPATCH" ]; then
    echo "ERROR: safepatch not found or not executable: $SAFEPATCH" >&2
    exit 10
fi

echo "=== Fixing libtool for IRIX: $LIBTOOL ==="

# Enable shared library building
# Some libtool versions use different quoting, try both patterns
if grep -q 'build_libtool_libs=no' "$LIBTOOL"; then
    "$SAFEPATCH" "$LIBTOOL" \
        --old 'build_libtool_libs=no' \
        --new 'build_libtool_libs=yes' \
        --count 0 --no-backup --quiet
    echo "  Fixed: build_libtool_libs=yes"
fi

# Fix dependency checking
if grep -q 'deplibs_check_method="unknown"' "$LIBTOOL"; then
    "$SAFEPATCH" "$LIBTOOL" \
        --old 'deplibs_check_method="unknown"' \
        --new 'deplibs_check_method="pass_all"' \
        --count 0 --no-backup --quiet
    echo "  Fixed: deplibs_check_method=pass_all"
fi

# Fix version type
if grep -q '^version_type=none$' "$LIBTOOL"; then
    "$SAFEPATCH" "$LIBTOOL" \
        --old $'version_type=none\n' \
        --new $'version_type=linux\n' \
        --count 0 --no-backup --quiet
    echo "  Fixed: version_type=linux"
fi

# Fix soname_spec if empty
# Note: The $ variables MUST be backslash-escaped to prevent shell expansion in libtool
if grep -q '^soname_spec=""$' "$LIBTOOL"; then
    "$SAFEPATCH" "$LIBTOOL" \
        --old 'soname_spec=""' \
        --new 'soname_spec="\$libname\${shared_ext}\$major"' \
        --count 0 --no-backup --quiet
    echo "  Fixed: soname_spec"
fi

# Fix library_names_spec if empty
if grep -q '^library_names_spec=""$' "$LIBTOOL"; then
    "$SAFEPATCH" "$LIBTOOL" \
        --old 'library_names_spec=""' \
        --new 'library_names_spec="\$libname\${shared_ext}\$versuffix \$libname\${shared_ext}\$major \$libname\${shared_ext}"' \
        --count 0 --no-backup --quiet
    echo "  Fixed: library_names_spec"
fi

# Fix runpath_var if set to LD_RUN_PATH (causes rpath issues)
if grep -q '^runpath_var=LD_RUN_PATH$' "$LIBTOOL"; then
    "$SAFEPATCH" "$LIBTOOL" \
        --old 'runpath_var=LD_RUN_PATH' \
        --new 'runpath_var=DIE_RPATH_DIE' \
        --count 0 --no-backup --quiet
    echo "  Fixed: runpath_var (disabled rpath)"
fi

echo "=== libtool fixed for IRIX ==="
