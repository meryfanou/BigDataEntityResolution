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
    newSche->threads_waiting = 0;

    pthread_cond_init( &newSche->start_con, NULL);
    pthread_mutex_init(&newSche->queue_mtx, NULL);

    newSche->threads = myThreads_Init(tnum);
    newSche->queue = myQueue_Init();

   

    for(int i=0; i< tnum; i++){
        t_Info* info = make_info(newSche);
        pthread_create(&newSche->threads->t_Nums[i], NULL, &main_thread_func, info);
        newSche->threads->active++;
    }

    return newSche;
}

void jobSch_Destroy(jobSch* jSch){
    jSch->die = 1;
    jobSch_waitAll(jSch);
    jobSch_Start(jSch);
    
    myThreads_Destroy(jSch->threads);
    myQueue_Destroy(jSch->queue);
    
    pthread_cond_destroy(&jSch->start_con);

    pthread_mutex_destroy(&jSch->queue_mtx);
    
    free(jSch);
}

void jobSch_subbmit(jobSch* sched, void* func, void* info, char* mode){
    pthread_mutex_lock(&sched->queue_mtx);

    myQueue_push(sched->queue, func, info, mode);

    pthread_mutex_unlock(&sched->queue_mtx);
}

void jobSch_Start(jobSch* sched){
    // printf("Scheduler: Im gonna sleep for (3) and then start my threads ...\n");
    // sleep(3);
    // printf("Scheduler: Begin Broadcasting ...\n");

    pthread_mutex_lock(&sched->queue_mtx);
    pthread_cond_broadcast(&sched->start_con);
    pthread_mutex_unlock(&sched->queue_mtx);
}

void jobSch_waitAll(jobSch* sched){
    printf("Waiting .. \n");
    while(sched->queue->entries !=0 && sched->threads_waiting != sched->threads->size){}
    printf("\t .. Done\n");
}

////////////////////////////////////////////////////
////////////////////////////////////////////////////

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

void myQueue_push(myQueue* myQ, void* to_do, void* info, char* mode){
    qNode* to_push = qNode_Init(to_do, info, mode);
    
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


////////////////////////////////////////////////////
////////////////////////////////////////////////////


qNode* qNode_Init(void* to_do, void* info, char* mode){
    qNode* newQNode = malloc(sizeof(qNode));
    newQNode->next = NULL;
    newQNode->job = jNode_Init(to_do, info, mode);

    return newQNode;
}

void qNode_Destroy(qNode* qN){
    jNode_Destroy(qN->job);
    free(qN);
}


////////////////////////////////////////////////////
////////////////////////////////////////////////////


jNode* jNode_Init(void* to_do, void* info, char* mode){
    jNode* newjNode = malloc(sizeof(jNode));
    newjNode->to_do = to_do;
    newjNode->info = info;
    newjNode->mode = strdup(mode);

    return newjNode;
}

void jNode_Destroy(jNode* myjNode){
    free(myjNode->mode);
    destroy_Info_train((t_Info_train*) myjNode->info);
    free(myjNode);
}


////////////////////////////////////////////////////
////////////////////////////////////////////////////


void test1(){
    printf("ciao !\n");
}


////////////////////////////////////////////////////
////////////////////////////////////////////////////


void* main_thread_func(void* myInfo){
        // parsh info
    t_Info* info = (t_Info*) myInfo;
    jobSch* sched = (jobSch*) info->Scheduler;

    int waiting_flag = 0;

        // main job
    while(sched->die == 0){
        pthread_mutex_lock(&sched->queue_mtx);
        while(sched->queue->entries == 0 && sched->die == 0){
            if(waiting_flag == 0){
                sched->threads_waiting++;
                waiting_flag = 1;
            }
            pthread_cond_wait(&sched->start_con, &sched->queue_mtx);
        }

        sched->threads_waiting--;
        waiting_flag = 0;

        qNode* f = myQueue_pop(sched->queue);
        pthread_mutex_unlock(&sched->queue_mtx);
        if(f == NULL){
            continue;
            // printf("no jobs for me, exiting ..\n");
        }
        else{
            t_Info_train* info_to_train = NULL;
            // printf("ACTUALLY DOING SMTHNIG ...\n");
            if(strcmp(f->job->mode, "train") == 0){
                info_to_train = (t_Info_train*) f->job->info;
                // printf("\t CALLING TRAIN ..\n");
                f->job->to_do(info_to_train->model, info_to_train->info_list);
                // printf("\t TRAIN_ENDED !!\n");
            }
            else if(strcmp(f->job->mode, "test") == 0){
                // printf("\t CALLING TRAIN ..\n");
                f->job->to_do();
                // printf("\t TRAIN_ENDED !!\n");
            }  
            
            // printf("prin to destroy\n");
            qNode_Destroy(f);
            // printf("meta to destroy\n");
        }
    }

        // return
    return info;
}


void* test(){ // TEST FUNCT
    printf("! oaic\n");
    return NULL;
}