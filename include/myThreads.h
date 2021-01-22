#ifndef THREADS_H
#define THREADS_H
#include "myHash.h"
#include "logistic.h"
#include "boWords.h"
#include <pthread.h>
// #include "jobScheduler.h"


typedef struct myThreads myThreads;
typedef struct t_Info t_Info;
typedef struct t_Info2 t_Info2;

struct myThreads{
	pthread_t* t_Nums;
	int active;
	int size;
};


struct t_Info{
	hashTable* hashT;
	logM* model;
	BoWords* bow;
	int cell;
	char* target;
};

struct t_Info2{
	void* Scheduler;
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
t_Info* make_info(int, char*);
t_Info2* make_info2(void*);

void destroy_Info(t_Info*);
void destroy_Info2(t_Info2*);
//////////////////////////////////////
void* thread_Start();
#endif