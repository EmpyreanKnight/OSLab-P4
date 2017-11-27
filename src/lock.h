#ifndef P4_LOCK_H
#define P4_LOCK_H

#include "counter.h"

typedef struct {
    unsigned int flag;
} spinlock_t;

typedef struct {
    unsigned int value;
} mutex_t;

typedef struct {
    unsigned int value;
} twophase_t;

typedef struct {
    unsigned value;
    mutex_t mutex;
} cond_t;

typedef struct {
    mutex_t lock;
    cond_t read, write;
    unsigned readers, writers;
    unsigned read_waiters;
    unsigned write_waiters;
} rwlock_t;

void spinlock_init(spinlock_t *lock);
void spinlock_acquire(spinlock_t *lock);
void spinlock_release(spinlock_t *lock);
void mutex_init(mutex_t *lock);
void mutex_acquire(mutex_t *lock);
void mutex_release(mutex_t *lock);

void twophase_init(twophase_t *lock);
void twophase_acquire(twophase_t *lock);
void twophase_release(twophase_t *lock);

void cond_init(cond_t* lock);
void cond_wait(cond_t* lock, mutex_t* mutex);
void cond_signal(cond_t* lock);
void cond_broadcast(cond_t* lock);

void rwlock_init(rwlock_t* self);
void reader_lock(rwlock_t *self);
void reader_unlock(rwlock_t* self);
void writer_lock(rwlock_t* self);
void writer_unlock(rwlock_t* self);

#endif //P4_LOCK_H
