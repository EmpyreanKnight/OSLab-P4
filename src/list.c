#include "list.h"

void list_init(list_t *list) {
    list->head = NULL;
    lock_init(&list->lock);
}

void list_insert(list_t *list, unsigned int key) {
    node_t *new = malloc(sizeof(node_t));
    if (new == NULL) {
        return;
    }
    new->key = key;
    //printf("%ld In insert1\n", pthread_self()%10000);
#if defined(LOCK_RWLOCK)
    rwlock_wrlock(&list->lock);
#else
    lock_acquire(&list->lock);
#endif
    //printf("%ld In insert2\n", pthread_self()%10000);
    new->next = list->head;
    list->head = new;
    //printf("%ld Out insert1\n", pthread_self()%10000);
    lock_release(&list->lock);
    //printf("%ld Out insert2\n", pthread_self()%10000);
}

void list_delete(list_t* list, unsigned int key) {
    //printf("%ld In delete1\n", pthread_self()%10000);
#if defined(LOCK_RWLOCK)
    rwlock_wrlock(&list->lock);
#else
    lock_acquire(&list->lock);
#endif
    //printf("%ld In delete2\n", pthread_self()%10000);
    node_t* cur = list->head;
    node_t* pre = NULL;
    while (cur != NULL) {
        if (cur->key == key) {
            break;
        }
        pre = cur;
        cur = cur->next;
    }
    if (cur != NULL) { // found target
        if (pre != NULL) {
            pre->next = cur->next;
        } else { // cur is head
            list->head = cur->next;
        }
        free(cur);
    }
    //printf("%ld out delete1\n", pthread_self()%10000);
    lock_release(&list->lock);
    //printf("%ld out delete2\n", pthread_self()%10000);
}

void* list_lookup(list_t* list, unsigned int key) {
    void* ret = NULL;
    //printf("%ld In lookup1\n", pthread_self()%10000);
#if defined(LOCK_RWLOCK)
    rwlock_rdlock(&list->lock);
#else
    lock_acquire(&list->lock);
#endif
    //printf("%ld In lookup2\n", pthread_self()%10000);
    node_t* cur = list->head;
    while (cur != NULL) {
        if (cur->key == key) {
            ret = cur;
            break;
        }
        cur = cur->next;
    }
    //printf("%ld Out lookup1\n", pthread_self()%10000);
    lock_release(&list->lock);
    //printf("%ld Out lookup2\n", pthread_self()%10000);
    return ret;
}

int list_count(list_t* list) {
    int cnt = 0;
    node_t *cur = list->head;
    while (cur != NULL) {
        cnt++;
        cur = cur->next;
    }
    return cnt;
}

long long list_sum(list_t* list) {
    long long res = 0;
    node_t *cur = list->head;
    while (cur != NULL) {
        res += cur->key;
        cur = cur->next;
    }
    return res;
}

void list_destroy(list_t* list) {
    node_t *cur = list->head;
    node_t *pre = NULL;
    while (cur != NULL) {
        pre = cur;
        cur = cur->next;
        free(pre);
    }
}