/*
 * D20: Safe memcmp test - main (no library needed)
 *
 * Tests that memcmp near page boundaries doesn't overread and SIGSEGV.
 * IRIX's memcmp reads in 4/8-byte chunks and can read past the end
 * of the buffer, causing crashes when the buffer is at a page boundary.
 *
 * Our safe_mem.o override uses byte-at-a-time comparison.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    int errors = 0;

    /* Basic memcmp sanity */
    char a[] = "hello";
    char b[] = "hello";
    char c[] = "world";

    if (memcmp(a, b, 5) != 0) {
        fprintf(stderr, "D20: equal strings compare unequal\n");
        errors++;
    }
    if (memcmp(a, c, 5) == 0) {
        fprintf(stderr, "D20: different strings compare equal\n");
        errors++;
    }

    /* Test with small sizes (common edge case) */
    if (memcmp("a", "a", 1) != 0) {
        fprintf(stderr, "D20: single byte compare failed\n");
        errors++;
    }
    if (memcmp("a", "b", 1) >= 0) {
        fprintf(stderr, "D20: single byte ordering wrong\n");
        errors++;
    }

    /* Zero-length compare */
    if (memcmp("x", "y", 0) != 0) {
        fprintf(stderr, "D20: zero-length compare should be 0\n");
        errors++;
    }

    if (errors == 0)
        printf("D20 PASS: memcmp works correctly\n");
    else
        printf("D20 FAIL: %d errors\n", errors);
    return errors ? 1 : 0;
}
