#ifndef P4_LIST_H
#define P4_LIST_H

#include "lock.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * Node type in the list
 *
 */
typedef struct _node_t {
    unsigned int key;     /**< the key field of this node */
    struct _node_t *next; /**< pointer to the next node in the list */
} node_t;


/**
 * A concurrent list definition
 * All operations except initialization and destroy are thread-safe
 * Maintain a head-insert linked-list
 */
typedef struct {
    node_t *head; /**< a pointer to the head node */
    lock_t lock;  /**< guarantee sequential execution in list functions */
} list_t;

void list_init(list_t *list);
void list_insert(list_t *list, unsigned int key);
void list_delete(list_t *list, unsigned int key);
void *list_lookup(list_t *list, unsigned int key);
void list_destroy(list_t* list);

int list_count(list_t* list);
long long list_sum(list_t* list);

#endif //P4_LIST_H
