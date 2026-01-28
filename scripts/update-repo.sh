#!/bin/bash
# Update IRIX RPM repository with newly built packages
#
# Usage: ./update-repo.sh
#
# Copies all .mips.rpm files from ~/rpmbuild/RPMS/mips/ to the repo
# and regenerates metadata.

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
MOGRIX_DIR="$(dirname "$SCRIPT_DIR")"
REPO_DIR="${HOME}/irix-repo/mips"
RPM_SOURCE="${HOME}/rpmbuild/RPMS/mips"

echo "Updating IRIX RPM repository..."

# Create repo directory if needed
mkdir -p "$REPO_DIR"

# Copy RPMs
if [ -d "$RPM_SOURCE" ] && ls "$RPM_SOURCE"/*.rpm 1> /dev/null 2>&1; then
    echo "Copying RPMs from $RPM_SOURCE..."
    cp "$RPM_SOURCE"/*.rpm "$REPO_DIR/"

    RPM_COUNT=$(ls -1 "$REPO_DIR"/*.rpm 2>/dev/null | wc -l)
    echo "  $RPM_COUNT RPMs in repository"
else
    echo "No RPMs found in $RPM_SOURCE"
    exit 1
fi

# Generate metadata
echo "Generating repository metadata..."
python3 "$MOGRIX_DIR/scripts/create-repo.py" "$REPO_DIR"

echo ""
echo "Repository updated: $REPO_DIR"
echo "Run ./start-repo-server.sh to serve it."
