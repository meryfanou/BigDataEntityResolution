#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "../include/mySpec.h"
#include "../include/myMatches.h"
#include "../include/myHash.h"
#include "../include/myMatches.h"
#include "../include/functs.h"

#define PATH_X "../camera_specs/2013_camera_specs/"
#define PATH_W "../sigmod_large_labelled_dataset.csv"


#define HASH_SIZE 10
#define BUC_SIZE 100

int main(int argc, char** argv){

    char* outputFile = NULL;
    if(argc != 0){
        int i = 0;
        while(i < argc){
            if(strcmp(argv[i], "-o") == 0){
                outputFile = strdup(argv[i+1]);
            }
            i++;
            // printf("i: %d, argc: %d\n", i, argc);
        }
    }
    printf("sliahcsdoh\n");


    DIR         *datasetX = NULL;

    printf("\nBuilding Hash ...");
    hashTable* hashT = hash_create(HASH_SIZE, BUC_SIZE);
    matchesInfo* allMatches = matchesInfoInit();


    // Open datasetX
    if((datasetX = opendir(PATH_X)) == NULL){
        perror("opendir");
        exit(-1);
    }

    // Read specs from dataset X and store them using hashT
    readDataset(datasetX, PATH_X, &hashT, allMatches);

    closedir(datasetX);

    printf("\t.. DONE !!\n");


    printf("Reading CSV ...");
    readCSV(PATH_W, hashT, allMatches);
    printf("  \t.. DONE !!\n");

    // EXTARCT PAIRS
    extractMatches(allMatches, outputFile);

    // FREE MEM
    printf("Cleaning Memory ...");
    deleteInfo(allMatches);
    hash_destroy(hashT);

    if(outputFile != NULL)
        free(outputFile);
    printf("\t.. DONE !!\n\n");

    return 0;
}