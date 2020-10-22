// #include "myList.h"
// #include "myPatient.h"
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
void hash_add(hashTable* table, char* key, list_node*, int hash);
void hash_print(hashTable* t);

int hash1(char* key);

bucket* bucket_create(int size);
void bucket_add(bucket* buc, char*, list_node* p);
void bucket_destroy(bucket* buc);
void bucket_print(bucket* b);
record* search_bucket(bucket*, char*);

record* record_create(char* key, list_node* p);
void record_add(record* rec, list_node* p);
void record_destroy(record* rec);
void record_print(record* rec);