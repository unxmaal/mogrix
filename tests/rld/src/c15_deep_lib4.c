/* C15 Deep Transitive Dependency Chain Test - Library 4 (calls lib3) */

extern int deep_3(void);

int deep_4(void) {
    return deep_3() + 1;
}
