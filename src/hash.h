#ifndef P4_HASH_H
#define P4_HASH_H

#include "list.h"

/**
 * The concurrent hash definition
 * The hash function is simply module bucket size (but effective)
 * This implementation need no more parallel protection since list is already thread-safe
 * All operations except initialize and destroy are thread-safe
 */
typedef struct {
    list_t *lists;   /**< lists for hash buckets */
    int bucket_size; /**< the bucket size designated when initialized, can't be changed during use */
} hash_t;

void hash_init(hash_t *hash, int size);
void hash_insert(hash_t *hash, unsigned int key);
void hash_delete(hash_t *hash, unsigned int key);
void *hash_lookup(hash_t *hash, unsigned int key);
void* hash_destroy(hash_t *hash);

#endif //P4_HASH_H
