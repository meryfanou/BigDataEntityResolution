#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <signal.h>
#include "../include/functs.h"
#include "../include/mySpec.h"
#include "../include/myHash.h"


int readDataset(DIR *datasetX, char *path, hashTable **hashT, matchesInfo* allMatches){
    int             propNum = -1;
    char            *dirpath = NULL, *filepath = NULL;
    specInfo        **properties = NULL;
    DIR             *sitePtr = NULL;
    FILE            *specFd = NULL;
    struct dirent   *siteDir = NULL, *specFile = NULL;

    struct sigaction    act;
    sigset_t            block_mask;

    received_signal = 0;

    sigemptyset(&(act.sa_mask));
	act.sa_flags = 0;
    act.sa_handler = sig_int_quit_handler;
	if(sigaction(SIGINT,&act,NULL) < 0 || sigaction(SIGQUIT,&act,NULL) < 0)
	{
		perror("sigaction");
		return -1;
	}
    sigemptyset(&block_mask);
	sigaddset(&block_mask,SIGINT);
	sigaddset(&block_mask,SIGQUIT);

    // Read from datasetX
    while((siteDir = readdir(datasetX)) != NULL){
        // Check for termination signal
        if(received_signal == 1)
            return 1;

        // Ignore . and ..
        if(!strcmp(siteDir->d_name, ".") || !strcmp(siteDir->d_name, ".."))
            continue;

        // Keep the full path
        dirpath = (char*)malloc(strlen(path) + strlen(siteDir->d_name) + 1);
        strcpy(dirpath,path);
        strcat(dirpath, siteDir->d_name);

        // Open current directory (website)
        if((sitePtr = opendir(dirpath)) == NULL){
            perror("opendir");
            free(dirpath);
            return -2;
        }

        // Read the files in the directory
        while((specFile = readdir(sitePtr)) != NULL){
            // Check for termination signal
            if(received_signal == 1){
                free(dirpath);
                closedir(sitePtr);
                return 1;
            }

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
                free(filepath);
                free(dirpath);
                closedir(sitePtr);
                return -3;
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
            // Check for termination signal
            if(properties == NULL){
                free(specID);
                free(filepath);
                fclose(specFd);
                free(dirpath);
                closedir(sitePtr);
                return 1;
            }

            // Create spec node
            mySpec      *newSpec = specCreate(specID, properties, propNum);
            myMatches   *newMatch = NULL;

            hash_add(*hashT, newSpec, hash1(newSpec->specID));
            newMatch = matchesAdd(allMatches, newSpec);
            updateSpecMatches(newSpec, newMatch);

            fclose(specFd);
            free(filepath);
            filepath = NULL;

            free(specID);
            for(int i=0; i<propNum; i++){
                specDelInfo(properties[i]);
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

    return 0;
}


specInfo** readFile(FILE *specFd, int *propNum, specInfo **properties){
    char    line[512];

    struct sigaction    act;
    sigset_t            block_mask;

    received_signal = 0;

    sigemptyset(&(act.sa_mask));
	act.sa_flags = 0;
    act.sa_handler = sig_int_quit_handler;

    sigemptyset(&block_mask);
	sigaddset(&block_mask,SIGINT);
	sigaddset(&block_mask,SIGQUIT);

    // Read spec's properties
    fgets(line, sizeof(line), specFd);
    while(line != NULL){
        // Check for termination signal
        if(received_signal == 1){
            for(int i=0; i<=(*propNum); i++){
                specDelInfo(properties[i]);
                free(properties[i]);
            }
            free(properties);

            return NULL;
        }

        // Ignore '{' and '}' lines in json file
        if(line[0] == '{'){
            fgets(line, sizeof(line), specFd);
            continue;
        }
        if( line[0] == '}')
            break;

        char    *rest = line;
        char    *token = NULL, *str = NULL, *prev = NULL, *temp = NULL;
        int     index = 0, str_len = 0, need_extra = 0, str_array = 0;

        // Seperate the key from the value in the ("key" : "value") pair
        while(index < 2){
            // Check for termination signal
            if(received_signal == 1){
                for(int i=0; i<=(*propNum); i++){
                    specDelInfo(properties[i]);
                    free(properties[i]);
                }
                free(properties);

                return NULL;
            }

            need_extra = 0;

            // Remove extra \" and , from the string
            while((token = strtok_r(rest, "\"", &rest)) != NULL){
                // Check for termination signal
                if(received_signal == 1){
                    if(str != NULL)
                        free(str);
                    for(int i=0; i<=(*propNum); i++){
                        specDelInfo(properties[i]);
                        free(properties[i]);
                    }
                    free(properties);

                    return NULL;
                }

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
                    str_array = 1;

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
                        // Check for termination signal
                        if(received_signal == 1){
                            if(str != NULL)
                                free(str);
                            for(int i=0; i<=(*propNum); i++){
                                specDelInfo(properties[i]);
                                free(properties[i]);
                            }
                            free(properties);

                            return NULL;
                        }

                        fgets(line, sizeof(line), specFd);
                        rest = line;

                        // Remove extra \" and , from the string
                        while((token = strtok_r(rest, "\"", &rest)) != NULL){
                            // Check for termination signal
                            if(received_signal == 1){
                                if(str != NULL)
                                    free(str);
                                for(int i=0; i<=(*propNum); i++){
                                    specDelInfo(properties[i]);
                                    free(properties[i]);
                                }
                                free(properties);

                                return NULL;
                            }

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
                    // Check for termination signal
                    if(received_signal == 1){
                        if(str != NULL)
                            free(str);
                        for(int i=0; i<=(*propNum); i++){
                            specDelInfo(properties[i]);
                            free(properties[i]);
                        }
                        free(properties);

                        return NULL;
                    }

                    fgets(line, sizeof(line), specFd);
                    rest = line;

                    // Remove extra \" and , from the string
                    while((token = strtok_r(rest, "\"", &rest)) != NULL){
                        // Check for termination signal
                        if(received_signal == 1){
                            if(str != NULL)
                                free(str);
                            for(int i=0; i<=(*propNum); i++){
                                specDelInfo(properties[i]);
                                free(properties[i]);
                            }
                            free(properties);

                            return NULL;
                        }

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

            // If it is the key
            if(index == 0){
                (*propNum)++;
                properties = realloc(properties, ((*propNum)+1)*sizeof(specInfo*));
                properties[*propNum] = malloc(sizeof(specInfo));
            }

            // Store the key
            if(index == 0){
                (properties[*propNum])->key = strdup(str);
                (properties[*propNum])->values = NULL;
            }
            else{
                // If the value string is an 'array' of strings, will keep a list of the values
                if(str_array){
                    temp = str;

                    specValue   *head = NULL, *current = NULL, *prev = NULL;
                    while((token = strtok_r(temp, "\n", &temp)) != NULL){
                        // Check for termination signal
                        if(received_signal == 1){
                            if(str != NULL)
                                free(str);
                            for(int i=0; i<=(*propNum); i++){
                                specDelInfo(properties[i]);
                                free(properties[i]);
                            }
                            free(properties);

                            return NULL;
                        }

                        if(!strcmp(token, "[") || !strcmp(token, "    ]") || !strcmp(token, "    ],"))
                            continue;

                        // Create a specValue node
                        current = malloc(sizeof(specValue));
                        current->value = strdup(token);
                        if(current->value[strlen(current->value)-1] == ',')
                            current->value[strlen(current->value)-1] = '\0';
                        current->next = NULL;

                        // If it is the first value
                        if(head == NULL)
                            head = current;
                        // If it is not the first value
                        if(prev != NULL)
                            prev->next = current;

                        prev = current;
                    }

                    (properties[*propNum])->values = head;
                }
                // If the value is a string
                else{
                    (properties[*propNum])->values = malloc(sizeof(specValue));
                    (properties[*propNum])->values->value = strdup(str);
                    (properties[*propNum])->values->next = NULL;
                }
            }
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

    struct sigaction    act;
    sigset_t            block_mask;

    received_signal = 0;

    sigemptyset(&(act.sa_mask));
	act.sa_flags = 0;
    act.sa_handler = sig_int_quit_handler;
	if(sigaction(SIGINT,&act,NULL) < 0 || sigaction(SIGQUIT,&act,NULL) < 0)
	{
		perror("sigaction");
		return -1;
	}
    sigemptyset(&block_mask);
	sigaddset(&block_mask,SIGINT);
	sigaddset(&block_mask,SIGQUIT);
    
            // READ FILE
    FILE* fpin = NULL;
    fpin = fopen(fName, "r+");

    if(fpin == NULL){
        printf("Can't Open CSV File !\n");
        fclose(fpin);
        return -2;
    }


    char line[100];
    memset(line, 0, 100);
    fgets(line, 100, fpin);
    memset(line, 0, 100);

    int count = 0;
    int failed = 0;
    int passed = 0;
    int skipped = 0;
    while(fgets(line, 100, fpin) != NULL){
        if(received_signal == 1){
            fclose(fpin);
            return 1;
        }
        
        // printf("count: %d\n", count);
                        // GET SPEC'S KEYS
        char* key1 = strtok(line, ",\n");
        char* key2 = strtok(NULL, ",\n");
        char* isMatch = strtok(NULL, ",\n");

                        // CHECK KEYS - UNCOMMENT FOR TESTING
        // printf("\tkey1: %s, key2: %s, isMatch: %s\n", key1, key2, isMatch);

        // printf("\tspec1: %s counts: %d, spec2: %s counts: %d\n", spec1->specID, spec1->matches->specsCount, spec2->specID, spec2->matches->specsCount);

                          // MERGE MATCHES + FIX POINTERS
                                // !! Check if match2 (to be deleted by merge)
                                // is head then swap them 
        if(strcmp(isMatch, "1") == 0){
                // SCAN HASH FOR ENTRIES
            mySpec *spec1 = findRecord_byKey(hashT, key1);
            mySpec *spec2 = findRecord_byKey(hashT, key2);

            if(spec2->matches == spec1->matches){
                // MATCHES ALREADY TOGETHER -> NO NEED TO MERGE
                memset(line, 0, 100);
                count++;
                continue;
            }
            else{
                if(swapSpecsMatches(spec1, spec2)){
                    mergeMatches(allMatches, spec1->matches, spec2->matches);
                    spec2->matches = spec1->matches;
                    passed++;
                }
                else
                    failed++;
                
            }
        }
        else
        {
            skipped++;
        }
        

        memset(line, 0, 100);

        count++;
    }
    
    fclose(fpin);

    // Uncomment to print stats
    // printf ("\n\t(total: %d, skipped: %d, failed: %d, passed: %d)\n\t", count, skipped, failed, passed);

    return 0;
}

int swapSpecsMatches(mySpec* dest, mySpec* source){

    if(dest == NULL || source == NULL)
        return -1;

    // printf("\tSWAPING VALUES ... \n");

    int i = 0;

    while(i < source->matches->specsCount){
        // printf("\t\t\t i: %d\n", i);
        if(source->matches->specsTable[i] != source)
            source->matches->specsTable[i]->matches = dest->matches;
        i++;
    }

    return 1;
    // printf("\t\t .. DONE !!\n");

}


void sig_int_quit_handler(int signo)
{
	if(signo == SIGINT || signo == SIGQUIT)
		received_signal = 1;
}