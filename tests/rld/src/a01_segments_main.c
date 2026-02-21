/*
 * A01: 2-segment layout test - main
 */
#include <stdio.h>
extern int segment_func(int x);
extern int segment_data;

int main(void) {
    if (segment_func(segment_data) != 43) {
        fprintf(stderr, "A01 FAIL: segment_func(42) != 43\n");
        return 1;
    }
    printf("A01 PASS: .so loaded with correct segment layout\n");
    return 0;
}
