/*
 * C12: dlopen basic test - plugin library
 */
int plugin_func(int x) {
    return x * x;
}

const char *plugin_name(void) {
    return "c12_plugin";
}
