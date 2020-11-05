/*MatchesList_test.c*/

#include <stdlib.h>
#include "../include/myMatches.h"
#include "../include/mySpec.h"
#include "../include/myuTesting.h"

void test_createMatches(void){

	matchesInfo* list = matchesInfoInit();
	matchesAdd(list, NULL);
	deleteMatches(list, list->head);

			// TEST CREATION
	if(myValidCheck(list != NULL, "test_create_1") == -1)
		return;
			// TEST SIZE
	if(myValidCheck(list->entries == 0, "test_create_2") == -1)
		return;

	deleteInfo(list);

}

void test_insertMatches(void){

	matchesInfo* list = matchesInfoInit();

	myMatches* testMatches;

	int flag = 0;

	int N = 100;
	mySpec* array = malloc(N*sizeof(mySpec));

	int i = 0;
	while(i < N){
		testMatches = matchesAdd(list, &array[i]);
						// TEST SIZE
		if(myValidCheck(list->entries == (i+1), "test_insert_1")){
			flag = 1;
			break;
		}
						// TEST INSERTION
		if(myValidCheck(list->head->specsTable[0] == &array[i], "test_insert_2")){
			flag = 1;
			break;
		}

		i++;
	}

						// TEST LIFO
	if(flag == 0){
		i = N - 1;
		while(i >= 0){
			if(myValidCheck(testMatches->specsTable[0] == &array[i], "test_inser_3"))
				break;
			testMatches = testMatches->prev;
			i--;
		}
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
	i = N - 1;
	while(i >= 0){
		if(myValidCheck(list->head->specsTable[0] == &array[i], "test_deleteMatches_1"))
			break;

		deleteMatches(list, list->head);
		
		if(myValidCheck(list->entries == (i-1), "test_deleteMatches_2"))
			break;
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
	myValidCheck(list->entries == (i-2), "test_deleteMatches_3");

	free(array);
	deleteInfo(list);

}

int main(void){
	to_test myTests[] = {
		{"test_createMatches", test_createMatches},
		{"test_insertMatches", test_insertMatches},
		{"test_deleteMatches", test_deleteMatches},
		{NULL, NULL}
	};

	runTests(myTests);
	return 0;
}