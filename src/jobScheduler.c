#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <time.h>
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/select.h>
#include <dirent.h>
#include <poll.h>
#include <errno.h>

#include "../include/jobScheduler.h"

jobSch* jobSch_Init(int tnum){
    jobSch* newSche = malloc(sizeof(jobSch));

    newSche->die = 0;
    newSche->start = 0;
    newSche->main_locked = 0;

    // pthread_mutex_init(&newSche->start, NULL);
    pthread_cond_init( &newSche->start_con, NULL);

    pthread_mutex_init(&newSche->print, NULL);
    newSche->threads = myThreads_Init(tnum);
    newSche->queue = myQueue_Init();

    pthread_mutex_lock(&newSche->print);
    newSche->main_locked = 1;

    for(int i=0; i< tnum; i++){
        // printf("mpou\n");
        t_Info2* info = make_info2(newSche);
        pthread_create(&newSche->threads->t_Nums[i], NULL, &main_thread_func, info);
        newSche->threads->active++;
    }

    return newSche;
}

void jobSch_Destroy(jobSch* jSch){
    jSch->die = 1;
    pthread_cond_broadcast(&jSch->start_con);
    if(jSch->main_locked == 1)
        pthread_mutex_unlock(&jSch->print);
    
    myThreads_Destroy(jSch->threads);
    myQueue_Destroy(jSch->queue);
    pthread_cond_destroy(&jSch->start_con);
    pthread_mutex_destroy(&jSch->print);
    // pthread_mutex_destroy(&jSch->start);
    free(jSch);
}

void jobSch_subbmit(jobSch* sched, void* func){
    myQueue_push(sched->queue, func);
}

void jobSch_Start(jobSch* sched){
    printf("Scheduler: Im gonna sleep for (3) and then start my threads ...\n");
    sleep(3);
    printf("Scheduler: Begin Broadcasting ...\n");
    sched->start = 1;
    pthread_cond_broadcast(&sched->start_con);
    sched->main_locked = 0;
    pthread_mutex_unlock(&sched->print);
}

myQueue* myQueue_Init(){
    myQueue* newQ =  malloc(sizeof(myQueue));
    newQ->entries = 0;
    newQ->head = NULL;
    newQ->tail = NULL;
    
    return newQ;
}

void myQueue_Destroy(myQueue* myQ){
    while(myQ->entries > 0){
        qNode_Destroy(myQueue_pop(myQ));
    }
    free(myQ);
}

void myQueue_push(myQueue* myQ, void* to_do){
    qNode* to_push = qNode_Init(to_do);
    
    if(myQ->entries == 0){
        myQ->head = to_push;
        myQ->tail = to_push;
    }
    else{
        myQ->tail->next = to_push;
        myQ->tail = to_push;
    }
    myQ->entries++;
}

qNode* myQueue_pop(myQueue* myQ){
    if(myQ->entries == 0)
        return NULL;

    qNode* to_pop = myQ->head;
    myQ->head = myQ->head->next;
    myQ->entries--;

    return to_pop;
}

qNode* qNode_Init(void* to_do){
    qNode* newQNode = malloc(sizeof(qNode));
    newQNode->next = NULL;
    newQNode->job = jNode_Init(to_do);

    return newQNode;
}

void qNode_Destroy(qNode* qN){
    jNode_Destroy(qN->job);
    free(qN);
}

jNode* jNode_Init(void* to_do){
    jNode* newjNode = malloc(sizeof(jNode));
    newjNode->to_do = to_do;

    return newjNode;
}

void jNode_Destroy(jNode* myjNode){
    free(myjNode);
}


void test1(){
    printf("ciao !\n");
}

void* main_thread_func(void* myInfo){
    t_Info2* info = (t_Info2*) myInfo;
    jobSch* sched = (jobSch*) info->Scheduler;
    while(sched->die == 0){
        // sleep(5);
        pthread_mutex_lock(&sched->print);
        while(sched->start != 1)
            pthread_cond_wait(&sched->start_con, &sched->print);
        
        qNode* f = myQueue_pop(sched->queue);
        if(f == NULL){
            printf("no jobs for me, exiting ..\n");
            pthread_mutex_unlock(&sched->print);
            // break;
        }
        else{
            f->job->to_do();
        }
        // printf("ciao !\n");
        pthread_mutex_unlock(&sched->print);
    }

    return info;
}


void* test(){
    printf("! oaic\n");
    return NULL;
}