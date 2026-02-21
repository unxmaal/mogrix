/* C15 Deep Transitive Dependency Chain Test - Library 2 (calls lib1) */

extern int deep_1(void);

int deep_2(void) {
    return deep_1() + 1;
}
