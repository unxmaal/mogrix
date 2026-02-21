/*
 * B10: Strict NEEDED test - library B
 * NEEDs liba. Main links against libb which pulls liba transitively.
 */
extern int needed_a_func(void);

int needed_b_func(void) {
    return needed_a_func() + 20;
}
