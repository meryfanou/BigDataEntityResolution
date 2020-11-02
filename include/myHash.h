// #include "myList.h"
// #include "myPatient.h"
#ifndef MYHASH_H
#define MYHASH_H
#include "mySpec.h"
#include "mySpec.h"

typedef struct hashTable hashTable;
typedef struct bucket bucket;
typedef struct record record;


struct hashTable{
	bucket** myTable;
	int maxRecs;
	int bucSize;
	int tableSize;
};

struct bucket{
	bucket* next;
	int cur;
	record* rec;
};

struct record{
	record* next;
	mySpec* spec;
};

hashTable* hash_create(int HashSize, int BucSize);
void hash_destroy(hashTable* table);
void hash_add(hashTable* table, mySpec* newSpec, int hash);
void hash_print(hashTable* t);

int hash1(char* key);

bucket* bucket_create(int size);
void bucket_add(bucket* buc, mySpec* newSpec);
void bucket_destroy(bucket* buc);
void bucket_print(bucket* b);
record* search_bucket(bucket*, char*);

record* record_create(mySpec* spec);
//void record_add(record* rec, mySpec* p);
void record_destroy(record* rec);
void record_print(record* rec);

#endif