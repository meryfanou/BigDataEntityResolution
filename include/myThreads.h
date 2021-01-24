#ifndef THREADS_H
#define THREADS_H
#include "myHash.h"
#include "logistic.h"
#include "boWords.h"
#include <pthread.h>
// #include "jobScheduler.h"


typedef struct myThreads myThreads;
typedef struct t_Info t_Info;
typedef struct t_Info_train t_Info_train;
typedef struct t_Info_test t_Info_test;
typedef struct t_Info_retrain t_Info_retrain;


struct myThreads{
	pthread_t* t_Nums;
	int active;
	int size;
};


struct t_Info{
	void* Scheduler;
};

struct t_Info_train{
	void* model;
	void* info_list;
};

struct t_Info_test{
	void* model;
	void* info_list;
};


struct t_Info_retrain{
	void* model;
	void* myrec;
	void* mybuc;
	info_ar* myar;
};

//functs

myThreads* myThreads_Init(int);
void myThreads_Destroy(myThreads*);
void myThreads_MASSACRE(myThreads*);
//////////////////

// void create_threads(myThreads*, int, void*);
// void myThreads_StopAll(myThreads*);
// void myThreads_StartAll(myThreads*);

//////////////////////////////////////
t_Info* make_info(void*);
t_Info_train* make_info_train(void*, void*);
t_Info_test* make_info_test(void*, void*);
t_Info_retrain* make_info_retrain(void*, void*, void*, info_ar*);

void destroy_Info(t_Info*);
void destroy_Info_train(t_Info_train*);
void destroy_Info_test(t_Info_test*);
void destroy_Info_retrain(t_Info_retrain*);
//////////////////////////////////////
void* thread_Start();
#endif