#!/bin/ksh
cd /tmp

cat > dltest-gnuld.c << 'ENDOFSRC'
#include <stdio.h>
#include <dlfcn.h>
int main(void) {
    void *h = dlopen("/tmp/libhello-gnuld.so", RTLD_NOW);
    if (!h) { fprintf(stderr, "dlopen: %s\n", dlerror()); return 1; }
    printf("dlopen succeeded!\n");
    int (*fn)(void) = dlsym(h, "hello_get_value");
    if (!fn) { fprintf(stderr, "dlsym: %s\n", dlerror()); dlclose(h); return 1; }
    int val = fn();
    printf("hello_get_value() = %d\n", val);
    dlclose(h);
    return val == 42 ? 0 : 1;
}
ENDOFSRC

gcc -o dltest-gnuld dltest-gnuld.c -ldl
echo "gcc RC: $?"
./dltest-gnuld
echo "test RC: $?"
