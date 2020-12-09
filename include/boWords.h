#ifndef BOWORDS_H
#define BOWORDS_H

#include "./mySpec.h"

typedef struct MBH MBH;

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
	char* word;
	int isSignificant;
	int significance;
    TextInfo* texts;
    int numofTexts;
};

struct TextInfo{
    mySpec* text;
    float numofInstances;
};

BoWords* bow_create(int ,int);
void bow_destroy(BoWords*);
void bow_add(BoWords*, char*, mySpec*, int);
void bow_print(BoWords*);
void bow_vectorize(BoWords*, float**, int*, mySpec*);
void bow_get_signWords(BoWords*, MBH*);
int bow_set_significance(BoWords*, char*);
void bow_keep_signWords(BoWords*);

Bucket* bow_bucket_create(int);
void bow_bucket_add(Bucket*, char*, mySpec*);
void bow_bucket_destroy(BoWords*, Bucket*);
void bow_bucket_print(Bucket*);
Record* bow_search_bucket(Bucket*, char*);
void bow_bucket_vectorize(Bucket*, float**, int*, mySpec*);
void bow_bucket_signWords(Bucket*, MBH*);
void bow_bucket_keep_signWords(BoWords*, Bucket*);

Record* bow_record_create(char*, mySpec*);
void bow_record_destroy(BoWords*, Record*);
void bow_record_print(Record*);
void bow_record_update(Record*, mySpec*);
void bow_record_vectorize(Record*, float**, int*, mySpec*);


#endif