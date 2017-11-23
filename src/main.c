#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

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

#define THREAD_COUNT 5
#define MAX_N 10000

counter_t counter;

void* test(void* args) {
    int i;
    srand((unsigned) time(0));
    for (i = 0; i < MAX_N; i++) {
        counter_increment(&counter);
    }
}

int main() {
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
    printf("%f\n", endTimer());
    printf("Counter value: %d\n", counter_get_value(&counter));
    return 0;
}