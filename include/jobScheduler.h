#ifndef JOB_H
#define JOB_H

#include "myThreads.h"

typedef struct jobScheduler jobSch;
typedef struct myQueue myQueue;
typedef struct myQueueNode qNode;
typedef struct jobNode jNode;

typedef void (*funcPtr)();

struct jobScheduler{
    myThreads* threads;
    myQueue* queue;

    pthread_cond_t start_con;
    // pthread_mutex_t start;
    pthread_mutex_t print;
    int die;
    int start;
    int main_locked;
};

struct myQueue{
    qNode* head;
    qNode* tail;
    int entries;
};


struct myQueueNode{
    jNode* job;
    qNode* next; 
};


struct jobNode{
    funcPtr to_do;
};


jobSch* jobSch_Init(int);
void jobSch_Start(jobSch*);
void jobSch_Destroy(jobSch*);
void jobSch_subbmit(jobSch*, void*);

myQueue* myQueue_Init();
void myQueue_Destroy(myQueue*);
void myQueue_push(myQueue*, void*);
qNode* myQueue_pop(myQueue*);

qNode* qNode_Init(void* to_do);
void qNode_Destroy(qNode*);

jNode* jNode_Init(void*);
void jNode_Destroy(jNode*);

void* main_thread_func(void*);
void* test();

#endif