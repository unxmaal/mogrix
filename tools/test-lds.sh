#!/bin/ksh
# Test linker-script-built shared library

cd /tmp

cat > dltest-lds.c << 'EOF'
#include <stdio.h>
#include <dlfcn.h>

int main(void) {
    void *handle = dlopen("/tmp/libhello-lds.so", RTLD_NOW);
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
        printf("SUCCESS! Cross-compiled shared library works!\n");
        return 0;
    }
    return 1;
}
EOF

gcc -o dltest-lds dltest-lds.c -ldl
RC=$?
echo "gcc RC: $RC"
if [ $RC -eq 0 ]; then
    ./dltest-lds
    echo "dltest-lds RC: $?"
fi
