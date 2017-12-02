#include "counter.h"

extern int user_error;

/**
 *  Initialization part of counter
 * @param c a pointer to a counter
 * @param value Initial value of the counter
 */
void counter_init(counter_t* c, int value) {
    c->value = value;
    lock_init(&c->lock);
}

/**
 *  Get the value of the counter
 * @param c a pointer to a counter
 * @return the value of the counter
 */
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

/**
 *  Make the counter's value increase by 1
 * @param c a pointer to a counter
 */
void counter_increment(counter_t* c) {
#if defined(LOCK_RWLOCK)
    rwlock_wrlock(&c->lock);
#else
    lock_acquire(&c->lock);
#endif
    if (c->value == INT_MAX) {
        user_error = E_DATA_OVERFLOW;
    }else {
        c->value++;
    }
    lock_release(&c->lock);
}

/**
 *  Make the counter's value increase by 1
 * @param c a pointer to a counter
 */
void counter_decrement(counter_t* c) {
#if defined(LOCK_RWLOCK)
    rwlock_wrlock(&c->lock);
#else
    lock_acquire(&c->lock);
#endif
    if (c->value == INT_MIN) {
        user_error = E_DATA_OVERFLOW;
    }else {
        c->value--;
    }
    lock_release(&c->lock);
}
