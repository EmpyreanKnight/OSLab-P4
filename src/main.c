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

#define THREAD_COUNT 8
#define MAX_N 100000

counter_t counter;
cond_t cond;
mutex_t mutex;
list_t list;
int test_var;

void* test(void* args) {
    int i;
    srand((unsigned) time(0));
    for (i = 0; i < MAX_N; i++) {
        counter_increment(&counter);
    }
}

void* test2(void* args) {
    printf("thread %d to sleep.\n", (int)args);
    mutex_acquire(&mutex);
    cond_wait(&cond, &mutex);
    mutex_release(&mutex);
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
void* test4(void* args) {
    int i;
    srand((unsigned) time(0));
    for (i = 0; i < MAX_N; i++) {
        int rd = rand() % 100;
        if (rd < READ_RATE) {
            list_lookup(&list, rand() % RANGE);
        } else if (rd < READ_RATE + INSERT_RATE) {
            list_insert(&list, rand() % RANGE);
        } else {
            list_delete(&list, rand() % RANGE);
        }
    }
}

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

void main2() {
    int i;
    test_var = 0;
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

void main3() {
    int i;
    list_init(&list);
    pthread_t* threads = malloc(sizeof(pthread_t)*THREAD_COUNT);
    startTimer();
    for (i = 0; i < THREAD_COUNT; i++) {
        pthread_create(&threads[i], NULL, test4, (void*)i);
    }
    for (i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }
    printf("Time: %f\n", endTimer());
}

int main() {
    main3();
    return 0;
}