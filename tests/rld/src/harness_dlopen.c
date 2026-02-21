/*
 * harness_dlopen.c — Incremental library load tester for IRIX rld.
 *
 * Compiled with: irix-cc -o harness_dlopen harness_dlopen.c -ldl
 * Sonames passed via stdin, one per line.
 *
 * Usage:
 *   echo -e "libfoo.so\nlibbar.so" | ./harness_dlopen /path/to/libs
 *
 * Exit codes:
 *   0 = all libraries loaded successfully
 *   1 = one or more libraries failed to load
 *   2 = usage error
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#define MAX_LIBS 512
#define MAX_PATH 1024

int main(int argc, char *argv[]) {
    const char *libdir = "/usr/sgug/lib32";
    char line[MAX_PATH];
    char path[MAX_PATH];
    int failed = 0, loaded = 0;

    if (argc > 1)
        libdir = argv[1];

    while (fgets(line, sizeof(line), stdin) != NULL) {
        /* Strip newline */
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n')
            line[len-1] = '\0';
        if (line[0] == '\0')
            continue;

        snprintf(path, sizeof(path), "%s/%s", libdir, line);
        fprintf(stderr, "Loading: %s\n", path);
        fflush(stderr);

        void *h = dlopen(path, RTLD_NOW | RTLD_GLOBAL);
        if (h == NULL) {
            fprintf(stderr, "FAIL: %s: %s\n", line, dlerror());
            fflush(stderr);
            failed++;
        } else {
            fprintf(stderr, "OK: %s\n", line);
            fflush(stderr);
            loaded++;
        }
    }

    fprintf(stdout, "loaded=%d failed=%d\n", loaded, failed);
    return failed > 0 ? 1 : 0;
}
