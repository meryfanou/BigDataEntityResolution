#ifndef MYHASH_H
#define MYHASH_H
#include "mySpec.h"
#include "boWords.h"
// #include "functs.h"

typedef struct hashTable hashTable;
typedef struct bucket bucket;
typedef struct record record;


struct hashTable{
	bucket** myTable;
	int entries;
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
	char visited;
};

hashTable* hash_create(int HashSize, int BucSize);
void hash_destroy(hashTable* table);
void hash_add(hashTable* table, mySpec* newSpec, int hash);
void hash_print(hashTable* t);

int hash1(char* key);

bucket* bucket_create(int size);
void bucket_add(bucket* buc, mySpec* newSpec);
void bucket_destroy(hashTable*, bucket* buc);
void bucket_print(bucket* b);
record* search_bucket(bucket*, char*);

record* record_create(mySpec* spec);
void record_destroy(hashTable*, record* rec);
void record_print(record* rec);

mySpec* findRecord_byKey(hashTable*, char*);
mySpec* findRecord_forSet(hashTable*, char*, char);		// Find a record for either the testing or the validation set
														// Make sure each spec is added only once to a set
void hash_to_bow(hashTable*, BoWords*);
void hash_bucket_to_bow(bucket*, BoWords*);
void hash_record_to_bow(record*, BoWords*);
#endif