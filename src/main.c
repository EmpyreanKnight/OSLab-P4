#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>

#include "lock.h"
#include "counter.h"
#include "list.h"
#include "hash.h"

struct timeval tvb, tve;
void startTimer() {
    gettimeofday(&tvb, NULL);
}

double endTimer() {
    gettimeofday(&tve, NULL);
    return (tve.tv_sec - tvb.tv_sec) * 1000 + (tve.tv_usec - tvb.tv_usec) / 1000.0;
}

#define THREAD_COUNT 4
#define MAX_N 100000
#define SEED 1234

counter_t counter;
cond_t cond;
spinlock_t spin;
pthread_mutex_t p_mutex;
mutex_t mutex;
list_t list;
int test_var;

void* test(void* args) {
    int i;
    srand(SEED);
    for (i = 0; i < MAX_N; i++) {
        counter_increment(&counter);
    }
}

void* test2(void* args) {
    printf("thread %d to sleep.\n", (int)args);
    twophase_acquire(&mutex);
    cond_wait(&cond, &mutex);
    twophase_release(&mutex);
    test_var = test_var + 1;
    printf("thread %d waked.\n", (int)args);
    return NULL;
}

void* test3(void* args) {
    sleep(2);
    printf("thread %d wake others.\n", (int)args);
    cond_broadcast(&cond);
    printf("thread %d broadcast.\n", (int)args);
    return NULL;
}

#define READ_RATE 80
#define INSERT_RATE 10
#define RANGE 1000

int glob_cnt;
long long glob_sum;

void* test4(void* args) {
    int i;
    srand(SEED);
    for (i = 0; i < MAX_N; i++) {
        int rd = rand() % 100;
        //pthread_mutex_lock(&p_mutex);
        if (rd < READ_RATE) {
            list_lookup(&list, rand() % RANGE);
        } else if (rd < READ_RATE + INSERT_RATE) {
            int value = rand() % RANGE;
            list_insert(&list, value);
            pthread_mutex_lock(&p_mutex);
            glob_cnt++;
            glob_sum+=value;
            pthread_mutex_unlock(&p_mutex);
        } else {
            int value = rand() % RANGE;
            if (list_lookup(&list, value) != NULL) {
                list_delete(&list, value);
                pthread_mutex_lock(&p_mutex);
                glob_cnt--;
                glob_sum -= value;
                pthread_mutex_unlock(&p_mutex);
            }
        }
        //pthread_mutex_unlock(&p_mutex);
    }
    return NULL;
}

void* test5(void* args) {
    int i;
    srand(SEED);
    for (i = 0; i < MAX_N; i++) {
        int rd = rand() % 100;
        if (rd < READ_RATE) {
            counter_get_value(&counter);
        } else if (rd < READ_RATE + INSERT_RATE) {
            counter_increment(&counter);
        } else {
            counter_decrement(&counter);
        }
    }
    return NULL;
}

long long timeTotal[THREAD_COUNT];
int opCouunt[THREAD_COUNT];

void* test6(void* args) {
    int i;
    int id = (int)args;
    timeTotal[id] = opCouunt[id] = 0;
    struct timeval tvb, tve;
    for (i = 0; i < MAX_N; i++) {
        gettimeofday(&tvb, NULL);
        counter_get_value(&counter);
        gettimeofday(&tve, NULL);
        timeTotal[id] += tve.tv_usec - tvb.tv_usec;
        opCouunt[id]++;
    }
    return NULL;
}

// test correctness of counter
void main1() {
    int i;
    counter_init(&counter, 0);
    pthread_t* threads = malloc(sizeof(pthread_t)*THREAD_COUNT);
    startTimer();
    for (i = 0; i < THREAD_COUNT; i++) {
        pthread_create(&threads[i], NULL, test, NULL);
    }
    for (i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }
    printf("%f ms\n", endTimer());
    printf("Counter value: %d\n", counter_get_value(&counter));
}

// test performance of cv
void main2() {
    int i;
    test_var = 0;
    mutex_init(&mutex);
    cond_init(&cond);
    pthread_t* threads = malloc(sizeof(pthread_t)*THREAD_COUNT);
    startTimer();
    printf("create threads.\n");
    for (i = 0; i < THREAD_COUNT - 1; i++) {
        pthread_create(&threads[i], NULL, test2, (void*)i);
    }
    pthread_create(&threads[THREAD_COUNT - 1], NULL, test3, (void*)i);
    for (i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }
    printf("Time: %f\n", endTimer());
    printf("Test var: %d\n", test_var);
}

// test performance of list
void main3() {
    int i;
    list_init(&list);
    pthread_mutex_init(&p_mutex, NULL);
    glob_sum = 0;
    glob_cnt = 0;
    pthread_t* threads = malloc(sizeof(pthread_t)*THREAD_COUNT);
    startTimer();
    for (i = 0; i < THREAD_COUNT; i++) {
        pthread_create(&threads[i], NULL, test4, (void*)i);
    }
    for (i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }
    printf("Time: %f\n", endTimer());
    printf("Node count: %d vs %d\n", list_count(&list), glob_cnt);
    printf("List sum: %lld vs %lld\n", list_sum(&list), glob_sum);
}

// test performance of counter
void main4() {
    int i;
    counter_init(&counter, 0);
    pthread_t* threads = malloc(sizeof(pthread_t)*THREAD_COUNT);
    startTimer();
    for (i = 0; i < THREAD_COUNT; i++) {
        pthread_create(&threads[i], NULL, test5, (void*)i);
    }
    for (i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }
    printf("Time: %f\n", endTimer());
}

// test fairness
void main5() {
    int i;
    counter_init(&counter, 0);
    pthread_t* threads = malloc(sizeof(pthread_t)*THREAD_COUNT);
    for (i = 0; i < THREAD_COUNT; i++) {
        pthread_create(&threads[i], NULL, test6, (void*)i);
    }
    for (i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Average lock acquire time:\n");
    for (i = 0; i < THREAD_COUNT; i++) {
        printf("%f ", 1.0 * timeTotal[i] / opCouunt[i]);
    }
    printf("\n");
}

int main() {
    main5();
    return 0;
}