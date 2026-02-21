/* C15 Deep Transitive Dependency Chain Test - Library 8 (calls lib7) */

extern int deep_7(void);

int deep_8(void) {
    return deep_7() + 1;
}
