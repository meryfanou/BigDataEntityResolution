/* Logistic Regrassion  .c */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "../include/logistic.h" 

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

logM* logistic_create(){
    logM* newModel = malloc(sizeof(logM));

    newModel->vectors_size = 0;
    newModel->weights_in = 0;

    newModel->allWeights = NULL;
    newModel->mySigmoid = sigmoid_create();

    return newModel;
}

void logistic_destroy(logM* model){

    int i = model->weights_in;
    while(i > 0){
        weights_destroy(model->allWeights[--i]);
    }
    sigmoid_destroy(model->mySigmoid);

    free(model);

}

void logistic_fit(logM* model, int vector_size, float** vector , int labels_size, myMatches** labels){
    model->vectors_size = vector_size;

    int count_labels = 0;
    int count_vector = 0;

    printf("lablesSize: %d, vectorSize: %d\n", labels_size, vector_size);

    while(count_labels < count_vector){
        model->allWeights = realloc(model->allWeights, (model->weights_in+1)*sizeof(weights*));
        
        model->allWeights[model->weights_in] = weights_create();
        weights_set(model->allWeights[model->weights_in], vector_size);
        weights_fit(model->allWeights[model->weights_in], vector[count_vector], labels[count_labels]);

        model->weights_in++;
        count_labels++;
        count_vector++;
    }

    printf("log_fit Finished\n");
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

weights* weights_create(){
    weights* newWeight = malloc(sizeof(weights));

    newWeight->entries = 0;
    newWeight->b = 0.0;
    newWeight->weightsT = NULL;

    return newWeight;
}


void weights_destroy(weights* myWeights){
    if(myWeights->entries > 0)
        free(myWeights->weightsT);

    free(myWeights);
}

void weights_set(weights* myWeights, int size){
    myWeights->entries = size;
    myWeights->weightsT = malloc(size*sizeof(float));

    int i = 0;
    while(i < size){
        myWeights->weightsT[i++] = 1.0;
    }
}

void weights_fit(weights* myWeights, float* vector, myMatches* target){
    myWeights->label = target;
    float test_num = calc_s(myWeights, vector);
    printf("Magic Num - p(x) = %.3f\n", test_num);
}


void weights_merge(weights* myWeights, float* newT){

}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

sigmoid* sigmoid_create(){
    sigmoid* newS = malloc(sizeof(sigmoid));


    return newS;
}


void sigmoid_destroy(sigmoid* myS){
    free(myS);
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

float calc_f(weights* myWeights, float* values){
    float sum = myWeights->b;

    int i = 0;
    while(i < myWeights->entries){
        sum += myWeights->weightsT[i] * values[i];
        i++;
    }

    printf("\t f: %.4f\n", sum);
    return sum;
}

float calc_s(weights* myWeights, float* values){
    float f = calc_f(myWeights, values);

    double fixed = -1.0*( (double) f);

    float sum =  1.0 / ( 1.0+ (float)exp(fixed));

    return sum;
}