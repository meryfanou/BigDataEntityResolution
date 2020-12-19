/*MatchesList_test.c*/

#define TEST_NO_MAIN

#include <stdlib.h>
#include "../include/myMatches.h"
#include "../include/mySpec.h"
#include "../include/acutest.h"

void test_createNegs(void){

    nlist* list = create_nlist();
    destroy_nNode(list->head);

			// TEST CREATION
	TEST_ASSERT(list != NULL);
    TEST_ASSERT(list->head == NULL);
    TEST_ASSERT(list->tail == NULL);
	
			// TEST SIZE
    TEST_ASSERT(list->entries == 0);
    destroy_nlist(list);
}

void test_addNegs(void){

    nlist* list = create_nlist();

	TEST_ASSERT(list->entries == 0);

	int N = 100;
	myMatches* array = malloc(N*sizeof(myMatches));

        // CHECK ADDITION
    int i = 0;
	while(i < N){
        add_nlist(list, &array[i]);
        TEST_ASSERT(list->entries == i+1);

        TEST_ASSERT(list->tail != NULL);
        TEST_ASSERT(list->head != NULL);
        TEST_ASSERT(list->tail->matchptr == &array[i]);
		i++;
	}

        // CHECK NEXT, PREV PTR'S
    TEST_ASSERT(list->head->prev == NULL);
    TEST_ASSERT(list->tail->next == NULL);


    nNode* temp = list->head;
    i = 0;
    while(i < N){
        TEST_ASSERT(temp->matchptr == &array[i]);
        if(i != N-1)
            TEST_ASSERT(temp->next->matchptr == &array[i+1]);
        else
            TEST_ASSERT(list->tail == temp);
        temp = temp->next;
        i++;
    }

	destroy_nlist(list);
	free(array);
}

void test_seek_nlist(void){
    nlist* list = create_nlist();
    
    int N = 100;
	myMatches* array = malloc(N*sizeof(myMatches));

			// FILL STRUCT (ALREADY CHECKED INSERT WORKS)
    int i = 0;
    while(i < N-1){
        add_nlist(list, &array[i++]);
    }

    nNode* check = NULL;
    check = seek_nlist(list, &array[0]);
    TEST_ASSERT(check == list->head);

    check = seek_nlist(list, &array[N-2]);
    TEST_ASSERT(check == list->tail);


    check = seek_nlist(list, &array[N/2]);
    TEST_ASSERT(check->matchptr == &array[N/2]);

    check = seek_nlist(list, &array[N-1]);
    TEST_ASSERT(check == NULL);

}

void test_remove_nlist(void){

    nlist* list = create_nlist();

	int N = 100;
	myMatches* array = malloc(N*sizeof(myMatches));

			// FILL STRUCT (ALREADY CHECKED INSERT WORKS)
    int i = 0;
    while(i < N){
        add_nlist(list, &array[i++]);
    }

    TEST_ASSERT(list->entries == N);

			// TEST DELETION FROM START
    i = 0;
    while(i < N){
        TEST_ASSERT(list->head->matchptr == &array[i]);

        remove_nlist(list, &array[i]);
        TEST_ASSERT(list->entries == N-i-1);
        
        if(list->entries != 0){
            TEST_ASSERT(list->head != NULL);
            TEST_ASSERT(list->head->matchptr != &array[i]);        }
        else
            TEST_ASSERT(list->head == NULL);
        

        i++;
    }

    TEST_ASSERT(list->entries == 0);

		// REFILL STRUCT
    i = 0;
    while(i < N){
        add_nlist(list, &array[i++]);
    }   
			// TEST DELETE SECOND NODE
    TEST_ASSERT(list->head->next->matchptr == &array[1]);
    TEST_ASSERT(list->head->next->next->matchptr == &array[2]);
    remove_nlist(list, &array[1]);
    TEST_ASSERT(list->head->next->matchptr != &array[1]);
    TEST_ASSERT(list->head->next->matchptr == &array[2]);
    TEST_ASSERT(list->entries == N-1);

    destroy_nlist(list);
	free(array);

}

// void test_mergeMatches(void){

// 	myMatches* to_merge1 = myMatchesInit();
// 	myMatches* to_merge2 = myMatchesInit();

// 	int N = 100;
// 	mySpec* specArray = malloc(N*sizeof(mySpec));


// 					// SPLIT SPECS ARRAY TO MATCHES_TO_MERGE
// 	int i = 0;
// 	while(i < 100){
// 		if(i < 50)
// 			pushMatch(to_merge1, &specArray[i]);
// 		else
// 			pushMatch(to_merge2, &specArray[i]);
// 		i++;
// 	}


// 				// CHECK ENTRIES BEFORE
// 	TEST_ASSERT(to_merge1->specsCount == 50);
// 	TEST_ASSERT(to_merge2->specsCount == 50);
	
// 	mergeMatches(NULL, to_merge1, to_merge2);

// 				// CHECK POINTERS
// 	TEST_ASSERT(to_merge1 != NULL);
	
// 	// printf("counts2: %d\n", to_merge2->specsCount);

// 				// CHECK ENTRIES AFTER
// 	TEST_ASSERT(to_merge1->specsCount == N);

// 				// CHECK INPUT LINE
// 	i = 0;
// 	while(i < N){
// 		TEST_ASSERT(to_merge1->specsTable[i] == &specArray[i]);
// 		i++;
// 	}


// 	free(specArray);
// 	deleteMatches(NULL, to_merge1);
// }