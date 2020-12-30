/*logistic_test.c*/

#define TEST_NO_MAIN

#include <stdlib.h>
#include "../include/mbh.h"
#include "../include/acutest.h"

void test_mbh_create(void){

    MBH* heap = NULL;
    mbh_init(&heap, 0);

    TEST_ASSERT(heap != NULL);
    TEST_ASSERT(heap->root == NULL);
    TEST_ASSERT(heap->max == NULL);
    TEST_ASSERT(heap->maxNodes == 0);

    mbh_delete(&heap);

}


void test_mbh_insert_extract_root(void){
    int N = 10;

    char* names[10];
    float vals[10];

    int i = 0;
    while(i < N){
        vals[i] = (float)i;

        names[i] = malloc(2);
        memset(names[i], 0, 2);
        sprintf(names[i], "%d", i);
        i++;
    }


        // TEST INSERT ALL - MAX = ALL
    MBH* heap = NULL;
    mbh_init(&heap, N);

    i = 0;
    while(i < N){
        TEST_ASSERT(mbh_insert(heap, names[i], vals[i]) == 0);
        i++;
    }
    TEST_ASSERT(heap->numofNodes == N);
    mbh_delete(&heap);

    //     // TEST INSERT ALL - MAX = 2
    // heap = NULL;
    // mbh_init(&heap, 2);
    // i = 0;
    // while(i < N){
    //     mbh_insert(heap, names[i], vals[i]);
    //     i++;
    // }
    // TEST_ASSERT(heap->numofNodes == 2);
    // TEST_ASSERT(strcmp(mbh_extract_root(heap), names[N-2]) == 0);
    // TEST_ASSERT(strcmp(mbh_extract_root(heap), names[N-1]) == 0);

    // mbh_delete(&heap);


    //     // TEST INSERT ALL - MAX = 1
    // heap = NULL;
    // mbh_init(&heap, 1);
    // i = 0;
    // while(i < N){
    //     mbh_insert(heap, names[i], vals[i]);
    //     i++;
    // }
    // TEST_ASSERT(heap->numofNodes == 1);
    // TEST_ASSERT(strcmp(mbh_extract_root(heap), names[N-1]) == 0);
    
    // mbh_delete(&heap);


    //     // TEST INSERT ALL - MAX = 0
    // heap = NULL;
    // mbh_init(&heap, 0);
    // i = 0;
    // while(i < N){
    //     mbh_insert(heap, names[i], vals[i]);
    //     i++;
    // }
    // TEST_ASSERT(heap->numofNodes == 0);
    // TEST_ASSERT(mbh_extract_root(heap) == NULL);
    
    // mbh_delete(&heap);


    i = 0;
    while(i < N){
        free(names[i++]);
    }
}
