/*mySpec_test.c*/

#include <stdlib.h>
#include "../include/mySpec.h"
#include "../include/myuTesting.h"


void test_initSpec(void){

	mySpec  *spec = specInit();

	if(myValidCheck(spec != NULL, "init spec") == -1){
        deleteSpec(spec);
		return;
    }
	if(myValidCheck(spec->matches == NULL, "init spec info 1") == -1){
        deleteSpec(spec);
		return;
    }
	if(myValidCheck(spec->specID == NULL, "init spec info 2") == -1){
        deleteSpec(spec);
		return;
    }
	if(myValidCheck(spec->properties == NULL, "init spec info 3") == -1){
        deleteSpec(spec);
		return;
    }
	if(myValidCheck(spec->propNum == 0, "init spec info 4") == -1){
        deleteSpec(spec);
		return;
    }

	deleteSpec(spec);
    spec = NULL;
}

void test_createSpec(){

    int     n = 100;
    char    **array = malloc(n*sizeof(char*));
    mySpec  *spec = specInit();

    for(int i=0; i<n; i++){
        array[i] = strdup("");
        spec = specCreate(array[i],NULL,0);

        if(myValidCheck(!strcmp(spec->specID, array[i]), "create spec info") == -1){
            deleteSpec(spec);
            free(array[i]);
            free(array);
            return;
        }

        deleteSpec(spec);
        free(array[i]);
    }

    free(array);
}

void test_updateSpecMatches(){

    int         n = 100;
    char        **array = malloc(n*sizeof(char*));
    matchesInfo *matches = matchesInfoInit();
    myMatches   *newMatch = NULL;
    mySpec      **specs = malloc(n*sizeof(mySpec*));

    for(int i=0; i<n; i++){
        array[i] = strdup("");
        specs[i] = specCreate(array[i],NULL,0);
        newMatch = matchesAdd(matches, specs[i]);
        updateSpecMatches(specs[i], newMatch);

        if(myValidCheck(specs[i]->matches == newMatch, "update spec matches 1") == -1){
            deleteInfo(matches);
            for(int j=0; j<i; j++)
                deleteSpec(specs[j]);
            free(specs);
            free(array[i]);
            free(array);
            return;
        }
        if(myValidCheck(!strcmp(specs[i]->matches->specsTable[0]->specID, array[i]), "update spec matches 2") == -1){
            deleteInfo(matches);
            for(int j=0; j<i; j++)
                deleteSpec(specs[j]);
            free(specs);
            free(array[i]);
            free(array);
            return;
        }

        free(array[i]);
    }

    deleteInfo(matches);
    for(int j=0; j<n; j++)
        deleteSpec(specs[j]);
    free(specs);
    free(array);
}


int main(void){
	to_test myTests[] = {
        {"test_initSpec", test_initSpec},
		{"test_createSpec", test_createSpec},
        {"test_updateSpecMatches", test_updateSpecMatches},
		{NULL, NULL}
	};

	runTests(myTests);
	return 0;
}