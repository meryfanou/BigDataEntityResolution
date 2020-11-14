#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <signal.h>
#include "../include/mySpec.h"
#include "../include/myMatches.h"
#include "../include/myHash.h"
#include "../include/myMatches.h"
#include "../include/functs.h"

char* PATH_X = "../testSpecs/camera_specs/2013_camera_specs/";
char* PATH_W = "../testSpecs/sigmod_large_labelled_dataset.csv";


#define HASH_SIZE 10
#define BUC_SIZE 100

int main(int argc, char** argv){

    char* outputFile = NULL;
    if(argc != 0){
        int i = 0;
        while(i < argc){
            if(strcmp(argv[i], "-o") == 0){
                if(argv[i+1] != NULL)
                    outputFile = strdup(argv[i+1]);
            }

            if(strcmp(argv[i], "-specs") == 0 || strcmp(argv[i], "-s") == 0){
                if(strcmp(argv[i+1], "small") == 0 || strcmp(argv[i+1], "s")){
                    PATH_X = "../testSpecs/camera_specs_small/2013_camera_specs";
                }
            }
            
            if(strcmp(argv[i], "-labels") == 0 || strcmp(argv[i], "-l") == 0){
                if(strcmp(argv[i+1], "medium") == 0 || strcmp(argv[i+1], "m") == 0){
                    PATH_W = "../testSpecs/sigmod_medium_labelled_dataset.csv";
                }
            }
            
            i++;
        }
    }

    DIR                 *datasetX = NULL;
    struct sigaction    act;
    sigset_t            block_mask;

    received_signal = 0;

    sigemptyset(&(act.sa_mask));
	act.sa_flags = 0;
    act.sa_handler = sig_int_quit_handler;
	if(sigaction(SIGINT,&act,NULL) < 0 || sigaction(SIGQUIT,&act,NULL) < 0)
	{
		perror("sigaction");
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
        if(outputFile != NULL)
            free(outputFile);

        printf("Exiting after receiving termination signal..\n");
        exit(-2);
    }

    // Open datasetX
    if((datasetX = opendir(PATH_X)) == NULL){
        perror("opendir");
        printf("\nCleaning Memory ...\n");
        deleteInfo(allMatches);
        hash_destroy(hashT);
        if(outputFile != NULL)
            free(outputFile);
        exit(-3);
    }

    // Read specs from dataset X and store them using hashT
    // If a termination signal was received, return 1. If an error occured, return negative value. Otherwise return 0
    int check = readDataset(datasetX, PATH_X, &hashT, allMatches);

    if(received_signal == 1 || check != 0){
        printf("\nCleaning Memory ...\n");
        deleteInfo(allMatches);
        hash_destroy(hashT);
        if(outputFile != NULL)
            free(outputFile);

        closedir(datasetX);

        if(check == 1)
            printf("Exiting after receiving termination signal..\n");

        exit(-4);
    }

    closedir(datasetX);

    printf("  \t\t.. DONE !!\n");


    printf("Reading CSV ...\n");
    // If a termination signal was received, return 1. If an error occured, return negative value. Otherwise return 0
    check = readCSV(PATH_W, hashT, allMatches);

    if(received_signal == 1 || check != 0){
        printf("\nCleaning Memory ...\n");
        deleteInfo(allMatches);
        hash_destroy(hashT);
        if(outputFile != NULL)
            free(outputFile);

        if(check == 1)
            printf("Exiting after receiving termination signal..\n");

        exit(-5);
    }

    printf("  \t\t.. DONE !!\n");

    // EXTARCT PAIRS
    extractMatches(allMatches, outputFile);

    // FREE MEM
    printf("\nCleaning Memory ...\n");
    deleteInfo(allMatches);
    hash_destroy(hashT);

    if(outputFile != NULL)
        free(outputFile);
    printf("\t\t.. DONE !!\n\n");

    return 0;
}