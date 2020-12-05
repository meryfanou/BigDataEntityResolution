#ifndef BOWORDS_H
#define BOWORDS_H

#include "./mySpec.h"

typedef struct BoWords BoWords;
typedef struct Bucket Bucket;
typedef struct Record Record;


struct BoWords{
	Bucket** myTable;
	int entries;
	int maxRecs;
	int bucSize;
	int tableSize;
};

struct Bucket{
	Bucket* next;
	int cur;
	Record* rec;
};

struct Record{
	Record* next;
    TextInfo* texts;
    int numofTexts;
};

struct TextInfo{
    mySpec* text;
    int   numofInstances;
};

BoWords* hash_create(int ,int);
void hash_destroy(BoWords*);
void hash_add(BoWords*, mySpec*, int, int);
void hash_print(BoWords*);

int hash1(char*);

Bucket* bucket_create(int);
void bucket_add(Bucket*, mySpec*, int);
void bucket_destroy(BoWords*, Bucket*);
void bucket_print(Bucket*);
Record* search_bucket(Bucket*, char*);

Record* record_create(mySpec*);
void record_destroy(BoWords*, Record*);
void record_print(Record*);

mySpec* findRecord_byKey(BoWords*, char*);

#endif