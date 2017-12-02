#include "hash.h"

/**
 * Initialize hash table with given bucket size
 * will call perror to print error message once malloc fail
 * @param hash A pointer to hash table
 * @param size designated bucket size
 *//*
void hash_init(hash_t *hash, int size, unsigned int threshold) {    // hash_init with threshold
    hash->lists = malloc(sizeof(list_t)*size);
    if (hash->lists == NULL) {
        perror("malloc");
        return;
    }
    int i;
    hash->bucket_size = size;
    hash->threshold = threshold;
    for (i = 0; i < size; i++) {
        list_init(&hash->lists[i]);
    }
    //lock_init(&hash->lock);
}*/
void hash_init(hash_t *hash, int size) {    // hash_init without threshold
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
 * if multiple keys detected in hash table, only delete one of them
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
 * @return A pointer to node contains given key, should a into node_t before use.
 */
void* hash_lookup(hash_t *hash, unsigned int key) {
    int bucket = key % hash->bucket_size;
    return list_lookup(&hash->lists[bucket], key);
}

/**
 * remove an given hash table and free the pointer of it
 * @param hash The pointer to hash table
 */
void* hash_destroy(hash_t *hash) {
    int i;
    for (i = 0; i < hash->bucket_size; i++) {
        list_destroy(&hash->lists[i]);
    }
    free(hash->lists);
}

/**
 * Check whether the total length of the hash table has exceeded the threshold
 * if so resize the hash table and destory the previous one
 * @param hash The pointer to a hash table
 */
void hash_resize(hash_t *hash) {
    unsigned int i, tmp_key, length = 0;
    for (i = 0; i < hash->bucket_size; ++i) {
        length += list_count(&hash->lists[i]);
    }
    if (length < hash->threshold) {
        return;
    }
    hash_t new_hash;
    hash_init(&new_hash, length*2, 2*hash->threshold);
    for (i = 0; i < hash->bucket_size; ++i) {
        list_t tmp_list = hash->lists[i];
        node_t *cur = tmp_list.head;
        while (cur != NULL) {
            tmp_key = cur->key;
            hash_insert(&new_hash, tmp_key);
            cur = cur->next;
        }
    }
    hash_t* hash_old = hash;
    hash = &new_hash;
    hash_destroy(hash_old);
}