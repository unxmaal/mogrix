#!/bin/ksh
# Test if libhello-stripped.so can be loaded with dlopen

cd /tmp

# Create a simple dlopen test program
cat > dltest.c << 'EOF'
#include <stdio.h>
#include <dlfcn.h>

int main(void) {
    void *handle;
    int (*hello_get_value)(void);
    char *error;

    printf("Attempting to load libhello-stripped.so...\n");

    handle = dlopen("/tmp/libhello-stripped.so", RTLD_NOW);
    if (!handle) {
        fprintf(stderr, "dlopen failed: %s\n", dlerror());
        return 1;
    }
    printf("dlopen succeeded!\n");

    hello_get_value = dlsym(handle, "hello_get_value");
    if ((error = dlerror()) != NULL) {
        fprintf(stderr, "dlsym failed: %s\n", error);
        dlclose(handle);
        return 1;
    }
    printf("dlsym succeeded!\n");

    int value = hello_get_value();
    printf("hello_get_value() returned: %d\n", value);

    dlclose(handle);

    if (value == 42) {
        printf("SUCCESS!\n");
        return 0;
    } else {
        printf("FAILED: expected 42, got %d\n", value);
        return 1;
    }
}
EOF

echo "=== Compiling dlopen test with SGUG GCC ==="
gcc -o dltest dltest.c -ldl
RC=$?
echo "gcc RC: $RC"

if [ $RC -eq 0 ]; then
    ls -la dltest
    echo ""
    echo "=== Running dlopen test ==="
    ./dltest
    echo "dltest RC: $?"
fi
