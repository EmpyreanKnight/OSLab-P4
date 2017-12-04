#ifndef P4_LOCK_H
#define P4_LOCK_H

#include <pthread.h>

/**
 * The following 6 lock types are exclusive
 * Select one of them and comment others to enable specific implementation for generic lock
 * Note that rwlock will not implement lock_acquire, but rewrite implements of counter and list instead
 * Also note that pthread rwlock rewrite rwlock implement
 */
//#define LOCK_SPIN
//#define LOCK_MUTEX
//#define LOCK_TWOPHASE
#define LOCK_RWLOCK
//#define LOCK_PTHREAD
//#define LOCK_PRWLOCK

/**
 * The following 3 lock type definitions are trivial, just literal meaning
 */
typedef unsigned int spinlock_t;

typedef unsigned int mutex_t;

typedef unsigned int twophase_t;

/**
 * Condition variable is a kind of synchronization means
 * Can cause threads wait on a specific condition
 * Also can wake one or all waiting thread(s) easily
 */
typedef struct {
    unsigned seq;      /**< sequence number used to judge cv status */
    twophase_t *mutex; /**< serialize operations on cv */
} cond_t;

/**
 * Read-write lock is designed to parallel read threads instead of sequential execution
 * This implement use condition variable to wait/wake and requeue sleeping threads
 */
#if defined(LOCK_PRWLOCK)
typedef pthread_rwlock_t rwlock_t;
#elif defined(LOCK_RWLOCK)
typedef struct {
    twophase_t mutex;       /**< serialize operations on rwlock */
    cond_t reader_lock;     /**< the cv for readers */
    cond_t writer_lock;     /**< the cv for writers */
    unsigned readers;       /**< reader counter */
    unsigned writers;       /**< writer counter, should only be 0 or 1 */
    unsigned read_waiters;  /**< counter for waiters on the read end */
    unsigned write_waiters; /**< counter for waiters on the write end */
} rwlock_t;
#else
typedef int rwlock_t;
#endif


/**
 * The generic lock type definition
 * Directly use existing lock types
 */
#if defined(LOCK_MUTEX)
typedef mutex_t lock_t;
#elif defined(LOCK_SPIN)
typedef spinlock_t lock_t;
#elif defined(LOCK_TWOPHASE)
typedef twophase_t lock_t;
#elif defined(LOCK_RWLOCK)
typedef rwlock_t lock_t;
#elif defined(LOCK_PTHREAD)
typedef pthread_mutex_t lock_t;
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

void cond_init(cond_t* cv);
void cond_wait(cond_t* cv, twophase_t* mutex);
void cond_signal(cond_t* cv);
void cond_broadcast(cond_t* cv);

void rwlock_init(rwlock_t* self);
void rwlock_rdlock(rwlock_t *lock);
void rwlock_wrlock(rwlock_t *lock);
void rwlock_unlock(rwlock_t* self);

#endif //P4_LOCK_H
