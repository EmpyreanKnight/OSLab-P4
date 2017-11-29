#ifndef P4_LOCK_H
#define P4_LOCK_H

#include <pthread.h>

/* The following 4 lock types are exclusive
 * select one of them and comment others to enable specific lock implement
 * note that rwlock will not implement lock_acquire and lock_release
 * rwlock also rewrite implements of counter and list
 */
//#define LOCK_MUTEX
//#define LOCK_SPIN
//#define LOCK_TWOPHASE
//#define LOCK_RWLOCK
//#define LOCK_PTHREAD
#define LOCK_PRWLOCK

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

#if defined(LOCK_PRWLOCK)
typedef pthread_rwlock_t rwlock_t;
#else
typedef struct {
    mutex_t lock;
    cond_t read, write;
    unsigned readers, writers;
    unsigned read_waiters;
    unsigned write_waiters;
} rwlock_t;
#endif

#if defined(LOCK_MUTEX)
typedef mutex_t lock_t;
#elif defined(LOCK_SPIN)
typedef spinlock_t lock_t;
#elif defined(LOCK_TWOPHASE)
typedef twophase_t lock_t;
#elif defined(LOCK_RWLOCK)
typedef rwlock_t lock_t;
#elif defined(LOCK_PRWLOCK)
typedef pthread_rwlock_t lock_t;
#endif

void lock_init(lock_t *lock);
void lock_acquire(lock_t *lock);
void lock_release(lock_t *lock);

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
