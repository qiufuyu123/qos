/*
 * @Description: hashmap
 * @Author: QIUFUYU
 * @Date: 2021-12-10 20:55:00
 * @LastEditTime: 2021-12-13 22:29:16
 */
#ifndef _H_HASHMAP
#define _H_HASHMAP
#include"types.h"

#define BUCKETCOUNT 16
struct hashEntry
{
    uint32 key;
    char* value;
    struct hashEntry* next;
};

typedef struct hashEntry entry;

struct hashTable
{
    entry bucket[BUCKETCOUNT];  //先默认定义16个桶
};

typedef struct hashTable table;
int keyToIndex(char * key);
void initHashTable(table* t);
void freeHashTable(table* t);
int insertEntry(table* t , uint32 key ,  char* value);
const char* findValueByKey(const table* t , uint32 key);
entry* removeEntry(table* t , uint32 key);
#endif