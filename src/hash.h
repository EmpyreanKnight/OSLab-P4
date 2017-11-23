#ifndef P4_HASH_H
#define P4_HASH_H

#include "list.h"

typedef struct {
    list_t *lists;
    int bucket_size;
} hash_t;

void hash_init(hash_t *hash, int size);
void hash_insert(hash_t *hash, unsigned int key);
void hash_delete(hash_t *hash, unsigned int key);
void *hash_lookup(hash_t *hash, unsigned int key);
void* hash_destroy(hash_t *hash);

#endif //P4_HASH_H
