#include "counter.h"

void counter_init(counter_t* c, int value) {
    c->value = value;
    lock_init(&c->lock);
}

int counter_get_value(counter_t* c) {
#if defined(LOCK_RWLOCK)
    rwlock_rdlock(&c->lock);
#else
    lock_acquire(&c->lock);
#endif
    int ret = c->value;
    lock_release(&c->lock);

    return ret;
}

void counter_increment(counter_t* c) {
#if defined(LOCK_RWLOCK)
    rwlock_wrlock(&c->lock);
#else
    lock_acquire(&c->lock);
#endif
    c->value++;
    lock_release(&c->lock);
}

void counter_decrement(counter_t* c) {
#if defined(LOCK_RWLOCK)
    rwlock_wrlock(&c->lock);
#else
    lock_acquire(&c->lock);
#endif
    c->value--;
    lock_release(&c->lock);
}
