#!/bin/ksh
# Strip .MIPS.abiflags from object file before linking
# Run this through /opt/sgug-exec

cd /tmp

echo "Original hello.o sections:"
readelf -S hello.o | grep -i mips

echo ""
echo "Stripping .MIPS.abiflags..."
objcopy --remove-section=.MIPS.abiflags hello.o hello-stripped.o
RC=$?
echo "objcopy RC: $RC"

if [ $RC -eq 0 ]; then
    echo ""
    echo "Stripped hello-stripped.o sections:"
    readelf -S hello-stripped.o | grep -i mips

    echo ""
    echo "Linking with SGUG ld..."
    ld -shared -z noexecstack -o libhello-stripped.so hello-stripped.o -lc
    RC=$?
    echo "ld RC: $RC"

    if [ $RC -eq 0 ]; then
        ls -la libhello-stripped.so
        echo ""
        echo "Checking program headers..."
        readelf -l libhello-stripped.so
        echo ""
        echo "Checking sections..."
        readelf -S libhello-stripped.so | grep -i mips
        echo ""
        echo "Testing with odump..."
        /usr/bin/odump -D libhello-stripped.so 2>&1 | head -20
        echo "odump RC: $?"
    fi
fi
