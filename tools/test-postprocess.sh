#!/bin/ksh
# Test post-processing shared library to strip problematic sections

cd /tmp

echo "=== Original cross-compiled library (libhello-native.so) ==="
readelf -S libhello-native.so | grep -i mips

echo ""
echo "=== Creating stripped copy ==="
objcopy --remove-section=.MIPS.abiflags libhello-native.so libhello-postproc.so
RC=$?
echo "objcopy RC: $RC"

if [ $RC -ne 0 ]; then
    echo "objcopy failed"
    exit 1
fi

echo ""
echo "=== Stripped library sections ==="
readelf -S libhello-postproc.so | grep -i mips

echo ""
echo "=== Stripped library program headers ==="
readelf -l libhello-postproc.so

echo ""
echo "=== Testing with odump ==="
/usr/bin/odump -D libhello-postproc.so 2>&1 | head -20
echo "odump RC: $?"
