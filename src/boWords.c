#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/boWords.h"
#include "../include/myHash.h"
#include "../include/mbh.h"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~ BAG OF WORDS ~~~~~~~~~~~~~~~~~~~~~~~~~

BoWords* bow_create(int HashSize, int BucSize){		// Allocate and return hash table

	BoWords* newTable = malloc(sizeof(BoWords));

	newTable->myTable = malloc(HashSize*sizeof(Bucket*));

	for(int i=0; i < HashSize; i++){
		newTable->myTable[i] = NULL;
	}

	newTable->bucSize = BucSize;
	newTable->tableSize = HashSize;

	newTable->maxRecs = (BucSize - sizeof(Bucket*)) / sizeof(Record);
	newTable->entries = 0;
	
	newTable->specsSum = 0;

	return newTable;
}

void bow_destroy(BoWords* table){	// Destroy hash, free memory

	for(int i=0; i < table->tableSize; i++){
		// If bucket is not empty
		if(table->myTable[i] != NULL){
			bow_bucket_destroy(table, table->myTable[i]);
			table->myTable[i] = NULL;
		}
	}

	free(table->myTable);
	free(table);
}

// Add a word about a text (spec) to bow
void bow_add(BoWords* table, char* word, mySpec* text, int hash){

	if(word == NULL)
		return;

	// Find hash table cell
	int cell = hash % (table->tableSize);
	if(cell < 0)
		cell = 0;

	// Case of first backet
	if(table->myTable[cell] == NULL){
		table->myTable[cell] = bow_bucket_create(table->bucSize);
	}

	Bucket* tempBuc = table->myTable[cell];
	int flag = 0;		// 0 ~ new key / 1 ~ existing key
	Record* existingRec = NULL;
	if((existingRec = bow_search_bucket(tempBuc, word)) != NULL)
		flag = 1;

	while(tempBuc->next != NULL && flag == 0){	// Search for rec in buckets of the same cell
		tempBuc = tempBuc->next;
		if((existingRec = bow_search_bucket(tempBuc, word)) != NULL){
			flag = 1;
			break;
		}
	}

	if(flag == 1){
		// Update word's number of appearances in current text
		bow_record_update(existingRec, text);
	}
	else{
		// If the last bucket is full
		if(tempBuc->cur >= table->maxRecs){
			// Create a new one
			tempBuc->next = bow_bucket_create(table->bucSize);
			tempBuc = tempBuc->next;
		}

		// Add record to bucket
		bow_bucket_add(tempBuc, word, text);
		table->entries++;
	}

}

void bow_print(BoWords* bow){
	printf("HASH_SIZE: %d\n", bow->tableSize);

	for(int i=0; i < bow->tableSize; i++){
		if(bow->myTable[i] != NULL){
			printf("cell [%d]:\n", i);
			bow_bucket_print(bow->myTable[i]);
		}
	}
	printf("\n");
}

// Get a vector (1xn) that corresponds to the significance of the words in bow about a text (spec)
void bow_vectorize(BoWords* bow, float** vector, int* vectorSize, mySpec* spec){
	Bucket*	bucket = NULL;

	for(int i=0; i<(bow->tableSize); i++){
		bucket = bow->myTable[i];
		while(bucket != NULL){
			bow_bucket_vectorize(bucket, vector, vectorSize, spec);
			bucket = bucket->next;
		}
	}
}

// Pass words to a min binary heap, based on their (maximum) idf value
void bow_get_signWords(BoWords* bow, MBH* heap){
	Bucket*	bucket = NULL;

	for(int i=0; i<(bow->tableSize); i++){
		bucket = bow->myTable[i];
		while(bucket != NULL){
			bow_bucket_signWords(bucket, heap);
			bucket = bucket->next;
		}
	}
}

// Mark a given word as significant in bow
void bow_set_significance(BoWords* bow, char* word){
	if(word == NULL)
		return;

	int	hash = hash1(word);

	// Find hash table cell
	int cell = hash % (bow->tableSize);
	if(cell < 0)
		cell = 0;

	Bucket*	bucket = bow->myTable[cell];
	Record*	record = NULL;

	// Search for the record with the given word
	while(bucket != NULL){
		record = bow_search_bucket(bucket, word);
		// If found
		if(record != NULL)
			break;

		bucket = bucket->next;
	}

	// If the word does not exist in bow
	if(record == NULL)
		return;

	// Mark record as significant
	record->isSignificant = 1;
}

// Remove from bow all insignificant words
void bow_keep_signWords(BoWords* bow){
	Bucket*	bucket = NULL;
	Bucket*	next = NULL;

	// For each bucket in bow
	for(int i=0; i<(bow->tableSize); i++){
		bucket = bow->myTable[i];

		while(bucket != NULL){
			next = bucket->next;
			// Remove all insignificant words from a bucket
			bow_bucket_keep_signWords(bow, bucket);

			bucket = next;
		}
	}

}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ BUCKET ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Bucket* bow_bucket_create(int size){				// Init bucket
	Bucket* newBucket = malloc(sizeof(bucket));
	newBucket->next = NULL;
	newBucket->rec = NULL;
	newBucket->cur = 0;

	return newBucket;
}

// Add a word about a text (spec) to a bucket
void bow_bucket_add(Bucket* buc, char* word, mySpec* text){
	// If there bucket is empty, create a new record
	if(buc->rec == NULL){
		buc->rec = bow_record_create(word, text);
		buc->cur++;
		return;
	}

	// Otherwise create the new record and add it before the (so far) first record
	Record* tempRec = buc->rec;
	buc->rec = bow_record_create(word, text);
	buc->rec->next = tempRec;

	buc->cur++;

}

// Find record with a key-word in a bucket (if it exists)
Record* bow_search_bucket(Bucket* bucket, char* key){
	if(bucket == NULL)
		return NULL;

	Record* temp = bucket->rec;
	while(temp != NULL){
		if(strcmp(temp->word, key) == 0)
			return temp;
		temp = temp->next;
	}

	return NULL;
}

// Print all buckets of the same cell
void bow_bucket_print(Bucket* b){
	if(b->cur != 0)
		printf("\t>buc: (%d)\n", b->cur);
	Record* temp = b->rec;
	while(temp != NULL){
		bow_record_print(temp);
		temp = temp->next;
	}
	if(b->next != NULL){
		bow_bucket_print(b->next);
	}
}

// Free a bucket
void bow_bucket_destroy(BoWords* hash, Bucket* buc){
	if(buc->next != NULL){
		bow_bucket_destroy(hash, buc->next);
		buc->next = NULL;
	}

	if(buc->rec != NULL){
		bow_record_destroy(hash, buc->rec);
		buc->rec = NULL;
	}

	free(buc);
	buc = NULL;
}

// Get a vector that corresponds to the significance of the words in a bucket about a text (spec)
void bow_bucket_vectorize(Bucket* bucket, float** vector, int* vectorSize, mySpec* spec){
	Record*	record = NULL;

	record = bucket->rec;
	while(record != NULL){
		bow_record_vectorize(record, vector, vectorSize, spec);
		record = record->next;
	}
}

// Pass words to a min binary heap, based on their (maximum) idf value
void bow_bucket_signWords(Bucket* bucket, MBH* heap){
	Record*	record = bucket->rec;

	while(record != NULL){
		mbh_insert(heap, record->word, record->idf);
		record = record->next;
	}
}

// Remove all insignificant words from a bucket
void bow_bucket_keep_signWords(BoWords* bow, Bucket* bucket){
	Record*	record = bucket->rec;
	Record	*prev = NULL, *next = NULL;

	// For each record
	while(record != NULL){
		// Keep its next
		next = record->next;

		// If current record is not significant, it should be removed
		if(record->isSignificant == 0){
			free(record->word);
			if(record->texts != NULL)
				free(record->texts);
			free(record);

			(bucket->cur)--;
			(bow->entries)--;
			// If it is bucket's first record
			if(prev == NULL)
				bucket->rec = next;
			else
				prev->next = next;
		}
		else{
			// Keep current record for the next loop
			prev = record;
		}

		record = next;
	}
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ RECORD ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Record* bow_record_create(char* word, mySpec* text){	// Init record
	Record* newRec = malloc(sizeof(Record));

	newRec->next = NULL;
	newRec->word = strdup(word);
	newRec->isSignificant = 0;
	newRec->idf = 0.0;
	newRec->texts = malloc(sizeof(TextInfo));
	newRec->texts[0].text = text;
	newRec->texts[0].tf_idf = 1.0;
	newRec->numofTexts = 1;

	return newRec;
}

void bow_record_print(Record* rec){						// Print record
	printf("\t\t>word %s has idf = %.4f\n", rec->word, rec->idf);

	for(int i=0; i<(rec->numofTexts); i++){
		printf("\t\t\t>has tf-idf = %.4f for text %s\n",rec->texts[i].tf_idf,rec->texts[i].text->specID);
	}
}

// Free all records in a bucket
void bow_record_destroy(BoWords* boWords, Record* rec){
	if(rec->next != NULL){
		bow_record_destroy(boWords, rec->next);
		rec->next = NULL;
	}

	free(rec->word);
	if(rec->texts != NULL)
		free(rec->texts);
	if(boWords != NULL)
		boWords->entries--;

	free(rec);
	rec = NULL;
}

// Update an already initialized record about another text (spec)
void bow_record_update(Record* rec, mySpec* text){
	if(rec == NULL)
		return;

	int	i = 0;

	for(; i<(rec->numofTexts); i++){
		// If the word has already appeared in current text
		if(rec->texts[i].text == text){
			rec->texts[i].tf_idf += 1.0;
			break;
		}
	}

	// If the word appears in current text for the first time
	if(i == rec->numofTexts){
		(rec->numofTexts)++;
		rec->texts = realloc(rec->texts, (rec->numofTexts)*sizeof(TextInfo));
		rec->texts[i].text = text;
		rec->texts[i].tf_idf = 1.0;
	}
}

// Get a vector that corresponds to the significance of a word about a text (spec)
void bow_record_vectorize(Record* record, float** vector, int* vectorSize, mySpec* spec){
	int i = 0;

	for(; i<(record->numofTexts); i++){
		// If the given text is found in record
		if(record->texts[i].text == spec){
			(*vector)[*vectorSize] = record->texts[i].tf_idf;
			(*vectorSize)++;
			break;
		}
	}

	// If the word does not appear in the given text
	if(i == record->numofTexts){
		(*vector)[*vectorSize] = 0.0;
		(*vectorSize)++;
	}
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ TF-IDF ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

tfidf* tfidf_create(){
	tfidf* newtf = malloc(sizeof(tfidf));

	newtf->all_Texts = -1;
	newtf->all_Words = -1;
	newtf->max_Texts = -1;
	newtf->max_Words = -1;
	newtf->count_Texts = 0;
	newtf->count_Words = 0;

	return newtf;
}

void tfidf_destroy(tfidf* tf){
	free(tf);
}

void tfidf_set(tfidf* tf, int maxTexts, int maxWords){
	if(maxTexts > -1)
		tf->max_Texts = maxTexts;
	if(maxWords > -1)
		tf->max_Words = maxWords;
}

void tfidf_apply(tfidf* tf, BoWords* bow){
	tf->all_Words = bow->entries;
	tf->all_Texts = bow->specsSum;

	int i = 0;
	while(i < bow->tableSize){
		Bucket* tempBuc = bow->myTable[i];
		while(tempBuc != NULL){
			tfidf_apply_toBucket(tf, tempBuc);

			tempBuc = tempBuc->next;
		}
		i++;
	}
}

void tfidf_apply_toBucket(tfidf* tf, Bucket* buc){
	Record* tempRec = buc->rec;
	while(tempRec != NULL){
		tfidf_apply_toRec(tf, tempRec);

		if(tf->count_Words == tf->max_Words)
			return;

		tempRec = tempRec->next;
	}
}

void tfidf_apply_toRec(tfidf* tf, Record* rec){
	int texts_sum = tf->all_Texts;
	int target_texts_sum = rec->numofTexts;

	rec->idf = idf_calc(texts_sum, target_texts_sum);

	TextInfo* tempText = NULL;
	int i = 0;
	while(i < rec->numofTexts){
		tempText = &rec->texts[i];

		int counts_inText = tempText->tf_idf;
		int words_sum = tempText->text->numofWords;

		tempText->tf_idf = tfidf_calc(counts_inText, words_sum, rec->idf);

		i++;
	}
}

float idf_calc(int texts_sum, int target_texts_sum){
	return log((float)texts_sum / (float)target_texts_sum);
}

float tfidf_calc(int counts_inText, int words_sum, float idf){

	float num = (float)counts_inText / (float)words_sum;

	/*
		!!!! TEST PRINTS !!!
	printf("counts_inText: %d, words_sum: %d", counts_inText, words_sum);
	printf(", texts_sum: %d, target_texts_sum: %d\n", texts_sum, target_texts_sum);
	printf("num_1: %.4f, num_2: %.4f, final: %.4f\n", num, idf, num*idf);
	
	*/

	return num*idf;
}