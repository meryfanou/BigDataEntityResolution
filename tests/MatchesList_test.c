/*MatchesList_test.c*/

#include <stdlib.h>
#include "../include/myMatches.h"
#include "../include/mySpec.h"
#include "../include/myuTesting.h"

void test_createMatches(void){

	matchesInfo* list = matchesInfoInit();
	matchesAdd(list, NULL);
	deleteMatches(list->head);

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
			myValidCheck(testMatches->specsTable[0] == &array[i], "test_inser_3");
			testMatches = testMatches->prev;
			i--;
		}
	}


	free(array);
	deleteInfo(list);

}

int main(void){
	to_test myTests[] = {
		{"test_create", test_createMatches},
		{"test_insert", test_insertMatches},
		{NULL, NULL}
	};

	runTests(myTests);
	return 0;
}