// IRIX replacement for cxa_thread_atexit.cpp
// IRIX lacks ELF TLS (__tls_get_addr), so we can't use __thread.
// This uses pthread_key_t for per-thread destructor lists instead.
//
// Limitation: thread_local destructors for the main thread run via atexit(),
// which is later than the standard requires. Acceptable for IRIX.

#include "cxxabi.h"
#include <stdlib.h>
#include <pthread.h>

namespace __cxxabiv1 {

using Dtor = void(*)(void*);

namespace {

struct DtorList {
    Dtor dtor;
    void* obj;
    DtorList* next;
};

static pthread_key_t dtors_key;
static pthread_once_t dtors_once = PTHREAD_ONCE_INIT;

static void run_dtors(void* arg) {
    DtorList* head = static_cast<DtorList*>(arg);
    while (head) {
        DtorList* next = head->next;
        head->dtor(head->obj);
        ::free(head);
        head = next;
    }
}

static void init_key() {
    pthread_key_create(&dtors_key, run_dtors);
}

} // namespace

extern "C" {

_LIBCXXABI_FUNC_VIS int __cxa_thread_atexit(Dtor dtor, void* obj, void* /*dso_symbol*/) throw() {
    pthread_once(&dtors_once, init_key);

    auto* head = static_cast<DtorList*>(::malloc(sizeof(DtorList)));
    if (!head)
        return -1;

    // Prepend to this thread's destructor list
    DtorList* old = static_cast<DtorList*>(pthread_getspecific(dtors_key));
    head->dtor = dtor;
    head->obj = obj;
    head->next = old;
    pthread_setspecific(dtors_key, head);

    return 0;
}

} // extern "C"
} // namespace __cxxabiv1
