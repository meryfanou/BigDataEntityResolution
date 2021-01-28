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

    int threads_waiting;

    pthread_cond_t start_con;    
    pthread_mutex_t queue_mtx;
    pthread_mutex_t lock_wait;

    int die;
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
    void* info;
    char* mode;
};


jobSch* jobSch_Init(int);
void jobSch_Start(jobSch*);
void jobSch_Destroy(jobSch*);
void jobSch_subbmit(jobSch*, void*, void*, char*);
void jobSch_waitAll(jobSch*);

myQueue* myQueue_Init();
void myQueue_Destroy(myQueue*);
void myQueue_push(myQueue*, void*, void*, char*);
qNode* myQueue_pop(myQueue*);

qNode* qNode_Init(void* to_do, void*, char*);
void qNode_Destroy(qNode*);

jNode* jNode_Init(void*, void*, char*);
void jNode_Destroy(jNode*);

void* main_thread_func(void*);
void* test();

#endif