/* C15 Deep Transitive Dependency Chain Test - Main executable */

#include <stdio.h>
#include <stdlib.h>

extern int deep_9(void);

int main(void) {
    int result = deep_9();
    if (result == 9) {
        printf("PASS: deep_9() returned %d\n", result);
        return 0;
    } else {
        printf("FAIL: deep_9() returned %d, expected 9\n", result);
        return 1;
    }
}
