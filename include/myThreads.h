#include <pthread.h>
#ifndef THREADS_H
#define THREADS_H
#include "myHash.h"
#include "logistic.h"
#include "boWords.h"

typedef struct myThreads myThreads;
typedef struct t_Info t_Info;

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
void destroy_Info(t_Info*);
//////////////////////////////////////
void* thread_Start();
#endif