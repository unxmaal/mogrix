#!/bin/ksh
# Link with native IRIX ld (not SGUG GNU ld)
/usr/bin/ld -shared -o /tmp/libhello-irix-native.so /tmp/hello.o -lc
RC=$?
echo "Return code: $RC"
if [ $RC -eq 0 ]; then
    ls -la /tmp/libhello-irix-native.so
fi
