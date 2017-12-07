#include "hash.h"

/**
 * Initialize the hash table with given bucket size
 * @param hash A pointer to hash table
 * @param size Designated bucket size
 */
void hash_init(hash_t *hash, int size) {
    int i;
    hash->bucket_size = size;
    hash->lists = malloc(sizeof(list_t)*size);
    for (i = 0; i < size; i++) {
        list_init(&hash->lists[i]);
    }
}

/**
 * Insert a key into hash table
 * NOT make duplicated keys unique
 * @param hash A pointer to hash table
 * @param key A key to be inserted
 */
void hash_insert(hash_t *hash, unsigned int key) {
    int bucket = key % hash->bucket_size;
    list_insert(&hash->lists[bucket], key);
}

/**
 * Delete a key into hash table
 * If multiple keys detected in hash table, only delete one of them
 * @param hash The pointer to hash table
 * @param key The key to be deleted
 */
void hash_delete(hash_t *hash, unsigned int key) {
    int bucket = key % hash->bucket_size;
    list_delete(&hash->lists[bucket], key);
}

/**
 * Find a given key in hash table
 * @param hash The pointer to hash table
 * @param key The key to find
 * @return A pointer to the node with given key, should cast to node_t type before use.
 */
void* hash_lookup(hash_t *hash, unsigned int key) {
    int bucket = key % hash->bucket_size;
    return list_lookup(&hash->lists[bucket], key);
}

/**
 * Remove an given hash table and free the pointer of it
 * @param hash The pointer to hash table
 */
void hash_destroy(hash_t *hash) {
    int i;
    for (i = 0; i < hash->bucket_size; i++) {
        list_destroy(&hash->lists[i]);
    }
    free(hash->lists);
}
