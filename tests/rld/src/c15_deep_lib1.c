/* C15 Deep Transitive Dependency Chain Test - Library 1 (calls lib0) */

extern int deep_0(void);

int deep_1(void) {
    return deep_0() + 1;
}
