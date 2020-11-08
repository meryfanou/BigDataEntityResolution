/* Hash_test.c */

/*MatchesList_test.c*/

#include <stdlib.h>
#include "../include/myMatches.h"
#include "../include/mySpec.h"
#include "../include/myHash.h"
#include "../include/acutest.h"

void test_createHash(void){

	hashTable* hash = hash_create(5, 5);
	bucket* buc = bucket_create(5);
	record* rec = record_create(NULL);

	// deleteMatches(list, list->head);

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

// void test_deleteHash(void){

// 	int hashSize = 2;
// 	int bucSize = 100;

// 	hashTable* hash = hash_create(hashSize, bucSize);

// 	// ~~~~~~ CHECK SEARCHING HASH FOR KEY, by key

// 	mySpec* spec1 = specCreate("mple", NULL, 0);
// 	mySpec* spec2 = specCreate("mple1", NULL, 0);

// 	hash_add(hash, spec1);
// 	hash_add(hash, spec1);

// 	bucket* buc = NULL;
// 	record* rec = NULL;

// 	// ~~~~~~ TEST DESTROYING LAST RECORD FROM BUCK 0

// 	buc = hash->myTable[0];
// 	while(buc->next != NULL){
// 		buc = buc->next;
// 	}
// 	rec = buc->rec;
// 	while(rec->next != NULL){
// 		rec = rec->next;
// 	}
// 	record_destroy(hash, rec);
// 	TEST_ASSERT(hash->entries == N-1);

// 	// ~~~~~~ TEST DESTROYING A FULL BUCKET
// 	int keep = buc->cur-1;
// 	bucket_destroy(hash, buc);
// 	TEST_ASSERT(hash->entries == N-keep);



// 	hash_destroy(hash);
// 	free(array);

// }


TEST_LIST = {
	{"test_createHash", test_createHash},
	{"test_addHash", test_addHash},
	// {"test_deleteHash", test_deleteHash},
	{"test_searchHash", test_searchHash},
	// {"test_mergeMatches", test_mergeMatches},
	{NULL, NULL}
};
