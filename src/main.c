#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "../include/mySpec.h"
#include "../include/myMatches.h"
#include "../include/myHash.h"
#include "../include/functs.h"

// <<<<<<< HEAD
#define PATH_X "../testSpecs/camera_specs_small/2013_camera_specs/"
#define PATH_W "../testSpecs/sigmod_large_labelled_dataset.csv"
// =======
// #define PATH_X "../camera_specs/2013_camera_specs/"
// #define PATH_W "../sigmod_large_labelled_dataset.csv"
// >>>>>>> 5088f4e21aa970de9af0dceb3f4a8f05906482c7

#define HASH_SIZE 10
#define BUC_SIZE 100

int main(void){

    hashTable* hashT = hash_create(HASH_SIZE, BUC_SIZE);

    int             propNum = -1;
    //char            line[512];
    char            *dirpath = NULL, *filepath = NULL;
    char            ***properties = NULL;
    DIR             *datasetX = NULL, *sitePtr = NULL;
    FILE            *specFd = NULL;
    struct dirent   *siteDir = NULL, *specFile = NULL;

    // Open datasetX
    if((datasetX = opendir(PATH_X)) == NULL){
        perror("opendir");
        exit(-1);
    }

    // Read from datasetX
    while((siteDir = readdir(datasetX)) != NULL){
        // Ignore . and ..
        if(!strcmp(siteDir->d_name, ".") || !strcmp(siteDir->d_name, ".."))
            continue;

        // Keep the full path
        dirpath = (char*)malloc(strlen(PATH_X) + strlen(siteDir->d_name) + 1);
        strcpy(dirpath,PATH_X);
        strcat(dirpath, siteDir->d_name);

        // Open current directory (website)
        if((sitePtr = opendir(dirpath)) == NULL){
            perror("opendir");
            exit(-1);
        }

        // Read the files in the directory
        while((specFile = readdir(sitePtr)) != NULL){
            // Ignore . and ..
            if(!strcmp(specFile->d_name, ".") || !strcmp(specFile->d_name, ".."))
                continue;

            // Keep the full path
            filepath = (char*)malloc(strlen(dirpath) + strlen("/") + strlen(specFile->d_name) + 1);
            strcpy(filepath, dirpath);
            strcat(filepath, "/");
            strcat(filepath, specFile->d_name);

            // Open current file (spec)
            if((specFd = fopen(filepath, "r")) == NULL){
                perror("fopen");
                exit(-2);
            }

            char    *filename = strdup(specFile->d_name);
            char    *specID = NULL;

            // Keep specID (<website_name>//<spec_num>)
            filename = strtok(filename, ".");
            specID = malloc(strlen(siteDir->d_name) + strlen("//") + strlen(filename) + 1);
            strcpy(specID, siteDir->d_name);
            strcat(specID, "//");
            strcat(specID, filename);
            free(filename);

            // Get array with the properties of current spec
            properties = readFile(specFd, &propNum, properties);
            propNum++;

            // Create spec node
            mySpec *newSpec = specInit(specID, &properties, propNum);
            //printSpec(newSpec);
            hash_add(hashT, newSpec, hash1(newSpec->specID));
            // deleteSpec(newSpec);

            fclose(specFd);
            free(filepath);
            filepath = NULL;

            free(specID);
            for(int i=0; i<propNum; i++){
                free(properties[i][0]);
                free(properties[i][1]);
                free(properties[i]);
            }
            free(properties);
            properties = NULL;
            propNum = -1;
        }

        closedir(sitePtr);
        free(dirpath);
        dirpath = NULL;
    }

    closedir(datasetX);

    hash_print(hashT);
    hash_destroy(hashT);

    return 0;
}