#!/bin/ksh
# Test linking with SGUG ld but suppress modern features

cd /tmp

# Try SGUG ld with -z noexecstack to suppress GNU_STACK
/usr/sgug/bin/ld -shared -z noexecstack -o libhello-nostack.so hello.o -lc
RC=$?
echo "SGUG ld -z noexecstack: RC=$RC"
if [ $RC -eq 0 ]; then
    ls -la libhello-nostack.so
    # Test with odump
    echo "Testing with odump..."
    /usr/bin/odump -D libhello-nostack.so 2>&1 | head -20
fi
