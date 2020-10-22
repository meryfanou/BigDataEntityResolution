#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#define PATH_X "../camera_specs/2013_camera_specs/"
#define PATH_W "../sigmod_large_labelled_dataset.csv"

int main(void){

    char            *dirpath = NULL, *filepath = NULL;
    char            line[256];
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
        if(!strcmp(siteDir->d_name, ".") || !strcmp(siteDir->d_name, ".."))
            continue;

        //printf("%s\n", siteDir->d_name);
        dirpath = (char*)malloc(strlen(PATH_X) + strlen(siteDir->d_name));
        strcpy(dirpath,PATH_X);
        strcat(dirpath, siteDir->d_name);

        if((sitePtr = opendir(dirpath)) == NULL){
            perror("opendir");
            exit(-1);
        }

        while((specFile = readdir(sitePtr)) != NULL){
            if(!strcmp(specFile->d_name, ".") || !strcmp(specFile->d_name, ".."))
                continue;

            //printf("\t%s\n", specFile->d_name);
            filepath = (char*)malloc(strlen(dirpath) + strlen("/") + strlen(specFile->d_name));
            strcpy(filepath, dirpath);
            strcat(filepath, "/");
            strcat(filepath, specFile->d_name);

            if((specFd = fopen(filepath, "r")) == NULL){
                perror("fopen");
                exit(-2);
            }

            while(fgets(line, sizeof(line), specFd) != NULL){
                if(!strcmp(line, "{") || !strcmp(line, "}"))
                    continue;

                //printf("\t\t%s\n", line);
                // Remove '"",' characters from string  (e.g. "shutter": "4-1/2000 sec",) 
            }

            fclose(specFd);
            free(filepath);
            filepath = NULL;
        }

        closedir(sitePtr);
        free(dirpath);
        dirpath = NULL;
    }

    closedir(datasetX);

    return 0;
}