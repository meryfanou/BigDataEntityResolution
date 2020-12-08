#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/boWords.h"
#include "../include/myHash.h"


BoWords* bow_create(int HashSize, int BucSize){
	// ALLOCARE AND RETURN NEW HASH_TABLE

	BoWords* newTable = malloc(sizeof(BoWords));

	newTable->myTable = malloc(HashSize*sizeof(Bucket*));
	int i = 0;
	while(i < HashSize){
		newTable->myTable[i] = NULL;
		i++;		
	}

	newTable->bucSize = BucSize;
	newTable->tableSize = HashSize;

	newTable->maxRecs = (BucSize - sizeof(Bucket*)) / sizeof(Record);
	newTable->entries = 0;
	
	newTable->specsSum = 0;

	return newTable;
}

void bow_destroy(BoWords* table){	// DESTROY HASH / FREE MEM
	int i = 0;
	while(i < table->tableSize){
		if(table->myTable[i] != NULL){
			bow_bucket_destroy(table, table->myTable[i]);
			table->myTable[i] = NULL;
		}
		i++;
	}

	free(table->myTable);
	free(table);

}

void bow_add(BoWords* table, char* word, mySpec* text, int hash){

	if(word == NULL)
		return;

	int cell = hash % (table->tableSize);		/// FIND HASH TABLE CELL
	if(cell < 0)
		cell = 0;

	if(table->myTable[cell] == NULL){			/// CASE OF FIRST BUCKET
		table->myTable[cell] = bow_bucket_create(table->bucSize);
	}

	Bucket* tempBuc = table->myTable[cell];
	int flag = 0;		// 0 ~ new key / 1 ~ existing key
	Record* existingRec = NULL;
	if((existingRec = bow_search_bucket(tempBuc, word)) != NULL)
		flag = 1;
	
	while(tempBuc->next != NULL && flag == 0){	/// FIND LAST BUCKET
		tempBuc = tempBuc->next;				/// + SEARCH FOR REC IN BUCS
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
		if(tempBuc->cur >= table->maxRecs){			/// CHECK IF ITS FULL
			tempBuc->next = bow_bucket_create(table->bucSize);
			tempBuc = tempBuc->next;
		}

		bow_bucket_add(tempBuc, word, text);					/// ADD AT BUCKET
		table->entries++;
	}

}

void bow_print(BoWords* t){			// testing funct - prints hash
	printf("HASH_SIZE: %d\n", t->tableSize);
	int i = 0;
	while(i < t->tableSize){
		if(t->myTable[i] != NULL){
			printf("cell [%d]:\n", i);
			bow_bucket_print(t->myTable[i]);
		}
		i++;
	}
	printf("\n");
}

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


Bucket* bow_bucket_create(int size){				/// INIT BUCKET
	Bucket* newBucket = malloc(sizeof(bucket));
	newBucket->next = NULL;
	newBucket->rec = NULL;
	newBucket->cur = 0;

	return newBucket;
}

void bow_bucket_add(Bucket* buc, char* word, mySpec* text){
	if(buc->rec == NULL){
		buc->rec = bow_record_create(word, text);
		buc->cur++;
		return;
	}

	Record* tempRec = buc->rec;					/// FIND LAST ENTRY
	while(tempRec->next != NULL){
		tempRec = tempRec->next;
	}

	tempRec->next = bow_record_create(word, text);				/// ADD RECORD
	buc->cur++;

}

Record* bow_search_bucket(Bucket* b, char* key){	// search bucket for target key
	if(b == NULL)
		return NULL;
	Record* temp = b->rec;

	while(temp != NULL){
		if(strcmp(temp->word, key) == 0)
			return temp;
		temp = temp->next;
	}

	return NULL;
}

void bow_bucket_print(Bucket* b){					// testing funct - prints bucket
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

void bow_bucket_destroy(BoWords* hash, Bucket* buc){		/// FREE BUC
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

void bow_bucket_vectorize(Bucket* bucket, float** vector, int* vectorSize, mySpec* spec){
	Record*	record = NULL;

	record = bucket->rec;
	while(record != NULL){
		bow_record_vectorize(record, vector, vectorSize, spec);
		record = record->next;
	}
}


Record* bow_record_create(char* word, mySpec* text){
	Record* newRec = malloc(sizeof(Record));

	newRec->next = NULL;
	newRec->word = strdup(word);
	newRec->texts = malloc(sizeof(TextInfo));
	newRec->texts[0].text = text;
	newRec->texts[0].numofInstances = 1.0;
	newRec->numofTexts = 1;

	return newRec;
}

void bow_record_print(Record* rec){
	printf("\t\t>word: %s\n", rec->word);

	for(int i=0; i<(rec->numofTexts); i++){
		printf("\t\t\t>appears %.4f times in %s\n",rec->texts[i].numofInstances,rec->texts[i].text->specID);
	}
}

void bow_record_destroy(BoWords* boWords, Record* rec){				/// FREE REC
	if(rec->next != NULL){
		bow_record_destroy(boWords, rec->next);
		rec->next = NULL;
	}

	free(rec->word);
	free(rec->texts);
	if(boWords != NULL)
		boWords->entries--;

	free(rec);
	rec = NULL;
}

void bow_record_update(Record* rec, mySpec* text){
	int	i = 0;

	for(; i<(rec->numofTexts); i++){
		// If the word has already appeared in current text
		if(rec->texts[i].text == text){
			rec->texts[i].numofInstances += 1.0;
			break;
		}
	}

	// If the word appears in current text for the first time
	if(i == rec->numofTexts){
		i = rec->numofTexts;

		(rec->numofTexts)++;
		rec->texts = realloc(rec->texts, (rec->numofTexts)*sizeof(TextInfo));
		rec->texts[i].text = text;
		rec->texts[i].numofInstances = 1.0;
	}
}

void bow_record_vectorize(Record* record, float** vector, int* vectorSize, mySpec* spec){
	int i = 0;

	for(; i<(record->numofTexts); i++){
		if(record->texts[i].text == spec){
			(*vector)[*vectorSize] = record->texts[i].numofInstances;
			(*vectorSize)++;
			break;
		}
	}

	if(i == record->numofTexts){
		(*vector)[*vectorSize] = 0;
		(*vectorSize)++;
	}
}


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

	TextInfo* tempText = NULL;
	int i = 0;
	while(i < rec->numofTexts){
		tempText = &rec->texts[i];

		int counts_inText = tempText->numofInstances;
		int words_sum = tempText->text->numofWords;
		
		tempText->numofInstances = tfidf_calc(counts_inText, words_sum, texts_sum, target_texts_sum);

		i++;
	}
}

float tfidf_calc(int counts_inText, int words_sum, int texts_sum, int target_texts_sum){

	float num1 = (float)counts_inText / (float)words_sum;

	float num2 = log((float)texts_sum / (float)target_texts_sum);

	/*
		!!!! TEST PRINTS !!!
	printf("counts_inText: %d, words_sum: %d", counts_inText, words_sum);
	printf(", texts_sum: %d, target_texts_sum: %d\n", texts_sum, target_texts_sum);
	printf("num_1: %.4f, num_2: %.4f, final: %.4f\n", num1, num2, num1*num2);
	
	*/

	return num1*num2;
}