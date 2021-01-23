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

#include "../include/myThreads.h"


myThreads* myThreads_Init(int tNum){

	myThreads* newT = malloc(sizeof(myThreads));
	
	newT->t_Nums = malloc(tNum*sizeof(pthread_t));
	newT->active = 0;
	newT->size = tNum;

	return newT;
}

void myThreads_Destroy(myThreads* threads){
	// wait for all threads to end and free mem
	void* res;

	int i = 0;
	while(threads->active > 0){
		i = threads->active-1;
		if(pthread_join(threads->t_Nums[i], &res) == 0){
			free(res);
			threads->active--;
		}
	}

	free(threads->t_Nums);
	free(threads);
}

void myThreads_MASSACRE(myThreads* threads){
	// LAST CHANCE
	sleep(15);

	// KILL THEM ALL
	int i = 0;

	i = 0;
	while(threads->active > 0){
		i = threads->active-1;

		pthread_cancel(threads->t_Nums[i]);
		threads->active--;
		
	}

	free(threads->t_Nums);
	free(threads);

}

//////////////////////////////////////

// void create_threads(myThreads* threads, int tNum, void* myfunc){
// 	int i = 0;

// 	while(i < tNum){
// 		pthread_create(&threads->t_Nums[i], NULL, myfunc, NULL);
// 		threads->count++;
// 		// printf("mpla\n");
// 		i++;
// 	}
// }


t_Info_train* make_info_train(void* model, void* info_list){
	t_Info_train* newinfo = malloc(sizeof(t_Info_train));
	newinfo->model = model;
	newinfo->info_list = info_list;
	
	return newinfo;
}

void destroy_Info_train(t_Info_train* info){
	dataI* list = (dataI*) info->info_list;
	dataN_destroy(list, list->head);
	free(info);
}

t_Info* make_info(void* scheduler){
	t_Info* newInfo = malloc(sizeof(t_Info));
	newInfo->Scheduler = scheduler;
	return newInfo;
}