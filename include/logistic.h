/* Logistic Regression */

#ifndef LOGISTIC_H
#define LOGISTIC_H

#include "../include/myMatches.h"

#define LEARING_RATE 2
#define STOP_LIMIT 0.003



typedef struct logistic_model logM;
typedef struct logistic_weights weights;

struct logistic_model{
    int size_totrain;
    int weights_count;

    int trained_times;

    myMatches* clique;

    weights* finalWeights;
};

struct logistic_weights{
    int entries;

    float limit;
    float rate;

    float b;
    float* weightsT;
};

// ~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~

logM* logistic_create();
void logistic_destroy(logM*);

void logistic_fit(logM*, int, int, float**, int*, myMatches*);
                //model, size, vector, tagsArray, clique
float logistic_predict(logM*, float*, int);

void logistic_regression(logM*, float**, int , int, int*);
float logistic_gradDescent();

// ~~~~~~~~~~~~~~
weights* weights_create();
void weights_destroy(weights*);

void weights_set(weights*, int);
void weights_fit(weights*, float*, int);
float weights_update(weights*, float*, int);
// ~~~~~~~~~~~~~~

// ~~~~~~~~~~~~~~
float calc_f(weights*, float*);
float calc_s(weights*, float*);
float calc_L_WB(weights*, float*, int);
float calc_L_Rate(weights*, float*, int);
// ~~~~~~~~~~~~~~

#endif