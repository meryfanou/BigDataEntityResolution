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

void destroy_Info(t_Info*);
void destroy_Info_train(t_Info_train*);
//////////////////////////////////////
void* thread_Start();
#endif