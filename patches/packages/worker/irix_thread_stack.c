/*
 * irix_thread_stack.c — Set default pthread stack size for IRIX
 *
 * IRIX default pthread stack is 256KB (vs Linux 2-8MB). std::async and
 * std::thread use the default, which is too small for worker's file
 * operations + libmagic + C++ exception handling.
 *
 * This constructor runs before main() and sets RLIMIT_STACK to 8MB,
 * which IRIX libpthread uses as the default for new threads.
 */
#include <sys/resource.h>

__attribute__((constructor))
static void _irix_set_thread_stack(void) {
    struct rlimit rl;
    if (getrlimit(RLIMIT_STACK, &rl) == 0) {
        if (rl.rlim_cur < 8 * 1024 * 1024) {
            rl.rlim_cur = 8 * 1024 * 1024;
            setrlimit(RLIMIT_STACK, &rl);
        }
    }
}
