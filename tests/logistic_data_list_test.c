
#define TEST_NO_MAIN

#include <stdlib.h>
#include "../include/logistic.h"
#include "../include/acutest.h"

void test_create_dataList(void){

    dataI* list = dataI_create(0);
    dataN_destroy(list, list->head);

			// TEST CREATION
	TEST_ASSERT(list != NULL);
    TEST_ASSERT(list->head == NULL);
	
			// TEST SIZE
    TEST_ASSERT(list->all_pairs == 0);
    dataI_destroy(list);
}

void test_push_dataList(void){

    dataI* list = dataI_create(0);
    
	TEST_ASSERT(list->all_pairs == 0);

	int N = 100;
	mySpec* array = malloc(N*sizeof(mySpec));

        // CHECK ADDITION
    int i = 0;
	while(i < N){
        dataI_push(list, &array[i], NULL, NULL, 0, 0);
        TEST_ASSERT(list->all_pairs == i+1);

        TEST_ASSERT(list->head != NULL);
        TEST_ASSERT(list->head->spec1 == &array[i]);
        i++;
	}

        // CHECK NEXT, PREV PTR'S
    TEST_ASSERT(list->head->prev == NULL);


    dataN* temp = list->head;
    dataN* last = NULL;
    i = 0;
    while(i < N){
        // printf("i: %d\n", i);
        TEST_ASSERT(temp->spec1 == &array[N-1-i]);
        if(temp->next == NULL)
            last = temp;
        temp = temp->next;
        i++;
    }

    i --;
    temp = last;

    while(i > 0){
        TEST_ASSERT(temp->spec1 == &array[N-1-i]);
        temp = temp->prev;
        i--;
    }

    dataI_destroy(list);
	free(array);
}

void test_delete_dataList(void){
    int N  = 100;
    mySpec* array = malloc(N*sizeof(mySpec));

    dataI* list = dataI_create(0);

    int i = 0;
    while(i < N){
        dataI_push(list, &array[i++], NULL, NULL, 0, 0);
    }

    TEST_ASSERT(list->all_pairs == N);
    
    // TEST DELETION FROM START
    dataN* temp = list->head;
    dataN_destroy(list, temp);
    TEST_ASSERT(list->all_pairs == N-1);

    // TEST DELETION FROM MIDDLE
    temp = list->head;
    i = 0;
    while(i < (N-1)/2){
        temp = temp->next;
        i++;
    }
    dataN_destroy(list, temp);
    TEST_ASSERT(list->all_pairs == N-2);

    // TEST DELETEION FROM END
    while(i < N-2){
        temp = temp->next;
        i++;
    }
    dataN_destroy(list, temp);
    TEST_ASSERT(list->all_pairs == N-3);

    // TEST DELETE ALL
    dataI_destroy(list);

}