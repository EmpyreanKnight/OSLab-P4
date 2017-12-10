#include "counter.h"

/**
 * Initialization part of counter
 * @param c Pointer to a counter
 * @param value Initial value of the counter
 */
void counter_init(counter_t *c, int value) {
    c->value = value;
    lock_init(&c->lock);
}

/**
 * Get the value of the counter
 * @param c Pointer to a counter
 * @return The current value of the counter
 */
int counter_get_value(counter_t *c) {
#if defined(LOCK_RWLOCK) || defined(LOCK_PRWLOCK)
    rwlock_rdlock(&c->lock);
#else
    lock_acquire(&c->lock);
#endif
    int ret = c->value;
    lock_release(&c->lock);
    return ret;
}

/**
 * Increase the counter by 1
 * @param c Pointer to a counter
 */
void counter_increment(counter_t *c) {
#if defined(LOCK_RWLOCK) || defined(LOCK_PRWLOCK)
    rwlock_wrlock(&c->lock);
#else
    lock_acquire(&c->lock);
#endif
    c->value++;
    lock_release(&c->lock);
}

/**
 * Decrease the counter by 1
 * @param c Pointer to a counter
 */
void counter_decrement(counter_t *c) {
#if defined(LOCK_RWLOCK) || defined(LOCK_PRWLOCK)
    rwlock_wrlock(&c->lock);
#else
    lock_acquire(&c->lock);
#endif
    c->value--;
    lock_release(&c->lock);
}
