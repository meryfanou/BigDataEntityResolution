#ifndef BOWORDS_H
#define BOWORDS_H

#include "./mySpec.h"
#include <math.h>

typedef struct MBH MBH;

typedef struct BoWords BoWords;
typedef struct Bucket Bucket;
typedef struct Record Record;
typedef struct TextInfo TextInfo;
typedef struct TfIdf_model tfidf;

struct BoWords{
	Bucket** myTable;
	int entries;
	int maxRecs;
	int bucSize;
	int tableSize;
	int specsSum;
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
	float idf;
	float tfidf_sum_mean;
    TextInfo* texts;
    int numofTexts;
};

struct TextInfo{
    mySpec* text;
    float tf_idf;
};

struct TfIdf_model{
	int count_Words;
	int all_Words;
	int max_Words;

	int count_Texts;
	int all_Texts;
	int max_Texts;
};

// ~~~~~~~~~~~~~~~~~~~~ BOW ~~~~~~~~~~~~~~~~~~~~~~~

BoWords* bow_create(int ,int);							// Create a boWords structure (hash table)
void bow_destroy(BoWords*);
void bow_add(BoWords*, char*, mySpec*, int);			// Add a word about a text (spec) to bow
void bow_print(BoWords*);
void bow_vectorize(BoWords*, float**, int*, mySpec*);	// Get a vector that corresponds to the significance
														// of the words in bow about a text (spec)
void bow_get_signWords(BoWords*, MBH*);					// Pass words to a min binary heap, based on their idf value
void bow_set_significance(BoWords*, char*);				// Mark a given word as significant in bow
void bow_keep_signWords(BoWords*);						// Remove from bow all insignificant words

// ~~~~~~~~~~~~~~~~~~~~ BUCKET ~~~~~~~~~~~~~~~~~~~~

Bucket* bow_bucket_create(int);
void bow_bucket_add(Bucket*, char*, mySpec*);			// Add a word about a text (spec) to a bucket
void bow_bucket_destroy(BoWords*, Bucket*);
void bow_bucket_print(Bucket*);							// Print all buckets of the same cell
Record* bow_search_bucket(Bucket*, char*);				// Find record with a key-word in a bucket
void bow_bucket_vectorize(Bucket*, float**, int*, mySpec*);	// Get a vector that corresponds to the significance
														// of the words in a bucket about a text (spec)
void bow_bucket_signWords(Bucket*, MBH*);				// Pass words to a min binary heap, based on their idf value
void bow_bucket_keep_signWords(BoWords*, Bucket*);		// Remove all insignificant words from a bucket

// ~~~~~~~~~~~~~~~~~~ RECORD ~~~~~~~~~~~~~~~~~~~~~~

Record* bow_record_create(char*, mySpec*);
void bow_record_destroy(BoWords*, Record*);				// Free all records in a bucket
void bow_record_print(Record*);
void bow_record_update(Record*, mySpec*);				// Update an already initialized record about another text
void bow_record_vectorize(Record*, float**, int*, mySpec*);	// Get a vector that corresponds to the significance
														// of a word about a text (spec)


// ~~~~~~~~~~~~~~~~~~~ TF-IDF ~~~~~~~~~~~~~~~~~~~~~


tfidf* tfidf_create();
void tfidf_set(tfidf*, int, int ); // (model, maxTexts, maxWords to scan)
void tfidf_destroy(tfidf*);
void tfidf_apply(tfidf*, BoWords*);
void tfidf_apply_toBucket(tfidf*, Bucket*);
void tfidf_apply_toRec(tfidf*, Record*);

float idf_calc(int, int);
			// > sum of all text's
			// > no. of texts that include the word
float tfidf_calc(int, int, float);
			// > no. of apperiences in specific text
			// > no. of specific text's word's sum
			// > idf


#endif