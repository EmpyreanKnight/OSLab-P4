#include <stdlib.h>
#include "lock.h"
#include <sys/syscall.h>
#include <time.h>
#include <linux/futex.h>
#include <unistd.h>

static long sys_futex(void *addr1, int op, int val1, struct timespec *timeout, void *addr2, int val3) {
    return syscall(SYS_futex, addr1, op, val1, timeout, addr2, val3);
}

static inline uint xchg(volatile unsigned int *addr, unsigned int newval) {
    uint result;
    asm volatile("lock; xchgl %0, %1" : "+m" (*addr), "=a" (result) : "1" (newval) : "cc");
    return result;
}

void spinlock_init(spinlock_t* lock) {
    lock->flag = 0;
}

void spinlock_acquire(spinlock_t *lock) {
    while (xchg(&lock->flag, 1) == 1) {
        // spin-wait
    }
}

void spinlock_release(spinlock_t *lock) {
    lock->flag = 0;
}

void mutex_init(mutex_t *lock) {
    lock->value = 0;
}

void mutex_acquire(mutex_t *lock) {
    while (xchg(&lock->value, 1) == 1) {
        sys_futex(&lock->value, FUTEX_WAIT, 1, NULL, NULL, 0);
    }
}

void mutex_release(mutex_t *lock) {
    lock->value = 0;
    sys_futex(&lock->value, FUTEX_WAKE, 1, NULL, NULL, 0);
}

void twophase_init(twophase_t *lock) {}

void twophase_acquire(twophase_t *lock) {}

void twophase_release(twophase_t *lock) {}


void cond_init(cond_t* lock) {
    lock->value = 0;
    mutex_init(&lock->mutex);
}

void cond_wait(cond_t* lock, mutex_t* mutex) {
    int old_value = lock->value;
    mutex_release(mutex);
    sys_futex(&lock->value, FUTEX_WAIT, old_value, NULL, NULL, 0);
    mutex_acquire(mutex);
}

void cond_signal(cond_t* lock) {
    mutex_acquire(&lock->mutex);
    lock->value++;
    sys_futex(&lock->value, FUTEX_WAKE, 1, NULL, NULL, 0);
    mutex_release(&lock->mutex);
}

void cond_broadcast(cond_t* lock) {
    mutex_acquire(&lock->mutex);
    lock->value++;
    sys_futex(&lock->value, FUTEX_WAKE, 2147483647, NULL, NULL, 0);
    mutex_release(&lock->mutex);
}

void rwlock_init(rwlock_t* self) {
#if defined(LOCK_PRWLOCK)
    pthread_rwlock_init(self, NULL);
#else
    self->readers = self->writers = 0;
    self->read_waiters = 0;
    self->write_waiters = 0;
    mutex_init(&self->lock);
    cond_init(&self->read);
    cond_init(&self->write);
#endif
}

void reader_lock(rwlock_t *self) {
#if defined(LOCK_PRWLOCK)
    pthread_rwlock_rdlock(self);
#else
    mutex_acquire(&self->lock);
    if (self->writers || self->write_waiters) {
        self->read_waiters++;
        do {
            cond_wait(&self->read, &self->lock);
        } while (self->writers || self->write_waiters);
        self->read_waiters--;
    }
    self->readers++;
    mutex_release(&self->lock);
#endif
}

void reader_unlock(rwlock_t* self) {
#if defined(LOCK_PRWLOCK)
    pthread_rwlock_unlock(self);
#else
    mutex_acquire(&self->lock);
    self->readers--;
    if (self->write_waiters) {
        cond_signal(&self->write);
    }
    mutex_release(&self->lock);
#endif
}

void writer_lock(rwlock_t* self) {
#if defined(LOCK_PRWLOCK)
    pthread_rwlock_wrlock(self);
#else
    mutex_acquire(&self->lock);
    if (self->readers || self->writers) {
        self->write_waiters++;
        do {
            cond_wait(&self->write, &self->lock);
        } while (self->readers || self->writers);
        self->write_waiters--;
    }
    self->writers = 1;
    mutex_release(&self->lock);
#endif
}

void writer_unlock(rwlock_t* self) {
#if defined(LOCK_PRWLOCK)
    pthread_rwlock_unlock(self);
#else
    mutex_acquire(&self->lock);
    self->writers = 0;
    if (self->write_waiters) {
        cond_signal(&self->write);
    } else if (self->read_waiters) {
        cond_broadcast(&self->read);
    }
    mutex_release(&self->lock);
#endif
}

void lock_init(lock_t* lock) {
#if defined(LOCK_MUTEX)
    mutex_init(lock);
#elif defined(LOCK_SPIN)
    spinlock_init(lock);
#elif defined(LOCK_TWOPHASE)
    twophase_init(lock);
#elif defined(LOCK_PTHREAD)
    pthread_mutex_lock(lock);
#elif defined(LOCK_RWLOCK)
    rwlock_init(lock);
#elif defined(LOCK_PRWLOCK)
    pthread_rwlock_init(lock, NULL);
#endif
}

void lock_acquire(lock_t* lock) {
#if defined(LOCK_MUTEX)
    mutex_acquire(lock);
#elif defined(LOCK_SPIN)
    spinlock_acquire(lock);
#elif defined(LOCK_TWOPHASE)
    twophase_acquire(lock);
#endif
}

void lock_release(lock_t* lock) {
#if defined(LOCK_MUTEX)
    mutex_release(lock);
#elif defined(LOCK_SPIN)
    spinlock_release(lock);
#elif defined(LOCK_TWOPHASE)
    twophase_release(lock);
#endif
}
