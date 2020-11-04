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

int main(void){

    DIR         *datasetX = NULL;

    printf("Building Hash ...\n");
    hashTable* hashT = hash_create(HASH_SIZE, BUC_SIZE);
    matchesInfo* allMatches = matchesInfoInit();


    // Open datasetX
    if((datasetX = opendir(PATH_X)) == NULL){
        perror("opendir");
        exit(-1);
    }

    // Read specs from dataset X and store them using hashT
    readDataset(datasetX, PATH_X, &hashT);

    closedir(datasetX);

    printf("Reading CSV ...\n");
    readCSV(PATH_W, hashT, allMatches);

    deleteInfo(allMatches);

    // hash_print(hashT);
    hash_destroy(hashT);

    return 0;
}