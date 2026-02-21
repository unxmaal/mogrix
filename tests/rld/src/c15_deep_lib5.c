/* C15 Deep Transitive Dependency Chain Test - Library 5 (calls lib4) */

extern int deep_4(void);

int deep_5(void) {
    return deep_4() + 1;
}
