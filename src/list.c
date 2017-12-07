#include "list.h"

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
 * @param key The value to be inserted
 */
void list_insert(list_t *list, unsigned int key) {
    node_t *new_node = malloc(sizeof(node_t));
    new_node->key = key;
#if defined(LOCK_RWLOCK) || defined(LOCK_PRWLOCK)
    rwlock_wrlock(&list->lock);
#else
    lock_acquire(&list->lock);
#endif
    new_node->next = list->head;
    list->head = new_node;
    lock_release(&list->lock);
}

/**
 * Delete one node with the given value
 * If multiple targets are found, only delete one of them
 * @param list A pointer to a list
 * @param key The key value of the node to be deleted
 */
void list_delete(list_t* list, unsigned int key) {
#if defined(LOCK_RWLOCK) || defined(LOCK_PRWLOCK)
    rwlock_wrlock(&list->lock);
#else
    lock_acquire(&list->lock);
#endif
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
    lock_release(&list->lock);
}

/**
 * Find out whether there exists a node with the given key.
 * If found, return a pointer to the target node.
 * Otherwise, return NULL instead.
 *
 * @param list A pointer to a list
 * @param key The key value of the node to be lookup
 * @return A pointer to the node with given key, should cast to node_t type before use.
 */
void* list_lookup(list_t* list, unsigned int key) {
#if defined(LOCK_RWLOCK) || defined(LOCK_PRWLOCK)
    rwlock_rdlock(&list->lock);
#else
    lock_acquire(&list->lock);
#endif
    node_t* cur = list->head;
    while (cur != NULL) {
        if (cur->key == key) {
            break;
        }
        cur = cur->next;
    }
    lock_release(&list->lock);
    return cur;
}

/**
 * Calculate the total number of nodes in this list
 * @param list A pointer to a list
 * @return The total number of nodes in the list
 */
int list_count(list_t* list) {
#if defined(LOCK_RWLOCK) || defined(LOCK_PRWLOCK)
    rwlock_rdlock(&list->lock);
#else
    lock_acquire(&list->lock);
#endif
    int cnt = 0;
    node_t *cur = list->head;
    while (cur != NULL) {
        cnt++;
        cur = cur->next;
    }
    lock_release(&list->lock);
    return cnt;
}

/**
 * Calculate the sum of all nodes' key field
 * @param list A pointer to a list
 * @return The sum of all nodes' value
 */
long long list_sum(list_t* list) {
#if defined(LOCK_RWLOCK) || defined(LOCK_PRWLOCK)
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
 * Destroy the given list
 * @param list The list to be deleted
 */
void list_destroy(list_t* list) {
#if defined(LOCK_RWLOCK) || defined(LOCK_PRWLOCK)
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