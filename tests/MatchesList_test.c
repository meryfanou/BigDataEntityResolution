/*MatchesList_test.c*/

#include "../include/myMatches.h"
#include "../include/myuTesting.h"

void test_create(void){

	matchesInfo* list = matchesInfoInit();
	matchesAdd(list, NULL);
	deleteMatches(list->head);

	if(myValidCheck(list != NULL, "test_create_1") == -1)
		return;
	if(myValidCheck(list->entries == 0, "test_create_2") == -1)
		return;

	deleteInfo(list);

}

int main(void){
	to_test myTests[] = {
		{"test_create", test_create},
		{"SKATA", test_create},
		{NULL, NULL}
	};

	runTests(myTests);
	return 0;
}
