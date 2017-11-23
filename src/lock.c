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