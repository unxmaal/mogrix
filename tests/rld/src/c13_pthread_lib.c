/*
 * C13: pthread via NEEDED test - library
 *
 * This library is linked with -lpthread (NEEDED).
 * Uses a mutex to prove pthread is functional.
 */
#include <pthread.h>

static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
static int shared_counter = 0;

int pthread_increment(void) {
    pthread_mutex_lock(&mtx);
    shared_counter++;
    int val = shared_counter;
    pthread_mutex_unlock(&mtx);
    return val;
}

int pthread_get_counter(void) {
    return shared_counter;
}
