#!/bin/sh
# irix-diag.sh — Mogrix bundle diagnostic collector for IRIX
#
# Collects par trace, rld library loading, NEEDED chains, syslog errors,
# and bundle structure into a single tarball for troubleshooting.
#
# Usage:
#   sh irix-diag.sh <bundle_dir> [binary_name]
#
# Examples:
#   sh irix-diag.sh ~/apps/decker-1.64-1-irix-bundle.0310261849
#   sh irix-diag.sh ~/apps/decker-1.64-1-irix-bundle.0310261849 decker
#   sh irix-diag.sh ~/apps/tmux-3.5a-1-irix-bundle.0310261848 tmux
#
# If binary_name is omitted, the script collects bundle info only (no par trace).
# Output: ~/diag_<name>_<timestamp>.tar.gz  (scp this to the Linux box)
#
# The script is safe to run as edodd — no root required.

BUNDLE_DIR="$1"
BINARY="$2"

if [ -z "$BUNDLE_DIR" ]; then
    echo "Usage: sh irix-diag.sh <bundle_dir> [binary_name]"
    echo ""
    echo "  bundle_dir   Path to extracted bundle directory"
    echo "  binary_name  Name of the binary to trace (e.g. decker, tmux)"
    echo ""
    echo "Examples:"
    echo "  sh irix-diag.sh ~/apps/decker-1.64-1-irix-bundle.0310261849"
    echo "  sh irix-diag.sh ~/apps/decker-1.64-1-irix-bundle.0310261849 decker"
    exit 1
fi

# Resolve to absolute path
case "$BUNDLE_DIR" in
    /*) ;;
    *)  BUNDLE_DIR="`/bin/pwd`/$BUNDLE_DIR" ;;
esac

# Strip trailing slash
BUNDLE_DIR=`echo "$BUNDLE_DIR" | sed 's:/*$::'`

if [ ! -d "$BUNDLE_DIR" ]; then
    echo "ERROR: $BUNDLE_DIR is not a directory"
    exit 1
fi

BUNDLE_NAME=`/bin/basename "$BUNDLE_DIR"`
TIMESTAMP=`date +%Y%m%d_%H%M%S`
DIAG_DIR="/tmp/diag_${BUNDLE_NAME}_${TIMESTAMP}"
# Use $HOME if writable, otherwise fall back to /tmp
_HOME="${HOME:-/tmp}"
if [ ! -w "$_HOME" ]; then
    _HOME="/tmp"
fi
OUTPUT="${_HOME}/diag_${BUNDLE_NAME}_${TIMESTAMP}.tar.gz"

/sbin/mkdir -p "$DIAG_DIR"
echo "Collecting diagnostics for: $BUNDLE_NAME"
echo "Output will be: $OUTPUT"
echo ""

# ============================================================
# 1. Bundle structure
# ============================================================
echo "  [1/8] Bundle structure..."
ls -laR "$BUNDLE_DIR" > "$DIAG_DIR/bundle_tree.txt" 2>&1

# ============================================================
# 2. Wrapper script contents (trampolines)
# ============================================================
echo "  [2/8] Wrapper scripts..."
/sbin/mkdir -p "$DIAG_DIR/wrappers"
for f in "$BUNDLE_DIR"/*; do
    name=`/bin/basename "$f"`
    case "$name" in
        _bin|_sbin|_lib32|share|etc|install|uninstall|README) continue ;;
    esac
    if [ -f "$f" ]; then
        head -30 "$f" > "$DIAG_DIR/wrappers/$name" 2>/dev/null
    fi
done

# ============================================================
# 3. Library inventory — NEEDED chains for all bundled .so files
# ============================================================
echo "  [3/8] Library NEEDED chains..."
(
if [ -d "$BUNDLE_DIR/_lib32" ]; then
    _diag_libs=`ls "$BUNDLE_DIR/_lib32"/*.so* 2>/dev/null`
    for lib in $_diag_libs; do
        [ -f "$lib" ] || continue
        name=`/bin/basename "$lib"`
        # Skip symlinks (only process real files)
        if [ -h "$lib" ]; then
            target=`ls -l "$lib" | sed 's/.* -> //'`
            echo "$name -> $target (symlink)" >> "$DIAG_DIR/lib_needed.txt"
            continue
        fi
        echo "=== $name ===" >> "$DIAG_DIR/lib_needed.txt"
        elfdump -L "$lib" 2>/dev/null | grep -E 'NEEDED|SONAME|RPATH|RUNPATH' >> "$DIAG_DIR/lib_needed.txt"
        echo "" >> "$DIAG_DIR/lib_needed.txt"
    done
else
    echo "No _lib32 directory found" > "$DIAG_DIR/lib_needed.txt"
fi
) 2>/dev/null

# ============================================================
# 4. Binary NEEDED chains
# ============================================================
echo "  [4/8] Binary NEEDED chains..."
(
for bindir in _bin _sbin; do
    [ -d "$BUNDLE_DIR/$bindir" ] || continue
    for bin in "$BUNDLE_DIR/$bindir"/*; do
        [ -f "$bin" ] || continue
        [ -h "$bin" ] && continue
        name=`/bin/basename "$bin"`
        echo "=== $bindir/$name ===" >> "$DIAG_DIR/bin_needed.txt"
        elfdump -L "$bin" 2>/dev/null | grep -E 'NEEDED|SONAME|RPATH|RUNPATH' >> "$DIAG_DIR/bin_needed.txt"
        file "$bin" >> "$DIAG_DIR/bin_needed.txt" 2>/dev/null
        echo "" >> "$DIAG_DIR/bin_needed.txt"
    done
done
) 2>/dev/null

# ============================================================
# 5. Soname resolution check — can rld find every NEEDED lib?
# ============================================================
echo "  [5/8] Soname resolution check..."
(
    echo "Checking if every NEEDED soname can be found..."
    echo "Bundle _lib32: $BUNDLE_DIR/_lib32"
    echo "System fallback: /usr/lib32"
    echo ""

    # Collect all NEEDED sonames
    all_needed=""
    _diag_files=`ls "$BUNDLE_DIR/_lib32"/*.so* "$BUNDLE_DIR/_bin"/* "$BUNDLE_DIR/_sbin"/* 2>/dev/null`
    for f in $_diag_files; do
        [ -f "$f" ] || continue
        [ -h "$f" ] && continue
        needs=`elfdump -L "$f" 2>/dev/null | grep NEEDED | sed 's/.*\[//' | sed 's/\]//'`
        for n in $needs; do
            all_needed="$all_needed $n"
        done
    done

    # Deduplicate
    all_needed=`echo $all_needed | tr ' ' '\n' | sort -u`

    missing=0
    for soname in $all_needed; do
        found=""
        if [ -f "$BUNDLE_DIR/_lib32/$soname" ] || [ -h "$BUNDLE_DIR/_lib32/$soname" ]; then
            found="bundle"
        elif [ -f "/usr/lib32/$soname" ]; then
            found="system"
        elif [ -f "/usr/lib32/internal/$soname" ]; then
            found="system-internal"
        elif [ -f "/lib32/$soname" ]; then
            found="system-lib32"
        fi

        if [ -n "$found" ]; then
            echo "  OK   $soname  ($found)"
        else
            echo "  MISS $soname  *** NOT FOUND ***"
            missing=`expr $missing + 1`
        fi
    done

    echo ""
    if [ "$missing" -gt 0 ]; then
        echo "WARNING: $missing soname(s) not found!"
    else
        echo "All sonames resolved."
    fi
) > "$DIAG_DIR/soname_check.txt" 2>/dev/null

# ============================================================
# 6. Recent rld errors from syslog
# ============================================================
echo "  [6/8] Syslog rld errors..."
{
    echo "=== Last 50 rld messages from syslog ==="
    grep rld /var/adm/SYSLOG 2>/dev/null | tail -50
} > "$DIAG_DIR/syslog_rld.txt" 2>&1

# ============================================================
# 7. System info
# ============================================================
echo "  [7/8] System info..."
{
    echo "=== uname ==="
    uname -a
    echo ""
    echo "=== hinv (brief) ==="
    hinv 2>/dev/null | head -20
    echo ""
    echo "=== DISPLAY ==="
    echo "DISPLAY=${DISPLAY:-<not set>}"
    echo ""
    echo "=== LD_LIBRARYN32_PATH ==="
    echo "LD_LIBRARYN32_PATH=${LD_LIBRARYN32_PATH:-<not set>}"
    echo ""
    echo "=== _RLDN32_LIST ==="
    echo "_RLDN32_LIST=${_RLDN32_LIST:-<not set>}"
    echo ""
    echo "=== /usr/lib32/libX11* ==="
    ls -la /usr/lib32/libX11* 2>/dev/null
    echo ""
    echo "=== /usr/lib32/libpthread* ==="
    ls -la /usr/lib32/libpthread* 2>/dev/null
} > "$DIAG_DIR/sysinfo.txt" 2>&1

# ============================================================
# 8. Par trace (if binary specified)
# ============================================================
if [ -n "$BINARY" ]; then
    echo "  [8/8] Par trace of '$BINARY'..."

    # Find the wrapper script
    WRAPPER="$BUNDLE_DIR/$BINARY"
    if [ ! -f "$WRAPPER" ]; then
        # Maybe it's a direct binary path
        WRAPPER="$BUNDLE_DIR/_bin/$BINARY"
    fi

    if [ ! -f "$WRAPPER" ]; then
        echo "  WARNING: Cannot find $BINARY in bundle, skipping par trace"
        echo "Cannot find $BINARY" > "$DIAG_DIR/par_trace.txt"
    else
        # Snapshot syslog line count before run
        SYSLOG_BEFORE=`cat /var/adm/SYSLOG 2>/dev/null | wc -l`

        # Run par with:
        #   -s   syscall events
        #   -i   follow forks
        #   -SS  full syscall trace (not just summary)
        #   -a 200  print up to 200 bytes of ascii args (shows full paths)
        #   -T   show thread/process IDs
        # Timeout after 10 seconds (GUI apps won't exit on their own)
        DISPLAY="${DISPLAY:-:0}"
        export DISPLAY

        # Use a subshell with alarm to timeout
        (
            par -siSS -T -a 200 -o "$DIAG_DIR/par_trace.txt" "$WRAPPER" &
            PAR_PID=$!
            # Wait up to 10 seconds
            sleep 10
            kill $PAR_PID 2>/dev/null
            # Also kill any children (the actual binary)
            kill -9 $PAR_PID 2>/dev/null
        ) > "$DIAG_DIR/par_stdout.txt" 2>&1

        # Capture any new syslog entries from the run
        SYSLOG_AFTER=`cat /var/adm/SYSLOG 2>/dev/null | wc -l`
        SYSLOG_NEW=`expr $SYSLOG_AFTER - $SYSLOG_BEFORE`
        if [ "$SYSLOG_NEW" -gt 0 ]; then
            tail -${SYSLOG_NEW} /var/adm/SYSLOG > "$DIAG_DIR/syslog_during_run.txt" 2>&1
        fi
    fi
else
    echo "  [8/8] Skipping par trace (no binary specified)"
    echo "No binary specified — run with: sh irix-diag.sh <bundle> <binary>" > "$DIAG_DIR/par_trace.txt"
fi

# ============================================================
# Package it up
# ============================================================
echo ""
echo "Packaging..."
cd /tmp
gtar czf "$OUTPUT" "diag_${BUNDLE_NAME}_${TIMESTAMP}" 2>/dev/null
if [ $? -ne 0 ]; then
    # Fallback: try tar + gzip separately
    tar cf - "diag_${BUNDLE_NAME}_${TIMESTAMP}" | gzip > "$OUTPUT" 2>/dev/null
fi
rm -rf "$DIAG_DIR"

echo ""
echo "Done! Diagnostic tarball:"
echo "  $OUTPUT"
echo ""
echo "Transfer to Linux with:"
echo "  scp blue:$OUTPUT /home/edodd/projects/github/unxmaal/mogrix/"
echo ""
echo "Contents:"
echo "  bundle_tree.txt      — full ls -laR of bundle"
echo "  wrappers/            — trampoline shell scripts (first 30 lines)"
echo "  lib_needed.txt       — NEEDED/SONAME for every bundled .so"
echo "  bin_needed.txt       — NEEDED for every binary"
echo "  soname_check.txt     — resolution check: can rld find each soname?"
echo "  syslog_rld.txt       — last 50 rld errors from syslog"
echo "  sysinfo.txt          — uname, hinv, env vars, system X11 libs"
if [ -n "$BINARY" ]; then
    echo "  par_trace.txt        — full par syscall trace (10s capture)"
    echo "  par_stdout.txt       — stdout/stderr from the run"
    echo "  syslog_during_run.txt — syslog entries generated during trace"
fi
