/* C15 Deep Transitive Dependency Chain Test - Library 9 (calls lib8) */

extern int deep_8(void);

int deep_9(void) {
    return deep_8() + 1;
}
