/* Hash_test.c */

#include <stdlib.h>
#include "../include/myMatches.h"
#include "../include/mySpec.h"
#include "../include/myHash.h"
#include "../include/acutest.h"
#include "../include/logistic.h"
#include "../include/tests.h"
#include "../include/functs.h"
#include "../include/boWords.h"
#include "../include/mbh.h"

void test_createHash(void){

	hashTable* hash = hash_create(5, 5);
	bucket* buc = bucket_create(5);
	record* rec = record_create(NULL);


			// TEST CREATION
	TEST_ASSERT(hash != NULL);
	TEST_ASSERT(buc != NULL);
	TEST_ASSERT(rec != NULL);

			// TEST SIZE
	hash_add(hash, NULL, -1);
	TEST_ASSERT(hash->entries == 0);
	TEST_ASSERT(buc->cur == 0);

	hash_destroy(hash);
	bucket_destroy(NULL, buc);
	record_destroy(NULL, rec);
}

void test_addHash(void){

	int hashSize = 1;
	int bucSize = 100;

	hashTable* hash = hash_create(hashSize, bucSize);


	// ~~~~~~ CHECK INSERTION AT SAME BUCKET - TEST INSERT ROW + BUCKETS SIZE

		// Create a table with 100 specs
	int N = 100;
	mySpec** array = malloc(N*sizeof(mySpec*));

		// Insert specs using distinct keys
	int i = 0;
	while(i < N){
		char to_add[100];
		sprintf(to_add, "%d", i);
		
		char* mpla = malloc(4+strlen(to_add)+1);
		memset(mpla, 0, 4+strlen(to_add)+1);
		
		strcat(mpla, "mpla");
		strcat(mpla, to_add);

		array[i] = specCreate(mpla, NULL, 0);

		hash_add(hash, array[i], hash1(mpla));
		TEST_ASSERT(hash->entries == i+1);
		i++;
	
		free(mpla);
	}
		// find target buc (only 1 buc exists)
	int target_buc1 = 0;

		// test all sub-bucs for records
	i = 0;
	bucket* buc1 = hash->myTable[target_buc1];
	while(buc1 != NULL){
		if(N % hash->maxRecs != 0){
			if(buc1->next == NULL){
				TEST_ASSERT(buc1->cur == (N % hash->maxRecs));
			}
		}
		else{
			TEST_ASSERT(buc1->cur == hash->maxRecs);
		}

		record* rec1 = buc1->rec;
		while(rec1 != NULL){
			TEST_ASSERT(rec1->spec == array[i]);
			i++;
			rec1 = rec1->next;
		}

		buc1 = buc1->next;
	}

	hash_destroy(hash);		// Array is now Empty !
	free(array);
}

  	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void test_searchHash(void){
	int hashSize = 1;
	int bucSize = 100;

	hashTable* hash = hash_create(hashSize, bucSize);

	// ~~~~~~ CHECK SEARCHING HASH FOR KEY, by key

		// Create a table with 100 specs
	int N = 100;
	mySpec** array = malloc(N*sizeof(mySpec*));

		// Insert specs using distinct keys
	int i = 0;
	while(i < N){
		char to_add[100];
		sprintf(to_add, "%d", i);
		
		char* mpla = malloc(4+strlen(to_add)+1);
		memset(mpla, 0, 4+strlen(to_add)+1);
		
		strcat(mpla, "mpla");
		strcat(mpla, to_add);

		array[i] = specCreate(mpla, NULL, 0);

		hash_add(hash, array[i], hash1(mpla));
		TEST_ASSERT(hash->entries == i+1);
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

}

void test_deleteHash(void){

	int hashSize = 2;
	int bucSize = 100;

	hashTable* hash = hash_create(hashSize, bucSize);

	int N = 100;
	mySpec** array = malloc(N*sizeof(mySpec*));

		// Create specs using distinct keys
	int i = 0;
	while(i < N){
		char to_add[100];
		sprintf(to_add, "%d", i);
		
		char* mpla = malloc(4+strlen(to_add)+1);
		memset(mpla, 0, 4+strlen(to_add)+1);
		
		strcat(mpla, "mpla");
		strcat(mpla, to_add);

		array[i] = specCreate(mpla, NULL, 0);
		
		hash_add(hash, array[i], hash1(array[i]->specID));

		i++;
		free(mpla);
	}

	bucket* buc = NULL;
	record* rec = NULL;
	bucket* temp_buc = NULL;
	record* temp_rec = NULL;



	buc = hash->myTable[0];
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
	record_destroy(hash, rec);
	temp_rec->next = NULL;
	TEST_ASSERT(hash->entries == N-1);

	// ~~~~~~ TEST DESTROYING A FULL BUCKET
	int keep = buc->cur;
	bucket_destroy(hash, buc);
	temp_buc->next = NULL;
	TEST_ASSERT(hash->entries == N-keep);


	// ~~~~~~ TEST DESTROYING FULL BUCKET CHAIN at buc[1]
	bucket_destroy(hash, hash->myTable[1]);
	hash->myTable[1] = NULL;
	TEST_ASSERT(hash->entries < N-keep);

	hash_destroy(hash);
	free(array);
}


TEST_LIST = {
	{"HASH .1: test_createHash", test_createHash},
	{"HASH .2: test_addHash", test_addHash},
	{"HASH .3: test_deleteHash", test_deleteHash},
	{"HASH .4: test_searchHash", test_searchHash},
	{"MATCHES .1: test_createMatches", test_createMatches},
	{"MATCHES .2: test_pushMatches", test_pushMatches},
	{"MATCHES .3: test_insertMatches", test_insertMatches},
	{"MATCHES .4: test_deleteMatches", test_deleteMatches},
	{"MATCHES .5: test_mergeMatches", test_mergeMatches},
	{"MATCHES_NEGS .1: test_createNegs", test_createNegs},
	{"MATCHES_NEGS .2: test_addNegs", test_addNegs},
	{"MATCHES_NEGS .3: test_remove_nlist", test_remove_nlist},
	{"MATCHES_NEGS .4: test_seek_nlist", test_seek_nlist},
	{"SPEC .1: test_specInit", test_specInit},
	{"SPEC .2: test_specCreate", test_specCreate},
    {"SPEC .3: test_updateSpecMatches", test_updateSpecMatches},
	{"DATA_LIST .1: test_create_dataList", test_create_dataList},
	{"DATA_LIST .2: test_push_dataList", test_push_dataList},
	{"DATA_LIST .3: test_delete_dataList", test_delete_dataList},
	{"DATA_LIST .4: test_pop_dataList", test_pop_dataList},
	{"LOGISTIC .1: test_logistic_create", test_logistic_create},
	{"LOGISTIC .2: test_logistic_fit_dataList", test_logistic_fit_dataList},
	{"BAG_OF_WORDS .1: test_create_boW", test_create_boW},
	{"BAG_OF_WORDS .2: test_add_boW", test_add_boW},
	{"BAG_OF_WORDS .3: test_delete_boW", test_delete_boW},
	{"MBH .1: test_mbh_create", test_mbh_create},
	{"MBH .2: test_mbh_insert & extract_root", test_mbh_insert_extract_root},
	{NULL, NULL}
};
