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
#include "../include/boWords.h"
#include "../include/logistic.h"

#define DATASET_X "../camera_specs/2013_camera_specs/"
#define DATASET_W "../sigmod_large_labelled_dataset.csv"

#define TRAIN_PERC 0.6
#define TEST_PERC 0.2

#define HASH_SIZE 200
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

    printf("path: %s\n", path_X);

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

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    //~~~~~~~~~~~~~~~~~~~~~~ SEPERATE SPECS TO TRAINING AND TESTING SETS
    mySpec*** trainSet = malloc(sizeof(mySpec**));
	mySpec*** testSet = malloc(sizeof(mySpec**));
	mySpec*** validSet = malloc(sizeof(mySpec**));

    int       trainSize, testSize, validSize;

    printf("\nCreating Training, Testing and Validation Sets..\n");
    split_train_test_valid(allMatches, trainSet, testSet, validSet, &trainSize, &testSize, &validSize, TRAIN_PERC, TEST_PERC);
    printf("       \t\t.. DONE !!\n");

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    //~~~~~~~~~~~~~~~~~~~~~~ PREPARE THE INPUT FOR THE MODEL
    printf("\nBuilding BoW (test_Set) ..\n");
    BoWords*    bow = bow_create(HASH_SIZE, BUC_SIZE);

    text_to_bow(*trainSet, trainSize, &bow);
    printf("       \t\t.. DONE !!\n");

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    //~~~~~~~~~~~~~~~~~~~~~~ TEST TF-IDF >
    printf("\nApplying TF-IDF (test_Set) ..\n");
    tfidf* mytf = tfidf_create();
    tfidf_set(mytf, -1, -1);    // (model, maxTexts, maxWords to scan)
    tfidf_apply(mytf, bow);
    tfidf_destroy(mytf);
    // bow_print(bow);
    printf("       \t\t.. DONE !!\n");
    //~~~~~~~~~~~~~~~~~~~~~ > TEST TF-IDF

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    //~~~~~~~~~~~~~~~~~~~~~~ VECTORIZATION  + LABELS (Ptr's to matches)>

    printf("Vectorizing train_Bow .. \n");
    //bow_print(bow);

    // int vectorSize = 0;
    float** train_vector = malloc(trainSize*sizeof(float*));
    myMatches** train_labels = malloc(trainSize*sizeof(myMatches*));


    int vectorSize = 0;
    int vecCount = 0;
    while(5*vecCount < trainSize){
        // printf("Train Size: %d, curr: %d\n", trainSize, vecCount);

        vectorSize = 0;
        train_vector[vecCount] = vectorization((*trainSet)[vecCount],bow,&vectorSize);
        train_labels[vecCount] = (*trainSet)[vecCount]->matches;
        vecCount++;
    }

    printf("       \t\t.. DONE !!\n");
    //~~~~~~~~~~~~~~~~~~~~~~ > VECTORIZATION

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    //~~~~~~~~~~~~~~~~~~~~~~ TEST LOGISTIC - NOT FINISHED !!!! >


    logM* model = logistic_create();
    logistic_fit(model, vecCount, train_vector, vectorSize, train_labels);

    //~~~~~~~~~~~~~~~~~~~~~~ > TEST LOGISTIC - NOT FINISHED !!!!



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    //~~~~~~~~~~~~~~~~~~~~~~ FREE MEM
    printf("\nCleaning Memory...\n");

        // free hash and match list
    deleteInfo(allMatches);
    hash_destroy(hashT);

        // free Sets
    free(*trainSet);
    free(trainSet);
    free(*testSet);
    free(testSet);
    free(*validSet);
    free(validSet);

        // free vectors - labels
    int free_t_vec = vecCount;
    while(free_t_vec > 0){
        free_t_vec--;
        free(train_vector[free_t_vec]);
    }
    free(train_vector);

        // free labels
    free(train_labels);

        // free bow
    bow_destroy(bow);

        //free model
    logistic_destroy(model);

        // free paths - strings
    free(path_X);
    free(path_W);

        // free File ptr's
    if(outputFile != NULL)
        free(outputFile);
    printf("       \t\t.. DONE !!\n\n");

    return 0;
}