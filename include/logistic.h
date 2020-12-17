/* Logistic Regression */

#ifndef LOGISTIC_H
#define LOGISTIC_H

#include "../include/myMatches.h"

#define LEARING_RATE 0.6
#define STOP_LIMIT 0.15
#define THRESHOLD 0.5
#define EXTRACT_FILE "extract_model.txt"



typedef struct logistic_model logM;
typedef struct logistic_weights weights;

struct logistic_model{
    int size_totrain;
    int weights_count;

    int trained_times;

    weights* finalWeights;
};

struct logistic_weights{
    int entries;

    float limit;
    float rate;
    float threshold;

    float b;
    float* weightsT;
};

// ~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~

logM* logistic_create();
void logistic_destroy(logM*);

void logistic_fit(logM*, int, int, float**, int*);
                //model, size, dimensions, vector, tagsArray
float* logistic_predict_proba(logM*, float**, int, int);
int* logistic_predict(logM*, float**, int, int);
float logistic_score(logM*, int*, int*, int);

void logistic_regression(logM*, float**, int , int, int*);
float logistic_gradDescent();

void logistic_extract(logM*);
// ~~~~~~~~~~~~~~
weights* weights_create();
void weights_destroy(weights*);

void weights_set(weights*, int);
void weights_fit(weights*, float*, int);
float weights_update(weights*, float*, int);
void weights_print(weights*);
void weights_extract(weights*, char*);
// ~~~~~~~~~~~~~~

// ~~~~~~~~~~~~~~
float calc_f(weights*, float*);
float calc_s(weights*, float*);
float calc_L_WB(weights*, float*, int);
float calc_L_Rate(weights*, float*, int);
float active_mean(float*, int);
// ~~~~~~~~~~~~~~

#endif