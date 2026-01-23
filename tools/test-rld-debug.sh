#!/bin/ksh
# Test with RLD debug output
cd /tmp

export _RLD_DEBUG=1

cat > dltest-debug.c << 'ENDOFSOURCE'
#include <stdio.h>
#include <dlfcn.h>
int main(void) {
    void *handle = dlopen("/tmp/libhello-full.so", RTLD_NOW);
    if (!handle) { fprintf(stderr, "dlopen failed: %s\n", dlerror()); return 1; }
    printf("dlopen succeeded!\n");
    dlclose(handle);
    return 0;
}
ENDOFSOURCE

gcc -o dltest-debug dltest-debug.c -ldl
./dltest-debug 2>&1 | head -30
