#include "counter.h"

void counter_init(counter_t* c, int value) {
    c->value = value;
    lock_init(&c->lock);
}

int counter_get_value(counter_t* c) {
#if defined(LOCK_RWLOCK)
    reader_lock(&c->lock);
#else
    lock_acquire(&c->lock);
#endif

    int ret = c->value;

#if defined(LOCK_RWLOCK)
    reader_unlock(&c->lock);
#else
    lock_release(&c->lock);
#endif

    return ret;
}

void counter_increment(counter_t* c) {
#if defined(LOCK_RWLOCK)
    writer_lock(&c->lock);
#else
    lock_acquire(&c->lock);
#endif

    c->value++;

#if defined(LOCK_RWLOCK)
    writer_unlock(&c->lock);
#else
    lock_release(&c->lock);
#endif
}

void counter_decrement(counter_t* c) {
#if defined(LOCK_RWLOCK)
    writer_lock(&c->lock);
#else
    lock_acquire(&c->lock);
#endif

    c->value--;

#if defined(LOCK_RWLOCK)
    writer_unlock(&c->lock);
#else
    lock_release(&c->lock);
#endif
}
