/* Logistic Regression */

#ifndef LOGISTIC_H
#define LOGISTIC_H

#include "../include/myMatches.h"

typedef struct logistic_model logM;
typedef struct logistic_weights weights;
typedef struct logistic_sigmoid sigmoid;

struct logistic_model{
    int vectors_size;
    int weights_in;

    weights** allWeights;
    sigmoid* mySigmoid;
};

struct logistic_weights{
    int entries;

    myMatches* label;

    float b;
    float* weightsT;
};

struct logistic_sigmoid{
};

// ~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~

logM* logistic_create();
void logistic_destroy(logM*);

void logistic_fit(logM*, int, float**, int, myMatches**);

// ~~~~~~~~~~~~~~
weights* weights_create();
void weights_destroy(weights*);

void weights_set(weights*, int);
void weights_fit(weights*, float*, myMatches*);
void weights_merge(weights*, float*);

// ~~~~~~~~~~~~~~
sigmoid* sigmoid_create();
void sigmoid_destroy(sigmoid*);

// ~~~~~~~~~~~~~~
float calc_f(weights*, float*);
float calc_s(weights*, float*);
// ~~~~~~~~~~~~~~

#endif