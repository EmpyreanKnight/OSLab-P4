#include "list.h"

void list_init(list_t *list) {
    list->head = NULL;
    mutex_init(&list->lock);
}

void list_insert(list_t *list, unsigned int key) {
    node_t *new = malloc(sizeof(node_t));
    if (new == NULL) {
        return;
    }
    new->key = key;
    mutex_acquire(&list->lock);
    new->next = list->head;
    list->head = new;
    mutex_release(&list->lock);
}

void list_delete(list_t* list, unsigned int key) {
    mutex_acquire(&list->lock);
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
    mutex_release(&list->lock);
}

void* list_lookup(list_t* list, unsigned int key) {
    void* ret = NULL;
    mutex_acquire(&list->lock);
    node_t* cur = list->head;
    while (cur != NULL) {
        if (cur->key == key) {
            ret = cur;
            break;
        }
        cur = cur->next;
    }
    mutex_release(&list->lock);
    return ret;
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