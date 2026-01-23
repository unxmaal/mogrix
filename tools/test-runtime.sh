#!/bin/ksh
# Test runtime loading of stripped shared library

cd /tmp

echo "=== Stripping main.o ==="
objcopy --remove-section=.MIPS.abiflags main.o main-stripped.o
RC=$?
echo "objcopy RC: $RC"

if [ $RC -ne 0 ]; then
    exit 1
fi

echo ""
echo "=== Linking test executable ==="
ld -o test-hello main-stripped.o -L/tmp -lhello-stripped -lc -dynamic-linker /lib32/rld
RC=$?
echo "ld RC: $RC"

if [ $RC -ne 0 ]; then
    exit 1
fi

ls -la test-hello

echo ""
echo "=== Checking test executable sections ==="
readelf -S test-hello | grep -i mips

echo ""
echo "=== Testing with odump ==="
/usr/bin/odump -D test-hello 2>&1 | head -20
echo "odump RC: $?"

echo ""
echo "=== Running test executable ==="
export LD_LIBRARYN32_PATH=/tmp:/usr/sgug/lib32:/usr/lib32:/lib32
./test-hello
RC=$?
echo "test-hello exit code: $RC"
if [ $RC -eq 0 ]; then
    echo "SUCCESS!"
else
    echo "FAILED"
fi
