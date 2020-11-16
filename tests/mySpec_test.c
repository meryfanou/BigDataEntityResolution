/*mySpec_test.c*/

#define TEST_NO_MAIN

#include <stdlib.h>
#include "../include/mySpec.h"
#include "../include/acutest.h"

void test_specInit(void){

	mySpec  *spec = specInit();

    // Test malloc
	if(TEST_CHECK_(spec != NULL, "init spec") == 0){
        deleteSpec(spec);
        test_abort_();
    }

    // Test initialization of mySpec's fields

	if(TEST_CHECK_(spec->matches == NULL, "init spec info 1") == 0){
        deleteSpec(spec);
		test_abort_();
    }
	if(TEST_CHECK_(spec->specID == NULL, "init spec info 2") == 0){
        deleteSpec(spec);
		test_abort_();
    }
	if(TEST_CHECK_(spec->properties == NULL, "init spec info 3") == 0){
        deleteSpec(spec);
		test_abort_();
    }
	if(TEST_CHECK_(spec->propNum == 0, "init spec info 4") == 0){
        deleteSpec(spec);
		test_abort_();
    }

	deleteSpec(spec);
    spec = NULL;
}

void test_specCreate(void){

    int     n = 100;
    char    **array = malloc(n*sizeof(char*));
    mySpec  *spec;

    for(int i=0; i<n; i++){
        array[i] = strdup("");
        spec = specCreate(array[i],NULL,0);

        // Test creation of specID field
        if(TEST_CHECK_(!strcmp(spec->specID, array[i]), "create spec info") == 0){
            deleteSpec(spec);
            free(array[i]);
            free(array);
            test_abort_();
        }

        deleteSpec(spec);
        free(array[i]);
    }

    free(array);
}

void test_updateSpecMatches(void){

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

        // Test if mySpec's 'matches' field points to the right match
        if(TEST_CHECK_(specs[i]->matches == newMatch, "update spec matches 1") == 0){
            deleteInfo(matches);
            for(int j=0; j<i; j++)
                deleteSpec(specs[j]);
            free(specs);
            free(array[i]);
            free(array);
            test_abort_();
        }
        // Test if the first item in mySpec's matches is the right one
        if(TEST_CHECK_(!strcmp(specs[i]->matches->specsTable[0]->specID, array[i]), "update spec matches 2") == 0){
            deleteInfo(matches);
            for(int j=0; j<i; j++)
                deleteSpec(specs[j]);
            free(specs);
            free(array[i]);
            free(array);
            test_abort_();
        }

        free(array[i]);
    }

    deleteInfo(matches);
    for(int j=0; j<n; j++)
        deleteSpec(specs[j]);
    free(specs);
    free(array);
}