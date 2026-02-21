/* C15 Deep Transitive Dependency Chain Test - Library 6 (calls lib5) */

extern int deep_5(void);

int deep_6(void) {
    return deep_5() + 1;
}
