#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <signal.h>
#include <ctype.h>
#include "../include/functs.h"
#include "../include/mySpec.h"
#include "../include/myHash.h"
#include "../include/boWords.h"


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

    // ~~~~~~~~~~~~~~~~~~ READ CSV FILE + FILL MATCHES STRUCT
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
    
        //~~~~~~~~~~~~~~~~~~ OPEN FILE
    FILE* fpin = NULL;
    fpin = fopen(fName, "r+");

    if(fpin == NULL){
        printf("Can't Open CSV File !\n");
        fclose(fpin);
        return -2;
    }

        // ~~~~~~~~~~~~~~~~~ SET MRU LIST
    // MRU_info* mruL = create_MRU(MRU_SIZE);

        // ~~~~~~~~~~~~~~~~~ READ FILE
    char line[100];
    memset(line, 0, 100);
    fgets(line, 100, fpin);
    memset(line, 0, 100);

    int count = 0;
    int failed = 0;
    int passed = 0;
    int skipped = 0;
    while(fgets(line, 100, fpin) != NULL){
        // printf("count: %d\n", count);
        if(received_signal == 1){
            fclose(fpin);
            return 1;
        }
        
                    //~~~~~~~~~ GET CSV INFO-KEYS
        char* key1 = strtok(line, ",\n");
        char* key2 = strtok(NULL, ",\n");
        char* isMatch = strtok(NULL, ",\n");

                    // SCAN HASH FOR ENTRIES
        mySpec *spec1 = findRecord_byKey(hashT, key1);
        mySpec *spec2 = findRecord_byKey(hashT, key2);

        if(spec1 == NULL || spec2 == NULL){
            failed++;
            count++;
            memset(line, 0, 100);
            continue;
        }

                    //~~~~~~~~~ CHECK KEYS - UNCOMMENT FOR TESTING
        // printf("\tkey1: %s, key2: %s, isMatch: %s\n", key1, key2, isMatch);

        // printf("\tspec1: %s counts: %d, spec2: %s counts: %d\n", spec1->specID, spec1->matches->specsCount, spec2->specID, spec2->matches->specsCount);

                    //~~~~~~~~~ MERGE MATCHES + FIX POINTERS
        if(strcmp(isMatch, "1") == 0){  //case keys r match

            if(spec2->matches == spec1->matches){
                // MATCHES ALREADY TOGETHER -> NO NEED TO MERGE
                memset(line, 0, 100);
                count++;
                continue;
            }
            else{       //~~~~~~~~ !!! FIX POINTERS SPECS -> MATCHES
                if(swapSpecsMatches(spec1, spec2)){ // ~~~~ MERGE MATCHES ARRAYS
                    mergeMatches(allMatches, spec1->matches, spec2->matches);
                    spec2->matches = spec1->matches;
                    passed++;
                }
                else
                    failed++;
                
            }
        }
        else if(strcmp(isMatch, "0") == 0){
            // printf("~~~ spec_1: %s, spec_2: %s ~~~\n", spec1->specID, spec2->specID);
            updateNegativeMatches(spec1->matches, spec2->matches);
        }

        else{
            skipped++;
        }

        memset(line, 0, 100);

        count++;
    }

    fclose(fpin);

    // Uncomment to print stats
    // printf ("\n\t(total: %d, skipped: %d, failed: %d, passed: %d)\n\t", count, skipped, failed, passed);

    // printMatchNeg(allMatches);

    return 0;
}


void text_to_bow(mySpec** set, int setSize, BoWords** boWords){

    for(int i=0; i<setSize; i++){
        spec_to_bow(set[i], *boWords);
    }
}

void spec_to_bow(mySpec* spec, BoWords* boWords){
    char*   sentence = NULL;

    for(int i=0; i<(spec->propNum); i++){
        sentence = strdup(spec->properties[i]->key);
        sentence_to_bow(sentence, spec, boWords);

        free(sentence);
        sentence = NULL;

        specValue*   currVal = spec->properties[i]->values;
        while(currVal != NULL){
            sentence = strdup(currVal->value);
            sentence_to_bow(sentence, spec, boWords);

            free(sentence);
            sentence = NULL;

            currVal = currVal->next;
        }
    }
}

void sentence_to_bow(char* sentence, mySpec* spec, BoWords* boWords){
    int     hash = 0;
    char*   word = NULL;

    while((word = strtok_r(sentence," ",&sentence)) != NULL){
        word = checkWord(word);
        if(word != NULL){
            hash = hash1(word);
            bow_add(boWords, word, spec, hash);
            (spec->numofWords)++;

            free(word);
            word = NULL;
        }
    }
}

char* checkWord(char* word){

    char*   result = strdup(word);

    for(int i=0; i<strlen(word); i++){
        // If it is not a digit or a letter
        if(isalnum(word[i]) == 0){
            free(result);
            return NULL;
        }

        // Turn to lower case
        result[i] = (char)tolower(word[i]);
    }

    char stopwords[] = "a able about across after all almost also am among an and any are as at be because been but by can cannot could dear did do does either else ever every for from get got had has have he her hers him his how however i if in into is it its just least let like likely may me might most must my neither no nor not of off often on only or other our own rather said say says she should since so some than that the their them then there these they this tis to too twas us wants was we were what when where which while who whom why will with would yet you your";

    // If it is a stopword
    if(strstr(stopwords, result) != NULL){
        free(result);
        return NULL;
    }

    return result;
}


void sig_int_quit_handler(int signo)
{
	if(signo == SIGINT || signo == SIGQUIT)
		received_signal = 1;
}
