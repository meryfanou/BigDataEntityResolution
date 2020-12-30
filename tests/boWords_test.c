/* boWords_test.c */

#define TEST_NO_MAIN

#include <stdlib.h>
#include <string.h>
#include "../include/acutest.h"
#include "../include/boWords.h"
#include "../include/myHash.h"

void test_create_boW(void){

	BoWords* bow = bow_create(5, 5);
	Bucket* buc = bow_bucket_create(5);
	Record* rec = bow_record_create("key", NULL);


			// TEST CREATION
	TEST_ASSERT(bow != NULL);
	TEST_ASSERT(buc != NULL);
	TEST_ASSERT(rec != NULL);

			// TEST SIZE
	bow_add(bow, "key", NULL, -1);
	TEST_ASSERT(bow->entries == 1);
	TEST_ASSERT(buc->cur == 0);

	bow_destroy(bow);
	bow_bucket_destroy(NULL, buc);
	bow_record_destroy(NULL, rec);
}

void test_add_boW(void){

	int bowSize = 1;
	int bucSize = 100;

	BoWords* bow = bow_create(bowSize, bucSize);


	// ~~~~~~ CHECK INSERTION AT SAME BUCKET - TEST INSERT ROW + BUCKETS SIZE

		// Create a table with 100 specs
	int N = 100;
	mySpec** array = malloc(N*sizeof(mySpec*));
	char** words = malloc(N*sizeof(char*));

		// Insert specs using distinct keys
	int i = 0;
	while(i < N){
		char to_add[100];
		sprintf(to_add, "%d", i);
		
		char* mpla = malloc(4+strlen(to_add)+1);
		memset(mpla, 0, 4+strlen(to_add)+1);		
		strcat(mpla, "mpla");
		strcat(mpla, to_add);

		words[i] = malloc(4+strlen(to_add)+1);
		memset(words[i], 0, 4+strlen(to_add)+1);		
		strcat(words[i], "word");
		strcat(words[i], to_add);

		array[i] = specCreate(mpla, NULL, 0);

		bow_add(bow, words[i], array[i], hash1(mpla));
		TEST_ASSERT(bow->entries == i+1);
		i++;
	
		free(mpla);
	}
		// find target buc (only 1 buc exists)
	int target_buc1 = 0;

		// test all sub-bucs for records
	i = 0;
	Bucket* buc1 = bow->myTable[target_buc1];
	while(buc1 != NULL){
		if(N % bow->maxRecs != 0){
			if(buc1->next == NULL){
				TEST_ASSERT(buc1->cur == (N % bow->maxRecs));
			}
		}
		else{
			TEST_ASSERT(buc1->cur == bow->maxRecs);
		}

		Record* rec1 = buc1->rec;
		while(rec1 != NULL){
            TEST_ASSERT(strcmp(rec1->word, words[i]) == 0);
            TEST_ASSERT(rec1->texts != NULL);
			TEST_ASSERT(rec1->texts[0].text == array[i]);
            TEST_ASSERT(rec1->numofTexts == 1);
			i++;
			rec1 = rec1->next;
		}

		buc1 = buc1->next;
	}

	bow_destroy(bow);		// Array is now Empty !
	free(array);
    for(i=0; i<N; i++)
        free(words[i]);
    free(words);
}

  	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
void test_search_boW(void){
	int bowSize = 1;
	int bucSize = 100;

	BoWords* bow = bow_create(bowSize, bucSize);

	// ~~~~~~ CHECK SEARCHING HASH FOR KEY, by key

		// Create a table with 100 specs
	int N = 100;
	mySpec** array = malloc(N*sizeof(mySpec*));

		// Insert specs using distinct keys
	int i = 0;
	while(i < N){
        char word[] = "word";
		char to_add[100];
		sprintf(to_add, "%d", i);
		
		char* mpla = malloc(4+strlen(to_add)+1);
		memset(mpla, 0, 4+strlen(to_add)+1);
		
		strcat(mpla, "mpla");
		strcat(mpla, to_add);

		array[i] = specCreate(mpla, NULL, 0);

		bow_add(bow, word, array[i], hash1(mpla));
		TEST_ASSERT(bow->entries == i+1);
		i++;
	
		free(mpla);
	}

	i = 0;
	while(i < N){
		mySpec* spec = findRecord_byKey(hash, array[i]->specID);
		TEST_ASSERT(spec == array[i]);
		i++;
	}

	hash_destroy(hash);
	free(array);

}*/

void test_delete_boW(void){

	int bowSize = 2;
	int bucSize = 100;

	BoWords* bow = bow_create(bowSize, bucSize);

	int N = 100;
	mySpec** array = malloc(N*sizeof(mySpec*));
	char** words = malloc(N*sizeof(char*));

		// Create specs using distinct keys
	int i = 0;
	while(i < N){
		char to_add[100];
		sprintf(to_add, "%d", i);
		
		char* mpla = malloc(4+strlen(to_add)+1);
		memset(mpla, 0, 4+strlen(to_add)+1);	
		strcat(mpla, "mpla");
		strcat(mpla, to_add);

		words[i] = malloc(4+strlen(to_add)+1);
		memset(words[i], 0, 4+strlen(to_add)+1);		
		strcat(words[i], "word");
		strcat(words[i], to_add);

		array[i] = specCreate(mpla, NULL, 0);
		
		bow_add(bow, words[i], array[i], hash1(array[i]->specID));

		i++;
		free(mpla);
	}

	Bucket* buc = NULL;
	Record* rec = NULL;
	Bucket* temp_buc = NULL;
	Record* temp_rec = NULL;

	buc = bow->myTable[0];
	while(buc->next != NULL){
		if(buc->next->next == NULL)
			temp_buc = buc;
		buc = buc->next;
	}
	rec = buc->rec;
	while(rec->next != NULL){
		if(rec->next->next == NULL)
			temp_rec = rec;
		rec = rec->next;
	}

	// ~~~~~~ TEST DESTROYING LAST RECORD FROM BUCK 0
	bow_record_destroy(bow, rec);
	temp_rec->next = NULL;
	TEST_ASSERT(bow->entries == N-1);

	// ~~~~~~ TEST DESTROYING A FULL BUCKET
	int keep = buc->cur;
	bow_bucket_destroy(bow, buc);
	temp_buc->next = NULL;
	TEST_ASSERT(bow->entries == N-keep);


	// ~~~~~~ TEST DESTROYING FULL BUCKET CHAIN at buc[1]
	bow_bucket_destroy(bow, bow->myTable[1]);
	bow->myTable[1] = NULL;
	TEST_ASSERT(bow->entries < N-keep);

	bow_destroy(bow);
	free(array);
    for(i=0; i<N; i++)
        free(words[i]);
    free(words);
}