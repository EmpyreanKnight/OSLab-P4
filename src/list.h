#ifndef P4_LIST_H
#define P4_LIST_H

#include "lock.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct _node_t {
    unsigned int key;
    struct _node_t *next;
} node_t;

typedef struct {
    node_t *head;
    lock_t lock;
} list_t;

void list_init(list_t *list);
void list_insert(list_t *list, unsigned int key);
void list_delete(list_t *list, unsigned int key);
void *list_lookup(list_t *list, unsigned int key);
void list_destroy(list_t* list);

#endif //P4_LIST_H
