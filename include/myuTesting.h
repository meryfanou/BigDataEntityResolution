/* myuTesting.h */
#include <stdio.h>
#include <string.h>

int EXIT = 0;

typedef struct to_test to_test;

struct to_test{
	char* name;
	void (*funct)();
};

int myValidCheck(int res, char* name){
	if(res != 1){
		printf("\t%s ~ FAILED !!\n", name);
		EXIT = 1;
		return -1;
	}
	else{
		// printf("\t%s ~ PASSED\n", name);
		return 1;
	}
}

void runTests(to_test* myTests){

	if(myTests == NULL)
		return;

	int i = 0;
	while(1){
		to_test temp_test = myTests[i];
		if(temp_test.name == NULL)
			break;


		printf("RUNNING: %s", temp_test.name);
		temp_test.funct();

		if(EXIT == 1){
			printf("ABORTING TESTS !\n");
			break;
		}
		else{
			printf(" .. PASSED\n");
		}

		i++;
	}
	return;

}