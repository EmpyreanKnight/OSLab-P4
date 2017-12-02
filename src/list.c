#include "list.h"
#include <stdio.h>

extern int user_error;

/**
 * Initialize the given list
 * @param list A pointer to a list
 */
void list_init(list_t *list) {
    list->head = NULL;
    lock_init(&list->lock);
}

/**
 * Insert a new node with value key at the head of the list
 * @param list A pointer to a list
 * @param key the value to be inserted
 */
void list_insert(list_t *list, unsigned int key) {
    node_t *new_node = malloc(sizeof(node_t));
    if (newNode == NULL) {
        perror("malloc");
        return;
    }
    new_node->key = key;
    //printf("%ld In insert1\n", pthread_self()%10000);
#if defined(LOCK_RWLOCK)
    rwlock_wrlock(&list->lock);
#else
    lock_acquire(&list->lock);
#endif
    //printf("%ld In insert2\n", pthread_self()%10000);
    new_node->next = list->head;
    list->head = new_node;
    //printf("%ld Out insert1\n", pthread_self()%10000);
    lock_release(&list->lock);
    //printf("%ld Out insert2\n", pthread_self()%10000);
}

/**
 * Delete the node with value key
 * @param list A pointer to a list
 * @param key The key value of the node to be deleted
 */
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
    }else {
        user_error = E_NO_CORRESPONDING_VALUE;
    }
    //printf("%ld out delete1\n", pthread_self()%10000);
    lock_release(&list->lock);
    //printf("%ld out delete2\n", pthread_self()%10000);
}

/**
 *  Find out whether there is a node with a value of key.
 *  If there is, return the pointer to the node with value key,
 *  return NULL otherwise
 * @param list A pointer to a list
 * @param key The key value of the node to be lookup
 * @return A pointer to the node with value key
 */
void* list_lookup(list_t* list, unsigned int key) {
#if defined(LOCK_RWLOCK)
    rwlock_rdlock(&list->lock);
#else
    lock_acquire(&list->lock);
#endif
    //printf("%ld In lookup1\n", pthread_self()%10000);
    //printf("%ld In lookup2\n", pthread_self()%10000);
    node_t* cur = list->head;
    while (cur != NULL) {
        if (cur->key == key) {
            break;
        }
        cur = cur->next;
    }
    //printf("%ld Out lookup1\n", pthread_self()%10000);
    lock_release(&list->lock);
    //printf("%ld Out lookup2\n", pthread_self()%10000);
    return cur;
}

/**
 *  Calculate the total number of nodes in list
 * @param list A pointer to a list
 * @return the total number of nodes in list
 */
int list_count(list_t* list) {
#if defined(LOCK_RWLOCK)
    rwlock_rdlock(&list->lock);
#else
    lock_acquire(&list->lock);
#endif
    int cnt = 0;
    node_t *cur = list->head;
    while (cur != NULL) {
        if (cnt == INT_MIN) {
            user_error = E_DATA_OVERFLOW;
        }
        cnt++;
        cur = cur->next;
    }
    lock_release(&list->lock);
    return cnt;
}

/**
 *  Calculate the sum of all nodes' value
 * @param list A pointer to a list
 * @return the sum of all nodes' value
 */
long long list_sum(list_t* list) {
#if defined(LOCK_RWLOCK)
    rwlock_rdlock(&list->lock);
#else
    lock_acquire(&list->lock);
#endif
    long long res = 0;
    node_t *cur = list->head;
    while (cur != NULL) {
        res += cur->key;
        cur = cur->next;
    }
    lock_release(&list->lock);
    return res;
}

/**
 *  Destroy the given list
 * @param list A pointer to a list
 */
void list_destroy(list_t* list) {
#if defined(LOCK_RWLOCK)
    rwlock_wrlock(&list->lock);
#else
    lock_acquire(&list->lock);
#endif
    node_t *cur = list->head;
    node_t *pre = NULL;
    while (cur != NULL) {
        pre = cur;
        cur = cur->next;
        free(pre);
    }
    lock_release(&list->lock);
}