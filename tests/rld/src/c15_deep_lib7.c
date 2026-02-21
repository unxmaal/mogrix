/* C15 Deep Transitive Dependency Chain Test - Library 7 (calls lib6) */

extern int deep_6(void);

int deep_7(void) {
    return deep_6() + 1;
}
