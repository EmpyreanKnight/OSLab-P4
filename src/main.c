#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

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

int THREAD_COUNT = 4;
int MAX_N = 20000;
#define SEED 1551
#define HASH_SIZE 1000

#define READ_RATE 70
#define INSERT_RATE 15
#define RANGE 1000

counter_t counter;
list_t list;
hash_t hash;

void* test_lock(void *args) {
    int i;
    for (i = 0; i < MAX_N; i++) {
        counter_increment(&counter);
    }
    return NULL;
}

void* test_counter(void *args) {
    int i;
    srand(SEED + (unsigned)(unsigned long)args);
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

void* test_list(void *args) {
    int i;
    srand(SEED + (unsigned)(unsigned long)args);
    for (i = 0; i < MAX_N; i++) {
        int rd = rand() % 100;
        if (rd < READ_RATE) {
            list_lookup(&list, (unsigned)(rand() % RANGE));
        } else if (rd < READ_RATE + INSERT_RATE) {
            int value = rand() % RANGE;
            list_insert(&list, (unsigned)value);
        } else {
            int value = rand() % RANGE;
            list_delete(&list, (unsigned)value);
        }
    }
    return NULL;
}

void* test_list_order(void *args) {
    int i;
    srand(SEED + (unsigned)(unsigned long)args);
    for (i = 0; i < MAX_N; i++) {
        int value = rand() % RANGE;
        list_insert(&list, (unsigned) value);
    }
    for (i = 0; i < MAX_N; i++) {
        int value = rand() % RANGE;
        list_delete(&list, (unsigned) value);
    }
    return NULL;
}

void* test_hash(void *args) {
    int i;
    srand(SEED + (unsigned)(unsigned long)args);
    for (i = 0; i < MAX_N; i++) {
        int rd = rand() % 100;
        if (rd < READ_RATE) {
            hash_lookup(&hash, (unsigned)(rand() % RANGE));
        } else if (rd < READ_RATE + INSERT_RATE) {
            int value = rand() % RANGE;
            hash_insert(&hash, (unsigned)value);
        } else {
            int value = rand() % RANGE;
            hash_delete(&hash, (unsigned)value);
        }
    }
    return NULL;
}

void* test_hash_order(void *args) {
    int i;
    srand(SEED + (unsigned)(unsigned long)args);
    for (i = 0; i < MAX_N; i++) {
        int value = rand() % RANGE;
        hash_insert(&hash, (unsigned) value);
    }
    for (i = 0; i < MAX_N; i++) {
        int value = rand() % RANGE;
        hash_delete(&hash, (unsigned) value);
    }
    return NULL;
}

double timeTotal[100];

void* test_exec(void *args) {
    int i;
    int id = (int)(unsigned long)args;
    struct timeval tvb, tve;
    gettimeofday(&tvb, NULL);
    for (i = 0; i < MAX_N; i++) {
        counter_increment(&counter);
    }
    gettimeofday(&tve, NULL);
    timeTotal[id] = (tve.tv_usec - tvb.tv_usec) / 1000.0 + (tve.tv_sec - tvb.tv_sec) * 1000.0;
    return NULL;
}

void* test_acquire(void *args) {
    int i;
    int id = (int)(unsigned long)args;
    timeTotal[id] = 0;
    struct timeval tvb, tve;
    for (i = 0; i < MAX_N; i++) {
        gettimeofday(&tve, NULL);
        if (i > 0) {
            timeTotal[id] += (tve.tv_usec - tvb.tv_usec) / 1000.0 + (tve.tv_sec - tvb.tv_sec) * 1000.0;
        }
        gettimeofday(&tvb, NULL);
        counter_increment(&counter);
    }
    return NULL;
}

void lock_performance() {
    int i;
    counter_init(&counter, 0);
    pthread_t* threads = malloc(sizeof(pthread_t)*THREAD_COUNT);

    startTimer();
    for (i = 0; i < THREAD_COUNT; i++) {
        pthread_create(&threads[i], NULL, test_lock, (void *)(unsigned long) i);
    }
    for (i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }

    //printf("Lock runtime:\n");
    printf("%f, ", endTimer());
}

void counter_performance() {
    int i;
    counter_init(&counter, 0);
    pthread_t* threads = malloc(sizeof(pthread_t)*THREAD_COUNT);

    startTimer();
    for (i = 0; i < THREAD_COUNT; i++) {
        pthread_create(&threads[i], NULL, test_counter, (void *)(unsigned long) i);
    }
    for (i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }

    //printf("Counter runtime:\n");
    printf("%f, ", endTimer());
}

void list_performance() {
    int i;
    list_init(&list);
    pthread_t* threads = malloc(sizeof(pthread_t)*THREAD_COUNT);

    startTimer();
    for (i = 0; i < THREAD_COUNT; i++) {
        pthread_create(&threads[i], NULL, test_list, (void *)(unsigned long) i);
        //pthread_create(&threads[i], NULL, test_list_order, (void *)(unsigned long) i);
    }
    for (i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }

    //printf("List runtime:\n");
    printf("%f, ", endTimer());
    list_destroy(&list);
}

void hash_performance() {
    int i;
    hash_init(&hash, HASH_SIZE);
    pthread_t* threads = malloc(sizeof(pthread_t)*THREAD_COUNT);

    startTimer();
    for (i = 0; i < THREAD_COUNT; i++) {
        pthread_create(&threads[i], NULL, test_hash, (void *)(unsigned long) i);
        //pthread_create(&threads[i], NULL, test_hash_order, (void *)(unsigned long) i);
    }
    for (i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }

    //printf("Hash runtime:\n");
    printf("%f, ", endTimer());
}

void fairness_execution() {
    int i;
    counter_init(&counter, 0);
    pthread_t* threads = malloc(sizeof(pthread_t)*THREAD_COUNT);
    for (i = 0; i < THREAD_COUNT; i++) {
        pthread_create(&threads[i], NULL, test_exec, (void *)(unsigned long) i);
    }
    for (i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }

    //printf("Average execution time:\n");
    printf("np.array([");
    for (i = 0; i < THREAD_COUNT; i++) {
        printf("%f, ", 1.0 * timeTotal[i]);
    }
    printf("\b\b]).var(),\n");
}

void fairness_reacquire() {
    int i;
    counter_init(&counter, 0);
    pthread_t* threads = malloc(sizeof(pthread_t)*THREAD_COUNT);
    for (i = 0; i < THREAD_COUNT; i++) {
        pthread_create(&threads[i], NULL, test_acquire, (void *)(unsigned long) i);
    }
    for (i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }

    //printf("Average reacquire time:\n");
    printf("cv(np.array([");
    for (i = 0; i < THREAD_COUNT; i++) {
        printf("%f, ", timeTotal[i]);
    }
    printf("\b\b])),\n");
}

// TODO: hash scaling, hash/list insertion/insertion&delete (serial/random)
int main() {
    int n = 6;
    char* notice[] = {
            "Lock performance",
            "Counter performance",
            "List performance",
            "Hash performance",
            "Fairness (execution)",
            "Fairness (reacquire)"
    };
    printf("Test options:\n");
    for (int i = 0; i < n; i++) {
        printf("%s\t%d\n", notice[i], i);
    }

    int op;
    scanf("%d", &op);

    for (THREAD_COUNT = 1; THREAD_COUNT <= 8; THREAD_COUNT++) {
        //printf("threads: %d, n: %d\n", THREAD_COUNT, MAX_N);
        switch (op) {
            case 0:
                lock_performance();
                break;
            case 1:
                counter_performance();
                break;
            case 2:
                list_performance();
                break;
            case 3:
                hash_performance();
                break;
            case 4:
                fairness_execution();
                break;
            case 5:
                fairness_reacquire();
                break;
            default:
                printf("No such option!");
        }
    }
    return 0;
}