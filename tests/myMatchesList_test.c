/*MatchesList_test.c*/

#include <stdlib.h>
#include "../include/myMatches.h"
#include "../include/mySpec.h"
#include "../include/acutest.h"

void test_createMatches(void){

	matchesInfo* list = matchesInfoInit();
	myMatches* node = myMatchesInit();

	deleteMatches(list, list->head);

			// TEST CREATION
	TEST_ASSERT(list != NULL);
	TEST_ASSERT(node != NULL);
			// TEST SIZE
	TEST_ASSERT(list->entries == 0);

	deleteMatches(list, node);
	deleteInfo(list);

}

void test_pushMatches(void){

	myMatches* node = myMatchesInit();
	TEST_ASSERT(node->specsCount == 0);

	int N = 5;
	mySpec* array = malloc(N*sizeof(mySpec));

	int i = 0;
	while(i < N){
		// printf("mpla\n");
		pushMatch(node, &array[i]);
		TEST_ASSERT(node->specsTable != NULL);
		TEST_ASSERT(node->specsTable[i] == &array[i]);
		TEST_ASSERT(node->specsCount == i+1);
		i++;
	}

	deleteMatches(NULL, node);

}

void test_insertMatches(void){

	matchesInfo* list = matchesInfoInit();

	myMatches* testMatches;

	int N = 100;
	mySpec* array = malloc(N*sizeof(mySpec));

	int i = 0;
	while(i < N){
		testMatches = matchesAdd(list, &array[i]);

						// TEST SIZE
		TEST_ASSERT(list->entries == (i+1));
						// TEST INSERTION
		TEST_ASSERT(list->head->specsTable[0] == &array[i]);

		i++;
	}
						// TEST LIFO
	testMatches = list->head;
	i = N-1;
	while(i >= 0){
		TEST_ASSERT(testMatches->specsTable[0] == &array[i]);

		testMatches = testMatches->next;
		i--;
	}


	free(array);
	deleteInfo(list);

}

void test_deleteMatches(void){

	matchesInfo* list = matchesInfoInit();

	myMatches* testMatches;

	int N = 100;
	mySpec* array = malloc(N*sizeof(mySpec));

			// FILL STRUCT (ALREADY CHECKED INSERT WORKS)
	int i = 0;
	while(i < N){
		testMatches = matchesAdd(list, &array[i]);
		i++;
	}

			// TEST DELETION FROM START
	i = N-1;
	while(i >= 0){
		TEST_ASSERT(list->head->specsTable[0] == &array[i]);
		deleteMatches(list, list->head);
		
		TEST_ASSERT(list->entries == i);

		i--;
	}

		// REFILL STRUCT
	i = 0;
	while(i < N){
		testMatches = matchesAdd(list, &array[i]);
		i++;
	}

			// TEST DELETION FROM SECOND NODE
	testMatches = list->head->next;
	deleteMatches(list, testMatches);
	TEST_ASSERT(list->entries == (i-1));

	free(array);
	deleteInfo(list);

}

void test_mergeMatches(void){
	// matchesInfo* list;
	myMatches* to_merge1 = myMatchesInit();
	myMatches* to_merge2 = myMatchesInit();

	int N = 100;
	mySpec* specArray = malloc(N*sizeof(mySpec));


					// SPLIT SPECS ARRAY TO MATCHES_TO_MERGE
	int i = 0;
	while(i < 100){
		if(i < 50)
			pushMatch(to_merge1, &specArray[i]);
		else
			pushMatch(to_merge2, &specArray[i]);
		i++;
	}


				// CHECK ENTRIES BEFORE
	TEST_ASSERT(to_merge1->specsCount == 50);
	TEST_ASSERT(to_merge2->specsCount == 50);
	
	mergeMatches(NULL, to_merge1, to_merge2);

				// CHECK POINTERS
	TEST_ASSERT(to_merge1 != NULL);
	
	// printf("counts2: %d\n", to_merge2->specsCount);
	// free(to_merge2);

				// CHECK ENTRIES AFTER
	TEST_ASSERT(to_merge1->specsCount == N);

				// CHECK INPUT LINE
	i = 0;
	while(i < N){
		TEST_ASSERT(to_merge1->specsTable[i] == &specArray[i]);
		i++;
	}


	free(specArray);
	deleteMatches(NULL, to_merge1);
	// deleteMatches(NULL, to_merge2);
}

TEST_LIST = {
	{"test_createMatches", test_createMatches},
	{"test_pushMatches", test_pushMatches},
	{"test_insertMatches", test_insertMatches},
	{"test_deleteMatches", test_deleteMatches},
	{"test_mergeMatches", test_mergeMatches},
	{NULL, NULL}
};
