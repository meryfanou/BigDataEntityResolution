#ifndef BOWORDS_H
#define BOWORDS_H

#include "./mySpec.h"

typedef struct BoWords BoWords;
typedef struct Bucket Bucket;
typedef struct Record Record;
typedef struct TextInfo TextInfo;


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

BoWords* bow_create(int ,int);
void bow_destroy(BoWords*);
void bow_add(BoWords*, mySpec*, int, int);
void bow_print(BoWords*);

int hash_function(char*);

Bucket* bow_bucket_create(int);
void bow_bucket_add(Bucket*, mySpec*, int);
void bow_bucket_destroy(BoWords*, Bucket*);
void bow_bucket_print(Bucket*);
Record* bow_search_bucket(Bucket*, char*);

Record* bow_record_create(mySpec*);
void bow_record_destroy(BoWords*, Record*);
void bow_record_print(Record*);

//mySpec* findRecord_byKey(BoWords*, char*);

#endif