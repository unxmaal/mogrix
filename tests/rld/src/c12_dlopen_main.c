/*
 * C12: dlopen basic test - main
 *
 * Tests basic dlopen/dlsym/dlclose cycle.
 */
#include <stdio.h>
#include <dlfcn.h>

int main(void) {
    void *h = dlopen("libc12_plugin.so", RTLD_LAZY);
    if (!h) {
        fprintf(stderr, "C12 FAIL: dlopen: %s\n", dlerror());
        return 1;
    }

    int (*func)(int) = (int (*)(int))dlsym(h, "plugin_func");
    if (!func) {
        fprintf(stderr, "C12 FAIL: dlsym(plugin_func): %s\n", dlerror());
        dlclose(h);
        return 1;
    }

    const char *(*name)(void) = (const char *(*)(void))dlsym(h, "plugin_name");
    if (!name) {
        fprintf(stderr, "C12 FAIL: dlsym(plugin_name): %s\n", dlerror());
        dlclose(h);
        return 1;
    }

    int result = func(7);
    if (result != 49) {
        fprintf(stderr, "C12 FAIL: plugin_func(7)=%d (want 49)\n", result);
        dlclose(h);
        return 1;
    }

    printf("C12 PASS: dlopen/dlsym/dlclose works (plugin=%s, 7^2=%d)\n", name(), result);
    dlclose(h);
    return 0;
}
