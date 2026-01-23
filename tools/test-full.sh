#!/bin/ksh
# Test fully-configured library
cd /tmp

cat > dltest-full.c << 'ENDOFSOURCE'
#include <stdio.h>
#include <dlfcn.h>
int main(void) {
    void *handle = dlopen("/tmp/libhello-full.so", RTLD_NOW);
    if (!handle) { fprintf(stderr, "dlopen failed: %s\n", dlerror()); return 1; }
    printf("dlopen succeeded!\n");
    int (*fn)(void) = dlsym(handle, "hello_get_value");
    if (!fn) { fprintf(stderr, "dlsym failed: %s\n", dlerror()); dlclose(handle); return 1; }
    int val = fn();
    printf("hello_get_value() returned: %d\n", val);
    dlclose(handle);
    if (val == 42) { printf("SUCCESS!\n"); return 0; }
    return 1;
}
ENDOFSOURCE

gcc -o dltest-full dltest-full.c -ldl
echo "gcc RC: $?"
./dltest-full
echo "dltest-full RC: $?"
