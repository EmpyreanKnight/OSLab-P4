#ifndef P4_COUNTER_H
#define P4_COUNTER_H

#include "lock.h"

typedef struct {
    int value;
    mutex_t lock;
} counter_t;

void counter_init(counter_t *c, int value);
int counter_get_value(counter_t *c);
void counter_increment(counter_t *c);
void counter_decrement(counter_t *c);

#endif //P4_COUNTER_H
