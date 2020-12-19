/*logistic_test.c*/

#define TEST_NO_MAIN

#include <stdlib.h>
#include "../include/logistic.h"
#include "../include/acutest.h"

void test_logistic_create(void){
    logM* model = logistic_create();

    TEST_ASSERT(model != NULL);
    TEST_ASSERT(model->finalWeights != NULL);
    TEST_ASSERT(model->size_totrain == 0);
    TEST_ASSERT(model->weights_count == 0);

    logistic_destroy(model);
}

// void test_logistic_destroy(void){

// }