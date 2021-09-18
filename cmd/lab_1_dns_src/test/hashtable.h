//
// Created by Il on 04.09.2021.
//

#include <malloc.h>
#include <stddef.h>
#include <memory.h>
#include <string.h>

#define HT_INITIAL_SIZE 13000;

size_t getHash(const char *key, size_t keyLength, size_t hashTableSize);

typedef struct Item
{
    char *key;
    char *value;
    size_t key_size;

    struct Item *next;
} Item;

typedef struct HashTable
{
    Item ** array;
    int arraySize;
} HashTable;

HashTable *HashTableInit();

char *HashTableSearch(HashTable *hashTable, char *key);

void HashTableInsert(HashTable *hashTable, char *key, char *value);