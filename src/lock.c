#include <stdlib.h>
#include "lock.h"
#include <sys/syscall.h>
#include <linux/futex.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

// struct timespec wait_time = { 1, 0 };

#define atomic_add(P, V) __sync_add_and_fetch((P), (V))

/**
 * Provide a method for waiting until a certain condition becomes true
 * @param addr1 A pointer to a futex word
 * @param op Options that specify the operation to be performed
 * @param val1 A value whose meaning and purpose depends on op
 * @param timeout A pointer to a timespec structure that specifies a timeout for the operation.
 * For some operations, the least significant four bytes of this argument are instead used as an integer
 * whose meaning is determined by the operation
 * @param addr2 A pointer to a second futex word that is employed by the operation
 * @param val3 A value whose meaning and purpose depends on op
 * @return
 */
static long sys_futex(void *addr1, int op, int val1, struct timespec *timeout, void *addr2, int val3) {
    return syscall(SYS_futex, addr1, op, val1, timeout, addr2, val3);
}

/**
 * Pause the cpu core using assembly code to avoid bad performance on some machine
 */
static inline void cpu_pause(void) {
    //asm volatile("rep; nop" : : : "memory");
    asm volatile("pause\n": : :"memory");
}

/**
 * Calling embedded assembly instruction "xchg"
 * This instruction completes atomic operation of
 * exchanging values from the pointer addr to the newval
 * @param addr A pointer to the mutex value
 * @param newval The new value to be swapped
 * @return The previous value of the pointer addr referenced
 */
static inline unsigned xchg(void *addr, unsigned newval) {
    asm volatile("xchgl %0, %1"
    : "=r" (newval)
    :"m" (*(volatile unsigned *)addr), "0" (newval)
    :"memory");
    return newval;
}

/**
 * Calling embedded assembly instruction "cmpxchg"
 * This instruction completes atomic operation of
 * comparing the value of the pointer addr pointed with oldval,
 * if equal, put newval into the position of the addr pointer and return to oldval
 * return to the value pointed by addr instead
 * @param addr A pointer to the mutex value
 * @param oldval A value to be compared
 * @param newval A value to write into addr
 * @return oldval or *addr
 */
static inline uint cmpxchg(void *addr, unsigned int oldval, unsigned int newval) {
    uint ret;
    asm volatile("lock; cmpxchgl %1, %2"
    : "=a" (ret)
    : "r" (newval), "m" (*(volatile unsigned *)addr), "0" (oldval)
    : "memory");
    return ret;
}

/**
 * Initialize a spin-lock, should be called before use
 * @param lock Pointer to the spin-lock need to be initialized
 */
void spinlock_init(spinlock_t* lock) {
    *lock = 0;
}

/**
 * Try to acquire the given spin-lock
 * If the lock currently not available, this function will keep trying until somebody release the lock
 * @param lock Pointer to the spin-lock want to obtain
 */
void spinlock_acquire(spinlock_t *lock) {
    while (xchg(lock, 1) == 1) {
        cpu_pause(); // spin-wait
    }
}

/**
 * Release the given spin-lock
 * Notice that release a lock not hold by itself will cause unpredictable result
 * @param lock Pointer to the spin-lock want to release
 */
void spinlock_release(spinlock_t *lock) {
    *lock = 0;
}

/**
 * Initialize a mutex, should be called before use
 * @param lock Pointer to the mutex need to be initialized
 */
void mutex_init(mutex_t *lock) {
    *lock = 0;
}

/**
 * Try to acquire the given mutex
 * A failed try will cause the thread sleep until another thread wake it by releasing this mutex
 * @param lock Pointer to the mutex want to obtain
 */
void mutex_acquire(mutex_t *lock) {
    int value = xchg(lock, 1);
    while (value) {
        sys_futex(lock, FUTEX_WAIT_PRIVATE, 1, NULL, NULL, 0);
        value = xchg(lock, 1);
    }
}

/**
 * Release the given mutex and wake up a thread in waiting condition
 * Notice that release a lock not hold by itself will cause unpredictable result
 * @param lock Pointer to the spin-lock want to release
 */
void mutex_release(mutex_t *lock) {
    xchg(lock, 0);
    sys_futex(lock, FUTEX_WAKE_PRIVATE, 1, NULL, NULL, 0);
}

/**
 * Initialize a two-phase lock, should be called before use
 * @param lock Pointer to the lock need to be initialized
 */
void twophase_init(twophase_t *lock) {
    *lock = 0;
}

/**
 * Acquire the given two-phase in two phases
 * Wait for some one to release first, the wait time is designated by the LOOP_MAX macro (defined below)
 * If the lock still acquired by someone else after wait phase, it will sleep until someone release the lock
 * lock = 0 means unlock state;
 * lock = 1 means the lock has been acquired and has no thread blocking;
 * lock = 2 means the lock has been acquired and at least one thread is sleeping;
 * @param lock Pointer to the two-phase lock want to obtain
 */
#define LOOP_MAX 100000
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

/**
 * Release the given two-phase lock in two phases
 * In the first phase it will try to give the lock to someone awake
 * If failed, it will wake up a sleeping thread on this lock
 * @param lock Pointer to the two-phase lock want to release
 */
void twophase_release(twophase_t *lock) {
    int i;

    if (*lock == 2) {
        *lock = 0;
    } else if (xchg(lock, 0) == 1) {
        return;
    }

    for (i = 0; i < LOOP_MAX; i++) {
        if (*lock) {
            if (cmpxchg(lock, 1, 2)) {
                return;
            }
        }
        cpu_pause();
    }

    sys_futex(lock, FUTEX_WAKE_PRIVATE, 1, NULL, NULL, 0);
}

/**
 * Initialize a condition variable, should be called before use
 * @param cv Pointer to the condition variable need to be initialized
 */
void cond_init(cond_t* cv) {
    cv->seq = 0;
    cv->mutex = NULL;
}

/**
 * Wait a thread on the given condition variable
 * The caller will sleep on the given cv, its mutex will be released
 * Note that the mutex should be two-phase type since cv need extra information in mutex
 * @param cv    Pointer the the condition variable to wait on
 * @param mutex The mutex owned by caller, will be released
 */
void cond_wait(cond_t* cv, twophase_t* mutex) {
    int old_seq = cv->seq;

    if (cv->mutex != mutex) {
        if (cv->mutex != NULL) {
            perror("cond mutex already exists!");
            return;
        }
        cmpxchg(&cv->mutex, (unsigned)(unsigned long)NULL, (unsigned)(unsigned long)mutex);
        if (cv->mutex != mutex) {
            perror("cond mutex incompatible!");
            return;
        }
    }

    twophase_release(mutex);	// release the mutex

    sys_futex(&cv->seq, FUTEX_WAIT_PRIVATE, old_seq, NULL, NULL, 0);	// wait the current thread on seq

    while (xchg(mutex, 2)) {	// get the mutex or wait
        sys_futex(mutex, FUTEX_WAIT_PRIVATE, 2, NULL, NULL, 0);
    }
}

/**
 * Wake up a thread waiting on the given condition variable
 * @param cv The condition variable to receive a signal
 */
void cond_signal(cond_t* cv) {
    atomic_add(&cv->seq, 1);
    sys_futex(&cv->seq, FUTEX_WAKE_PRIVATE, 1, NULL, NULL, 0);
}

/**
 * Wake up ALL threads waiting on the given condition variable
 * @param cv The condition variable to receive a broadcast
 */
void cond_broadcast(cond_t* cv) {
    mutex_t* old_mutex = cv->mutex;
    if (old_mutex == NULL) {
        return;
    }
    atomic_add(&cv->seq, 1);

    // wake up 1 waiting thread, requeue other threads on mutex to avoid thundering herd effect
    sys_futex(&cv->seq, FUTEX_REQUEUE_PRIVATE, 1, (void*) 0x0FFFFFFF, old_mutex, 0);	// *((int*) 0x0FFFFFFF)
}

/**
 * Initialize a read-write lock, should be called before use
 * @param lock Pointer to the read-write lock need to be initialized
 */
void rwlock_init(rwlock_t* lock) {
#if defined(LOCK_PRWLOCK)
    pthread_rwlock_init(lock, NULL);
#else //if defined(LOCK_RWLOCK)
    lock->readers = 0;
    lock->writers = 0;
    lock->read_waiters = 0;
    lock->write_waiters = 0;
    twophase_init(&lock->mutex);
    cond_init(&lock->reader_lock);
    cond_init(&lock->writer_lock);
#endif
}

/**
 * Acquire the read-end of the given read-write lock
 * The calling thread acquires the read lock if no writer hold the lock
 * and there are no writers in blocked on the lock.
 * @param lock Pointer to the read-write lock to be acquired
 */
void rwlock_rdlock(rwlock_t *lock) {
#if defined(LOCK_PRWLOCK)
    pthread_rwlock_rdlock(lock);
#else //if defined(LOCK_RWLOCK)
    twophase_acquire(&lock->mutex);
    while (lock->writers || lock->write_waiters) {
        lock->read_waiters++;
        cond_wait(&lock->reader_lock, &lock->mutex);
        lock->read_waiters--;
    }
    lock->readers++;
    twophase_release(&lock->mutex);
#endif
}

/**
 * Acquire the write-end of the given read-write lock
 * The calling thread acquires the write lock if no other thread holds the rwlock
 * Otherwise, the thread shall block until it can acquire the rwlock
 * @param lock Pointer to the read-write lock to be acquired
 */
void rwlock_wrlock(rwlock_t *lock) {
#if defined(LOCK_PRWLOCK)
    pthread_rwlock_wrlock(lock);
#else //if defined(LOCK_RWLOCK)
    twophase_acquire(&lock->mutex);
    while (lock->readers || lock->writers) {
        lock->write_waiters++;
        cond_wait(&lock->writer_lock, &lock->mutex);
        lock->write_waiters--;
    }
    lock->writers = 1;
    twophase_release(&lock->mutex);
#endif
}

/**
 * Release the read-write lock, for both read and write end according to POSIX standard
 * If this function is called by a read lock:
 * firstly, if there are other writers blocked currently, one write thread(s) shall acquire the lock.
 * otherwise, the rwlock shall turn into unlocked state.
 * If this function is called by a write lock:
 * firstly, if there are other writers blocked currently, one write thread shall acquire the lock.
 * secondly, if there are other readers blocked currently, all of the reader threads shall be awakened.
 * Note this implement slightly favour the writers
 * @param lock Pointer to the read-write to be released
 */
void rwlock_unlock(rwlock_t* lock) {
#if defined(LOCK_PRWLOCK)
    pthread_rwlock_unlock(lock);
#else //if defined(LOCK_RWLOCK)
    twophase_acquire(&lock->mutex);
    if (lock->readers) {
        lock->readers--;
        if (lock->write_waiters) {
            cond_signal(&lock->writer_lock);
        }
        // reader thread must be awakened by writer thread?
    } else if (lock->writers) {
        lock->writers = 0;
        if (lock->write_waiters) {
            cond_signal(&lock->writer_lock);
        } else if (lock->read_waiters) {
            cond_broadcast(&lock->reader_lock);
        }
    }
    twophase_release(&lock->mutex);
#endif
}

/**
 * The generic lock initialization method, not use macro for the debug consideration
 * The lock type is designated by the macros defined in header file
 * @param lock A pointer to the lock to be initialized
 */
void lock_init(lock_t* lock) {
#if defined(LOCK_MUTEX)
    mutex_init(lock);
#elif defined(LOCK_SPIN)
    spinlock_init(lock);
#elif defined(LOCK_TWOPHASE)
    twophase_init(lock);
#elif defined(LOCK_PTHREAD)
    pthread_mutex_init(lock, NULL);
#elif defined(LOCK_RWLOCK) || defined(LOCK_PRWLOCK)
    rwlock_init(lock);
#endif
}

/**
 * The generic lock acquire method
 * Note that read-write locks DO NOT call this function
 * @param lock A pointer to the lock to be acquired
 */
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

/**
 * The generic lock release method
 * @param lock A pointer to the lock to be released
 */
void lock_release(lock_t* lock) {
#if defined(LOCK_MUTEX)
    mutex_release(lock);
#elif defined(LOCK_SPIN)
    spinlock_release(lock);
#elif defined(LOCK_TWOPHASE)
    twophase_release(lock);
#elif defined(LOCK_PTHREAD)
    pthread_mutex_unlock(lock);
#elif defined(LOCK_RWLOCK) || defined(LOCK_PRWLOCK)
    rwlock_unlock(lock);
#endif
}
