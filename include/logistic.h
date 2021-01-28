/* Logistic Regression */

#ifndef LOGISTIC_H
#define LOGISTIC_H

#include "../include/myMatches.h"
#include "../include/mySpec.h"
#include "../include/jobScheduler.h"
#include <pthread.h>

#define LEARING_RATE 0.6
#define STOP_LIMIT 0.15
#define THRESHOLD 0.5
#define EXTRACT_FILE "extract_model.txt"
#define FILE_OUT_POSITIVES "positives.csv"
#define FILE_OUT_NEGATIVES "negatives.csv"

// #define PATH "./Outputs"



typedef struct logistic_model logM;
typedef struct logistic_weights weights;
typedef struct logistic_data_node dataN;
typedef struct logistic_data_info dataI;
typedef struct info_threads_Node threads_node;
typedef struct info_threads_List threads_list;

struct logistic_model{
    int size_totrain;
    int weights_count;

    int trained_times;

    dataI* dataList;


    pthread_mutex_t model_mtx;
    pthread_mutex_t weights_mtx;

    int fit1;
    int fit0; 

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

struct logistic_data_info{
    int all_pairs;
    int corrects;
    int dimensions;
    
    float score;

    dataN* head;
    dataN* poped;
};

struct logistic_data_node{
    mySpec* spec1;
    mySpec* spec2;

    float** spars;
    int spars_size;

    dataN* next;
    dataN* prev;

    int label;
    float proba;
    int predict;
};


struct info_threads_List{
    threads_node* head;
    int entries;
    threads_node* tail;
    int point;
    pthread_mutex_t lock_it;
    int max;
    jobSch* Scheduler;
    logM* model;
    int pairs_limit;
};

struct info_threads_Node{
    dataI* node;
    threads_node* next;
};

// ~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~

logM* logistic_create();
void logistic_destroy(logM*);

// LOGISTIC VECTOR
int logistic_fit(logM*, int, int, float**, int*);
                //model, size, dimensions, vector, tagsArray
float* logistic_predict_proba(logM*, float**, int, int);
int* logistic_predict(logM*, float**, int, int);
int logistic_regression(logM*, float**, int , int, int*);

// LOGISTIC SPARS
int logistic_fit_spars(logM*, int, float**, int*, int, int);
                    //model, spars_size, spars, tags, tags_size, dimensions
float* logistic_predict_proba_spars(logM*, float**, int, int, int);
int* logistic_predict_spars(logM*, float**, int, int, int);
int logistic_regression_spars(logM*, float**, int, int*, int, int);
void logistic_overfit(logM*, int*, int);

// LOGISTIC DATA LIST
int logistic_fit_dataList(logM*, dataI*);
int logistic_refit_dataList(logM*, dataI*);
void logistic_predict_proba_dataList(logM*, dataI*);
int logistic_regression_dataList(logM*, dataI*);
void logistic_overfit_dataList(logM*, dataI*);
float logistic_score_dataList(logM*, dataI*);


float logistic_score(logM*, int*, int*, int);
void logistic_extract(logM*);
// void logistic_print_strongs(logM*, float*, int);

// ~~~~~~~~~~~~~~
weights* weights_create();
void weights_destroy(weights*);

void weights_set(weights*, int);
void weights_fit(weights*, float*, int);
float weights_update(weights*, float*, int);
void weights_print(weights*);
void weights_extract(weights*);
// ~~~~~~~~~~~~~~

// ~~~~~~~~~~~~~~
float calc_f(weights*, float*);
float calc_f_spars(weights*, float**, int, int , int*);
float calc_f_dataList(weights*, dataN*);

float calc_s(weights*, float*);
float calc_s_spars(weights*, float**, int, int, int*);
void calc_s_dataList(weights*, dataN*);

float calc_L_WB(weights*, float*, int);
float calc_L_Rate(weights*, float*, int);

float active_mean(float*, int);
// ~~~~~~~~~~~~~~

dataI* dataI_create(int);
void dataI_push(dataI*, mySpec*, mySpec*, float**, int, int);
void dataI_destroy(dataI*);
void dataI_remove_weak(dataI*, logM*);
void dataI_extract(dataI*);
dataN* dataI_pop(dataI*);
void dataI_rewind_pop(dataI*);

dataN* dataN_create();
void dataN_destroy(dataI*, dataN*);

// ~~~~~~~~~~~~~~

threads_list* t_list_create(int, jobSch*, logM* model);
void t_list_destroy(threads_list*);

void t_list_push(threads_list*, mySpec*, mySpec*, float**, int, int, int d);

void t_list_subbmit_all(threads_list* list);
void t_list_subbmit_tail(threads_list* list);

threads_node* t_node_create();
void t_node_destroy(threads_node*);



#endif