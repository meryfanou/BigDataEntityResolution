#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "../include/mySpec.h"
#include "../include/myMatches.h"
#include "../include/myHash.h"

#define PATH_X "../camera_specs_small/2013_camera_specs/"
#define PATH_W "../sigmod_large_labelled_dataset.csv"

#define HASH_SIZE 10
#define BUC_SIZE 100

int main(void){

    hashTable* hashT = hash_create(HASH_SIZE, BUC_SIZE);

    int             propNum = -1;
    char            line[512];
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

            // Read spec's properties
            fgets(line, sizeof(line), specFd);
            while(line != NULL){
                // Ignore '{' and '}' lines in json file
                if(line[0] == '{'){
                    fgets(line, sizeof(line), specFd);
                    continue;
                }
                if( line[0] == '}')
                    break;
                
                char    *rest = line;
                char    *token = NULL, *str = NULL, *prev = NULL, *temp = NULL;
                int     index = 0, str_len = 0, need_extra = 0;

                // Seperate the key from the value in the ("key" : "value") pair
                while(index < 2){
                    need_extra = 0;

                    // Remove extra \" and , from the string
                    while((token = strtok_r(rest, "\"", &rest)) != NULL){
                        // If it is the end of the key string or the value string
                        if(!strcmp(token,": ") || !strcmp(token,": [\n") || !strcmp(token,",\n") || !strcmp(token,"\n")){
                            if(!strcmp(token, ": [\n"))
                                prev = strdup(token);

                            need_extra = 0;
                            break;
                        }

                        if(str != NULL)
                            temp = strdup(str);

                        str = realloc(str, str_len + strlen(token) + 1);
                        if(temp != NULL){
                            strcpy(str, temp);
                            strcat(str, token);
                            free(temp);
                            temp = NULL;
                        }
                        else{
                            strcpy(str, token);
                        }

                        str_len = strlen(str);
                    }

                    // If there is more to read from the file for the value string
                    if(token == NULL && index == 1){
                        need_extra = 1;

                        // If the value string is an 'array' of strings
                        if(prev != NULL && !strcmp(prev, ": [\n")){
                            if(str != NULL)
                                temp = strdup(str);

                            str = realloc(str, str_len + strlen("[\n") + 1);
                            if(temp != NULL){
                                strcpy(str, temp);
                                strcat(str, "[\n");
                                free(temp);
                                temp = NULL;
                            }
                            else{
                                strcpy(str, "[\n");
                            }

                            str_len = strlen(str);
                            free(prev);
                            prev = NULL;

                            while(need_extra){
                                fgets(line, sizeof(line), specFd);
                                rest = line;

                                // Remove extra \" and , from the string
                                while((token = strtok_r(rest, "\"", &rest)) != NULL){
                                    // Read from the file until ']' is found
                                    if(!strcmp(token,"],\n") || !strcmp(token,"    ],\n") || !strcmp(token,"]\n") || !strcmp(token,"    ]\n")){
                                        if(str != NULL)
                                            temp = strdup(str);

                                        str = realloc(str, str_len + strlen(token) + 1);
                                        if(temp != NULL){
                                            strcpy(str, temp);
                                            strcat(str, token);
                                            free(temp);
                                            temp = NULL;
                                        }
                                        else{
                                            strcpy(str, token);
                                        }

                                        str_len = strlen(str);
                                        need_extra = 0;
                                        break;
                                    }

                                    if(str != NULL)
                                        temp = strdup(str);

                                    str = realloc(str, str_len + strlen(token) + 1);
                                    if(temp != NULL){
                                        strcpy(str, temp);
                                        strcat(str, token);
                                        free(temp);
                                        temp = NULL;
                                    }
                                    else{
                                        strcpy(str, token);
                                    }

                                    str_len = strlen(str);
                                }
                            }
                        }

                        // If the value is a string
                        while(need_extra){
                            fgets(line, sizeof(line), specFd);
                            rest = line;

                            // Remove extra \" and , from the string
                            while((token = strtok_r(rest, "\"", &rest)) != NULL){
                                // Read until the end of the string (and the line) is reached
                                if(!strcmp(token,",\n") || !strcmp(token,"\n")){
                                    need_extra = 0;
                                    break;
                                }

                                if(str != NULL)
                                    temp = strdup(str);

                                str = realloc(str, str_len + strlen(token) + 1);
                                if(temp != NULL){
                                    strcpy(str, temp);
                                    strcat(str, token);
                                    free(temp);
                                    temp = NULL;
                                }
                                else{
                                    strcpy(str, token);
                                }

                                str_len = strlen(str);
                            }
                        }
                    }

                    // Will save key,value in a 2D array
                    if(index == 0){
                        propNum++;
                        properties = realloc(properties, (propNum+1)*sizeof(char**));
                        properties[propNum] = malloc(2*sizeof(char*));
                    }

                    //printf("%s\n", specID);
                    properties[propNum][index] = strdup(str);
                    free(str);
                    str = NULL;
                    str_len = 0;

                    index++;
                }

                fgets(line, sizeof(line), specFd);
            }

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