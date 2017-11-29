#include <stdlib.h>
#include "lock.h"
#include <sys/syscall.h>
#include <linux/futex.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

// struct timespec wait_time = { 1, 0 };

static long sys_futex(void *addr1, int op, int val1, struct timespec *timeout, void *addr2, int val3) {
    return syscall(SYS_futex, addr1, op, val1, timeout, addr2, val3);
}

static inline void cpu_pause(void) {
    asm volatile("rep; nop" : : : "memory");
    //asm volatile("pause\n": : :"memory");
}

/*
static inline uint xchg(volatile unsigned int *addr, unsigned int newval) {
    uint result;
    asm volatile("lock; xchgl %0, %1"
    : "+m" (*addr), "=a" (result)
    : "1" (newval)
    : "cc");
    return result;
}*/

static inline unsigned xchg(void *addr, unsigned newval) {
    asm volatile("xchgl %0, %1"
    : "=r" (newval)
    :"m" (*(volatile unsigned *)addr), "0" (newval)
    :"memory");
    return newval;
}

static inline uint cmpxchg(volatile unsigned int *addr, unsigned int oldval, unsigned int newval) {
    uint ret;
    asm volatile("lock; cmpxchgl %1, %2"
    : "=a" (ret)
    : "r" (newval), "m" (*addr), "0" (oldval)
    : "memory");
    return ret;
}

void spinlock_init(spinlock_t* lock) {
    lock->flag = 0;
}

void spinlock_acquire(spinlock_t *lock) {
    while (xchg(&lock->flag, 1) == 1) {
        cpu_pause(); // spin-wait
    }
}

void spinlock_release(spinlock_t *lock) {
    lock->flag = 0;
}

void mutex_init(mutex_t *lock) {
    *lock = 0;
}

void mutex_acquire(mutex_t *lock) {
    int value = xchg(lock, 1);
    while (value) {
        //printf("%d lock rd: %d\n", pthread_self()%10000, value);
        sys_futex(lock, FUTEX_WAIT_PRIVATE, 1, NULL, NULL, 0);
        value = xchg(lock, 1);
    }
    //printf("%d locked\n", pthread_self()%10000);
}

void mutex_release(mutex_t *lock) {
    while (xchg(lock, 0)) ;
    //printf("%d unlock : %d\n", pthread_self()%10000, *lock);
    sys_futex(lock, FUTEX_WAKE_PRIVATE, 1, NULL, NULL, 0);
}

void twophase_init(twophase_t *lock) {
    *lock = 0;
}

#define LOOP_MAX 1000
void twophase_acquire(twophase_t *lock) {
    int i, value = 1;
    for (i = 0; i < LOOP_MAX; i++) {
        value = cmpxchg(lock, 0, 1);
        if (value == 0) {
            return;
        }
        cpu_pause();
    }
    if (value == 1) {
        value = xchg(lock, 2);
    }

    while (value) {
        sys_futex(lock, FUTEX_WAIT_PRIVATE, 2, NULL, NULL, 0);
        value = xchg(lock, 2);
    }
}

void twophase_release(twophase_t *lock) {
    int i;

    if (*lock == 2) {
        *lock = 0;
    } else if (xchg(lock, 0) == 1) {
        return;
    }

    for (i = 0; i < LOOP_MAX; i++) {
        if (*lock && cmpxchg(lock, 1, 2)) {
            return;
        }
    }

    sys_futex(lock, FUTEX_WAKE_PRIVATE, 1, NULL, NULL, 0);
}


void cond_init(cond_t* lock) {
    lock->seq = 0;
    lock->mutex = NULL;
}

void cond_wait(cond_t* lock, mutex_t* mutex) {
    int old_seq = lock->seq;

    if (lock->mutex != mutex) {
        if (lock->mutex != NULL) { // TODO: ERROR
            return;
        }
        cmpxchg(&lock->mutex, NULL, mutex);
        if (lock->mutex != mutex) { // TODO: ERROR
            return;
        }
    }

    mutex_release(mutex);

    sys_futex(&lock->seq, FUTEX_WAIT, old_seq, NULL, NULL, 0);

    while (xchg(mutex, 2)) {
        sys_futex(mutex, FUTEX_WAIT, 2, NULL, NULL, 0);
    }
}

void cond_signal(cond_t* lock) {
    lock->seq++;
    sys_futex(&lock->seq, FUTEX_WAKE, 1, NULL, NULL, 0);
}

void cond_broadcast(cond_t* lock) {
    mutex_t* old_mutex = lock->mutex;
    if (old_mutex == NULL) {
        return;
    }
    lock->seq++;
    sys_futex(&lock->seq, FUTEX_REQUEUE, 1, (void*)2147483647, old_mutex, 0);
}

void rwlock_init(rwlock_t* lock) {
#if defined(LOCK_PRWLOCK)
    pthread_rwlock_init(lock, NULL);
#elif defined(LOCK_RWLOCK)
    lock->readers = 0;
    lock->writers = 0;
    lock->read_waiters = 0;
    lock->write_waiters = 0;
    mutex_init(&lock->mutex);

    lock->writer_preferred = 1; // enable this to prefer writer
#elif defined(LOCK_RWLOCK2)
    lock->readers = 0;
    lock->writers = 0;
    lock->read_waiters = 0;
    lock->write_waiters = 0;
    mutex_init(&lock->mutex);
    cond_init(&lock->reader_lock);
    cond_init(&lock->writer_lock);
#endif
}

void rwlock_rdlock(rwlock_t *lock) {
#if defined(LOCK_PRWLOCK)
    pthread_rwlock_rdlock(lock);
#elif defined(LOCK_RWLOCK)
    mutex_acquire(&lock->mutex);
    while (lock->writers || (lock->write_waiters && lock->writer_preferred)) {
        lock->read_waiters++;
        mutex_release(&lock->mutex);
        sys_futex(&lock->read_waiters, FUTEX_WAIT_PRIVATE, 1, NULL, NULL, 0);
        mutex_acquire(&lock->mutex);
        lock->read_waiters--;
    }
    lock->readers++;
    mutex_release(&lock->mutex);
#elif defined(LOCK_RWLOCK2)
    mutex_acquire(&lock->mutex);
    while (lock->writers || lock->write_waiters) {
        lock->read_waiters++;
        cond_wait(&lock->reader_lock, &lock->mutex);
        lock->read_waiters--;
    }
    lock->readers++;
    mutex_release(&lock->mutex);
#endif
}

void rwlock_wrlock(rwlock_t *lock) {
#if defined(LOCK_PRWLOCK)
    pthread_rwlock_wrlock(lock);
#elif defined(LOCK_RWLOCK)
    mutex_acquire(&lock->mutex);
    while (lock->readers || lock->writers) {
        lock->write_waiters++;
        mutex_release(&lock->mutex);
        sys_futex(&lock->write_waiters, FUTEX_WAIT_PRIVATE, 1, NULL, NULL, 0);
        mutex_acquire(&lock->mutex);
        lock->write_waiters--;
    }
    lock->writers = 1;
    mutex_release(&lock->mutex);
#elif defined(LOCK_RWLOCK2)
    mutex_acquire(&lock->mutex);
    while (lock->readers || lock->writers) {
        lock->write_waiters++;
        cond_wait(&lock->writer_lock, &lock->mutex);
        lock->write_waiters--;
    }
    lock->writers = 1;
    mutex_release(&lock->mutex);
#endif
}

void rwlock_unlock(rwlock_t* lock) {
#if defined(LOCK_PRWLOCK)
    pthread_rwlock_unlock(lock);
#elif defined(LOCK_RWLOCK)
    mutex_acquire(&lock->mutex);
    if (lock->readers) { // if there're readers
        lock->readers--;
        if (lock->readers) { // if still have other readers
            mutex_release(&lock->readers);
            return;
        }
    } else {
        lock->writers = 0;
    }
    mutex_release(&lock->mutex);

    if (lock->write_waiters) { // wake a writer first
        sys_futex(&lock->write_waiters, FUTEX_WAKE_PRIVATE, 1, NULL, NULL, 0);
    } else if (lock->read_waiters) { // wake all readers
        sys_futex(&lock->read_waiters, FUTEX_WAKE_PRIVATE, 2147483647, NULL, NULL, 0);
    }

#elif defined(LOCK_RWLOCK2)
    mutex_acquire(&lock->mutex);
    if (lock->readers) { // if there're readers
        lock->readers--;
        if (lock->readers) { // if still have other readers
            mutex_release(&lock->readers);
            return;
        }
    } else {
        lock->writers = 0;
    }
    mutex_release(&lock->mutex);

    if (lock->write_waiters) { // wake a writer first
        cond_signal(&lock->writer_lock);
    } else if (lock->read_waiters) { // wake all readers
        cond_broadcast(&lock->reader_lock);
    }
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
    pthread_mutex_init(lock, NULL);
#elif defined(LOCK_RWLOCK)
    rwlock_init(lock);
#endif
}

void lock_acquire(lock_t* lock) {
#if defined(LOCK_MUTEX)
    mutex_acquire(lock);
#elif defined(LOCK_SPIN)
    spinlock_acquire(lock);
#elif defined(LOCK_TWOPHASE)
    twophase_acquire(lock);
#elif defined(LOCK_PTHREAD)
    pthread_mutex_lock(lock);
#endif
}

void lock_release(lock_t* lock) {
#if defined(LOCK_MUTEX)
    mutex_release(lock);
#elif defined(LOCK_SPIN)
    spinlock_release(lock);
#elif defined(LOCK_TWOPHASE)
    twophase_release(lock);
#elif defined(LOCK_PTHREAD)
    pthread_mutex_unlock(lock);
#elif defined(LOCK_RWLOCK)
    rwlock_unlock(lock);
#endif
}
