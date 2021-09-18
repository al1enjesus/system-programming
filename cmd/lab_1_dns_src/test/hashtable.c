//
// Created by Il on 14.09.2021.
//

#include <stdio.h>
#include "hashtable.h"

size_t getHash(const char *key, size_t keyLength, size_t hashTableSize) {
    size_t sum = 0;
    for (size_t i = 0; i < keyLength; i++) {
        sum += (size_t) (key)[i] * (i + 1);
    }
    return sum % (hashTableSize);
}

HashTable *HashTableInit() {
    HashTable *hashTable = (HashTable *) malloc(sizeof(HashTable));
    hashTable->arraySize = HT_INITIAL_SIZE;
    hashTable->array = calloc(hashTable->arraySize, sizeof(**(hashTable->array)));

    for (unsigned int i = 0; i < hashTable->arraySize; i++) {
        hashTable->array[i] = NULL;
    }
    return hashTable;
}

char *HashTableSearch(HashTable *hashTable, char *key) {
    size_t index = getHash(key, strlen(key), hashTable->arraySize);
    if (hashTable->array[index] == NULL) {
        return NULL;
    }
    Item *node = hashTable->array[index];
    while (node != NULL) {
        if (node->key_size == strlen(key)) {
            if (!strcmp(key, node->key)) {
                return node->value;
            }
        }
        node = node->next;
    }

    return NULL;
}

void HashTableInsert(HashTable *hashTable, char *key, char *value) {
    size_t index = getHash(key, strlen(key), hashTable->arraySize);
    Item *node, *last_node;
    node = hashTable->array[index];
    last_node = NULL;

    while (node != NULL) {
        // todo: write overwriting if similar keys
        last_node = node;
        node = node->next;
    }

    Item *new_node;
    new_node = malloc(sizeof(Item));
    new_node->key = malloc(strlen(key));
    new_node->value = malloc(strlen(value));
    strcpy(new_node->key, key);
    strcpy(new_node->value, value);
    new_node->key_size = strlen(key);
    new_node->next = NULL;

    if (last_node != NULL) {
        last_node->next = new_node;
    } else {
        hashTable->array[index] = new_node;
    }
}