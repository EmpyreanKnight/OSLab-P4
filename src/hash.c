#include "hash.h"

void hash_init(hash_t *hash, int size) {
    hash->bucket_size = size;
    hash->lists = malloc(sizeof(list_t)*size);
}

void hash_insert(hash_t *hash, unsigned int key) {
    int bucket = key % hash->bucket_size;
    list_insert(&hash->lists[bucket], key);
}

void hash_delete(hash_t *hash, unsigned int key) {
    int bucket = key % hash->bucket_size;
    list_delete(&hash->lists[bucket], key);
}

void* hash_lookup(hash_t *hash, unsigned int key) {
    int bucket = key % hash->bucket_size;
    return list_lookup(&hash->lists[bucket], key);
}

void* hash_destroy(hash_t *hash) {
    int i;
    for (i = 0; i < hash->bucket_size; i++) {
        list_destroy(&hash->lists[i]);
    }
    free(hash->lists);
}