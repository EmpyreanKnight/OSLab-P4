#ifndef P4_LOCK_H
#define P4_LOCK_H

typedef struct {
    unsigned int flag;
} spinlock_t;

typedef struct {
    unsigned int value;
} mutex_t;

void spinlock_init(spinlock_t *lock);
void spinlock_acquire(spinlock_t *lock);
void spinlock_release(spinlock_t *lock);
void mutex_init(mutex_t *lock);
void mutex_acquire(mutex_t *lock);
void mutex_release(mutex_t *lock);

#endif //P4_LOCK_H
