//
// Created by Il on 14.09.2021.
//

#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include "hashtable.h"


uint32_t murmurhash (const char *key, uint32_t len, uint32_t seed) {
    uint32_t c1 = 0xcc9e2d51;
    uint32_t c2 = 0x1b873593;
    uint32_t r1 = 15;
    uint32_t r2 = 13;
    uint32_t m = 5;
    uint32_t n = 0xe6546b64;
    uint32_t h = 0;
    uint32_t k = 0;
    uint8_t *d = (uint8_t *) key; // 32 bit extract from `key'
    const uint32_t *chunks = NULL;
    const uint8_t *tail = NULL; // tail - last 8 bytes
    int i = 0;
    int l = len / 4; // chunk length

    h = seed;

    chunks = (const uint32_t *) (d + l * 4); // body
    tail = (const uint8_t *) (d + l * 4); // last 8 byte chunk of `key'

    // for each 4 byte chunk of `key'
    for (i = -l; i != 0; ++i) {
        // next 4 byte chunk of `key'
        k = chunks[i];

        // encode next 4 byte chunk of `key'
        k *= c1;
        k = (k << r1) | (k >> (32 - r1));
        k *= c2;

        // append to hash
        h ^= k;
        h = (h << r2) | (h >> (32 - r2));
        h = h * m + n;
    }

    k = 0;

    // remainder
    switch (len & 3) { // `len % 4'
        case 3: k ^= (tail[2] << 16);
        case 2: k ^= (tail[1] << 8);

        case 1:
            k ^= tail[0];
            k *= c1;
            k = (k << r1) | (k >> (32 - r1));
            k *= c2;
            h ^= k;
    }

    h ^= len;

    h ^= (h >> 16);
    h *= 0x85ebca6b;
    h ^= (h >> 13);
    h *= 0xc2b2ae35;
    h ^= (h >> 16);

    return h;
}

HashTable *HashTableInit() {
    HashTable *hashTable = (HashTable *) malloc(sizeof(HashTable));
    hashTable->arraySize = HT_INITIAL_SIZE;
    hashTable->seed = time(NULL);
    hashTable->array = calloc(hashTable->arraySize, sizeof(**(hashTable->array)));

    for (uint32_t i = 0; i < hashTable->arraySize; i++) {
        hashTable->array[i] = NULL;
    }

    return hashTable;
}

char *HashTableSearch(HashTable *hashTable, char *key) {
    uint32_t key_length = strlen(key);

    uint32_t index = murmurhash(key, key_length, hashTable->seed) % hashTable->arraySize;
    if (hashTable->array[index] == NULL) {
        return NULL;
    }
    Item *node = hashTable->array[index];
    while (node != NULL) {
        if (node->key_size == key_length) {
            if (!strcmp(key, node->key)) {
                return node->value;
            }
        }
        node = node->next;
    }
    return NULL;
}

void HashTableInsert(HashTable *hashTable, char *key, char *value) {
    uint32_t key_length = strlen(key);
    uint32_t value_length = strlen(value);

    uint32_t index = murmurhash(key, key_length, hashTable->seed) % hashTable->arraySize;
    Item *node, *last_node;
    node = hashTable->array[index];
    last_node = NULL;

    while (node != NULL) {
        last_node = node;
        if (!strcmp(last_node->key, key)){
            break;
        }
        node = node->next;
    }

    Item *new_node;
    new_node = malloc(sizeof(Item));
    new_node->key = malloc(key_length);
    new_node->value = malloc(value_length);
    strcpy(new_node->key, key);
    strcpy(new_node->value, value);
    new_node->key_size = key_length;
    new_node->next = NULL;

    if (last_node != NULL) {
        last_node->next = new_node;
    } else {
        hashTable->array[index] = new_node;
    }
}

void HashTableRemove(HashTable *hashTable){
    for (uint32_t index = 0; index < HT_INITIAL_SIZE; index ++ ){
        Item *node = hashTable->array[index];
        Item *next_node = NULL;
        while (node != NULL){
            next_node = node->next;
            free(node->key);
            free(node->value);
            free(node->next);
            free(node);
            node = next_node;
        }
        free(hashTable->array[index]);
    }
    free(hashTable);
}