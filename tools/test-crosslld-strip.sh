#!/bin/ksh
# Test stripping .MIPS.abiflags from cross-LLD compiled shared library

cd /tmp

echo "=== Original cross-LLD library ==="
readelf -l libhello-crosslld.so
readelf -S libhello-crosslld.so | grep -i mips

echo ""
echo "=== Testing odump on original (should crash) ==="
/usr/bin/odump -D libhello-crosslld.so 2>&1 | head -5
echo "odump RC: $?"

echo ""
echo "=== Stripping .MIPS.abiflags section ==="
objcopy --remove-section=.MIPS.abiflags libhello-crosslld.so libhello-crosslld-stripped.so
echo "objcopy RC: $?"

echo ""
echo "=== Stripped library sections ==="
readelf -S libhello-crosslld-stripped.so | grep -i mips

echo ""
echo "=== Stripped library program headers ==="
readelf -l libhello-crosslld-stripped.so

echo ""
echo "=== Testing odump on stripped ==="
/usr/bin/odump -D libhello-crosslld-stripped.so 2>&1 | head -20
echo "odump RC: $?"

echo ""
echo "=== Testing dlopen on stripped library ==="
cat > dltest-crosslld.c << 'EOF'
#include <stdio.h>
#include <dlfcn.h>

int main(void) {
    void *handle = dlopen("/tmp/libhello-crosslld-stripped.so", RTLD_NOW);
    if (!handle) {
        fprintf(stderr, "dlopen failed: %s\n", dlerror());
        return 1;
    }
    printf("dlopen succeeded!\n");

    int (*fn)(void) = dlsym(handle, "hello_get_value");
    if (!fn) {
        fprintf(stderr, "dlsym failed: %s\n", dlerror());
        dlclose(handle);
        return 1;
    }
    printf("dlsym succeeded!\n");

    int value = fn();
    printf("hello_get_value() returned: %d\n", value);
    dlclose(handle);

    if (value == 42) {
        printf("SUCCESS!\n");
        return 0;
    }
    return 1;
}
EOF

gcc -o dltest-crosslld dltest-crosslld.c -ldl
echo "gcc RC: $?"
./dltest-crosslld
echo "dltest-crosslld RC: $?"
