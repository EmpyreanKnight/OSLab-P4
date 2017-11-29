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
#if defined(LOCK_RWLOCK)
    writer_lock(&list->lock);
#else
    lock_acquire(&list->lock);
#endif
    new->next = list->head;
    list->head = new;
#if defined(LOCK_RWLOCK)
    writer_unlock(&list->lock);
#else
    lock_release(&list->lock);
#endif
}

void list_delete(list_t* list, unsigned int key) {
#if defined(LOCK_RWLOCK)
    writer_lock(&list->lock);
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
#if defined(LOCK_RWLOCK)
    writer_unlock(&list->lock);
#else
    lock_release(&list->lock);
#endif
}

void* list_lookup(list_t* list, unsigned int key) {
    void* ret = NULL;
#if defined(LOCK_RWLOCK)
    reader_lock(&list->lock);
#else
    lock_acquire(&list->lock);
#endif
    node_t* cur = list->head;
    while (cur != NULL) {
        if (cur->key == key) {
            ret = cur;
            break;
        }
        cur = cur->next;
    }
#if defined(LOCK_RWLOCK)
    reader_unlock(&list->lock);
#else
    lock_release(&list->lock);
#endif
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