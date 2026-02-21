/* C15 Deep Transitive Dependency Chain Test - Library 3 (calls lib2) */

extern int deep_2(void);

int deep_3(void) {
    return deep_2() + 1;
}
