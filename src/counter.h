#ifndef P4_COUNTER_H
#define P4_COUNTER_H

#include "lock.h"
#define INT_MAX 2147483647
#define INT_MIN (-INT_MAX - 1)
/**
 * A concurrent counter type
 */
typedef struct {
    int value;    /**< internal counter variable */
    lock_t lock;  /**< lock for critical section */
} counter_t;

void counter_init(counter_t *c, int value);
int counter_get_value(counter_t *c);
void counter_increment(counter_t *c);
void counter_decrement(counter_t *c);

#endif //P4_COUNTER_H
