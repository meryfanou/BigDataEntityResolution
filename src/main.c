#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <signal.h>
#include <math.h>
#include "../include/mySpec.h"
#include "../include/myMatches.h"
#include "../include/myHash.h"
#include "../include/myMatches.h"
#include "../include/functs.h"

#define DATASET_X "../camera_specs/2013_camera_specs/"
#define DATASET_W "../sigmod_large_labelled_dataset.csv"

#define TRAIN_PERC 0.6

#define HASH_SIZE 500
#define BUC_SIZE 100

int received_signal = 0;

int main(int argc, char** argv){

    // ./main (-o (file_name)) (-l medium or -l <path_to_W>) (-p <path_to_X>)

    char*   path_X = strdup(DATASET_X);
    char*   path_W = strdup(DATASET_W);

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ READ ARGUMENTS
    char* outputFile = NULL;
    if(argc != 0){
        int i = 0;
        while(i < argc){
            if(strcmp(argv[i], "-o") == 0){
                if(argv[i+1] != NULL)
                    outputFile = strdup(argv[i+1]);
            }
            else if(strcmp(argv[i], "-labels") == 0 || strcmp(argv[i], "-l") == 0){
                if(strcmp(argv[i+1], "medium") == 0 || strcmp(argv[i+1], "m") == 0){
                    char* temp = "../sigmod_medium_labelled_dataset.csv";
                    free(path_W);
                    path_W = strdup(temp);
                }
                else{
                    free(path_W);
                    path_W = strdup(argv[i+1]);
                }
            }
            else if(!strcmp(argv[i], "-path") || !strcmp(argv[i], "-p")){
                free(path_X);
                path_X = strdup(argv[i+1]);
            }
            
            i++;
        }
    }

    DIR                 *datasetX = NULL;
    struct sigaction    act;
    sigset_t            block_mask;


    sigemptyset(&(act.sa_mask));
	act.sa_flags = 0;
    act.sa_handler = sig_int_quit_handler;
	if(sigaction(SIGINT,&act,NULL) < 0 || sigaction(SIGQUIT,&act,NULL) < 0)
	{
		perror("sigaction");
        free(path_X);
        free(path_W);
		exit(-1);
	}
    sigemptyset(&block_mask);
	sigaddset(&block_mask,SIGINT);
	sigaddset(&block_mask,SIGQUIT);

    printf("\nBuilding Hash ...\n");
    hashTable* hashT = hash_create(HASH_SIZE, BUC_SIZE);
    matchesInfo* allMatches = matchesInfoInit();

    // Check for termination signal
    if(received_signal == 1){
        printf("\nCleaning Memory ...\n");
        deleteInfo(allMatches);
        hash_destroy(hashT);
        free(path_X);
        free(path_W);
        if(outputFile != NULL)
            free(outputFile);

        printf("Exiting after receiving termination signal..\n");
        exit(-2);
    }

    // Open datasetX
    if((datasetX = opendir(path_X)) == NULL){
        perror("opendir");
        printf("\nCleaning Memory ...\n");
        deleteInfo(allMatches);
        hash_destroy(hashT);
        free(path_X);
        free(path_W);
        if(outputFile != NULL)
            free(outputFile);
        exit(-3);
    }

    // Read specs from dataset X and store them using hashT
    // If a termination signal was received, return 1. If an error occured, return negative value. Otherwise return 0
    int check = readDataset(datasetX, path_X, &hashT, allMatches);

    if(received_signal == 1 || check != 0){
        printf("\nCleaning Memory ...\n");
        deleteInfo(allMatches);
        hash_destroy(hashT);
        free(path_X);
        free(path_W);
        if(outputFile != NULL)
            free(outputFile);

        closedir(datasetX);

        if(check == 1)
            printf("Exiting after receiving termination signal..\n");

        exit(-4);
    }

    closedir(datasetX);

    printf("       \t\t.. DONE !!\n");


    printf("Reading CSV ...\n");
    // If a termination signal was received, return 1. If an error occured, return negative value. Otherwise return 0
    check = readCSV(path_W, hashT, allMatches);

    if(received_signal == 1 || check != 0){
        printf("\nCleaning Memory ...\n");
        deleteInfo(allMatches);
        hash_destroy(hashT);
        free(path_X);
        free(path_W);
        if(outputFile != NULL)
            free(outputFile);

        if(check == 1)
            printf("Exiting after receiving termination signal..\n");

        exit(-5);
    }

    printf("       \t\t.. DONE !!\n");

    //~~~~~~~~~~~~~~~~~~~~~~ EXTARCT PAIRS
    extractMatches(allMatches, outputFile);


    //~~~~~~~~~~~~~~~~~~~~~~ SEPERATE SPECS TO TRAINING AND TESTING SETS
    mySpec*** trainSet = malloc(sizeof(mySpec**));
	mySpec*** testSet = malloc(sizeof(mySpec**));

    int       trainSize, testSize;

    split_train_n_test(allMatches, trainSet, testSet, TRAIN_PERC, &trainSize, &testSize);


    //~~~~~~~~~~~~~~~~~~~~~~ PREPARE THE INPUT FOR THE MODEL


    //~~~~~~~~~~~~~~~~~~~~~~ FREE MEM
    printf("\nCleaning Memory...\n");
    deleteInfo(allMatches);
    hash_destroy(hashT);

    free(*trainSet);
    free(trainSet);
    free(*testSet);
    free(testSet);

    free(path_X);
    free(path_W);

    if(outputFile != NULL)
        free(outputFile);
    printf("       \t\t.. DONE !!\n\n");

    return 0;
}