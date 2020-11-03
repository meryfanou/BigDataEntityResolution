#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "../include/functs.h"
#include "../include/mySpec.h"

char*** readFile(FILE *specFd, int *propNum, char ***properties){
    char    line[512];

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
                (*propNum)++;
                properties = realloc(properties, ((*propNum)+1)*sizeof(char**));
                properties[*propNum] = malloc(2*sizeof(char*));
            }

            properties[*propNum][index] = strdup(str);
            free(str);
            str = NULL;
            str_len = 0;

            index++;
        }

        fgets(line, sizeof(line), specFd);
    }

    return properties;
}

int readCSV(char* fName, hashTable* hashT, matchesInfo* allMatches){
    
            // READ FILE
    FILE* fpin = NULL;
    fpin = fopen(fName, "r+");

    if(fpin == NULL){
        printf("Can't Open CSV File !\n");
        return -1;
    }


    char line[100];
    memset(line, 0, 100);
    fgets(line, 100, fpin);
    memset(line, 0, 100);

    int count = 0;
    while(fgets(line, 100, fpin) != NULL){
        // printf("count: %d\n", count);
                        // GET SPEC'S KEYS
        char* key1 = strtok(line, ",\n");
        char* key2 = strtok(NULL, ",\n");
        char* isMatch = strtok(NULL, ",\n");

                        // CHECK KEYS - UNCOMMENT FOR TESTING
        // printf("\tkey1: %s, key2: %s, isMatch: %s\n", key1, key2, isMatch);

                        // SCAN HASH FOR ENTRIES
        mySpec* spec1 = findRecord_byKey(hashT, key1);
        mySpec* spec2 = findRecord_byKey(hashT, key2);

        // printf("\tspec1: %s counts: %d, spec2: %s counts: %d\n", spec1->specID, spec1->matches->specsCount, spec2->specID, spec2->matches->specsCount);

                          // MERGE MATCHES + FIX POINTERS
                                // !! Check if match2 (to be deleted by merge)
                                // is head then swap them 
        if(strcmp(isMatch, "1") == 0){

            if(spec2->matches == spec1->matches){
                // MATCHES ALREADY TOGETHER -> NO NEED TO MERGE
                memset(line, 0, 100);
                count++;
                continue;
            }
            else if(spec2->matches->prev != NULL){
                swapSpecsMatches(spec1, spec2);
                mergeMatches(allMatches, spec1->matches, spec2->matches);
                spec2->matches = spec1->matches;
            }
            else{
                // printf(" ------ SWAPED --------- \n");
                swapSpecsMatches(spec2, spec1);
                mergeMatches(allMatches, spec2->matches, spec1->matches);
                spec1->matches = spec2->matches;
            }
        }

        memset(line, 0, 100);

        count++;
    }
    
    fclose(fpin);

    return 1;
}

void swapSpecsMatches(mySpec* dest, mySpec* source){

    // printf("\tSWAPING VALUES ... \n");

    int i = 0;

    while(i < source->matches->specsCount){
        // printf("\t\t\t i: %d\n", i);
        if(source->matches->specsTable[i] != source)
            source->matches->specsTable[i]->matches = dest->matches;
        i++;
    }

    // printf("\t\t .. DONE !!\n");

}