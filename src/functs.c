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
#include "../include/mbh.h"

int received_signal = 0;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ READ DATA ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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


int readCSV(char* fName, hashTable* hashT, matchesInfo* allMatches, float percentage, long int* offset){
    // ~~~~~~~~~~~~~~~~~~ READ CSV FILE + FILL MATCHES STRUCT

    // Signal handling
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

        // ~~~~~~~~~~~~~~~~~ GET FILE SIZE
    fseek(fpin, 0, SEEK_END);
    long int totalBytes = ftell(fpin);
    fseek(fpin, 0, SEEK_SET);

        // ~~~~~~~~~~~~~~~~~ SET MRU LIST
    // MRU_info* mruL = create_MRU(MRU_SIZE);

        // ~~~~~~~~~~~~~~~~~ READ FILE
    char line[100];
    memset(line, 0, 100);
    fgets(line, 100, fpin);
    memset(line, 0, 100);

    // Ignore first line's size
    totalBytes -= ftell(fpin);
    // Number of bytes to be read
    long int readBytes = percentage*totalBytes;
    // Number of bytes in current line
    long int lineBytes = ftell(fpin);
    // Number of lines read
    int      lines = 0;

    int count = 0;
    int failed = 0;
    int passed = 0;
    int skipped = 0;
    // Until expected number of bytes is read or end of file is reached
    while(readBytes > 0 && fgets(line, 100, fpin) != NULL){
        lines++;
        // If current line has more bytes than the amount that is to be read
        if((ftell(fpin) - lineBytes) > readBytes)
            break;

        if(received_signal == 1){
            fclose(fpin);
            return -3;
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

        // Update number of bytes left to be read
        lineBytes = ftell(fpin) - lineBytes;
        readBytes -= lineBytes;
        lineBytes = ftell(fpin);
    }

    // Keep the point in file where the reading stopped
    *offset = ftell(fpin);
    fclose(fpin);

    // Uncomment to print stats
    // printf ("\n\t(total: %d, skipped: %d, failed: %d, passed: %d)\n\t", count, skipped, failed, passed);

    // printMatchNeg(allMatches);

    return lines;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ SETS OF SPECS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

mySpec** get_trainSet(matchesInfo* allMatches, int* trainSize){

    mySpec** trainSet = NULL;
    *trainSize = 0;

    myMatches* match = allMatches->head;
    while(match != NULL){
        // If current clique was not in dataset W
        if(match->specsCount == 1){
            match = match->next;
            continue;
        }

        for(int j=0; j<match->specsCount; j++){
            (*trainSize)++;
            trainSet = realloc(trainSet, (*trainSize)*sizeof(mySpec*));
            trainSet[*trainSize - 1] = match->specsTable[j];
        }

        match = match->next;
    }

    return trainSet;
}

mySpec** get_testSet(char* path, hashTable* hashT, int* testSize, long int* offset, int lines){
    return get_set(path, hashT, testSize, offset, lines, 't');
}

mySpec** get_validationSet(char* path, hashTable* hashT, int* testSize, long int* offset, int lines){
    return get_set(path, hashT, testSize, offset, lines, 'v');
}

mySpec** get_set(char* path, hashTable* hashT, int* size, long int* offset, int lines, char setType){

    FILE*   fp = fopen(path, "r");
    if(fp == NULL){
        perror("fopen");
        return NULL;
    }

    mySpec* spec = NULL;
    mySpec** set = NULL;
    *size = 0;

    // Start reading from where it stopped before
    fseek(fp, *offset, SEEK_SET);

    char    line[100];
    // Until expected number of lines is read or end of file is reached
    while(lines > 0 && fgets(line, sizeof(line), fp) != NULL){
        // If it is the testing set, read the expected number of lines
        // If it is the validation set, read all the lines left in dataset W
        if(setType == 't' || (setType == 'v' && lines > 1))
            lines--;
        
        // Get csv info-keys
        char* key1 = strtok(line, ",\n");
        char* key2 = strtok(NULL, ",\n");

        // Find spec with key1 in hash table, return it only if it has not been already used in the set
        spec = findRecord_forSet(hashT, key1, setType);
        // If the spec was found, add it to the set
        if(spec != NULL){
            (*size)++;
            set = realloc(set, (*size)*sizeof(mySpec*));
            set[*size - 1] = spec;
        }

        // Find spec with key2 in hash table, return it only if it has not been already used in the set
        spec = findRecord_forSet(hashT, key2, setType);
        // If the spec was found, add it to the set
        if(spec != NULL){
            (*size)++;
            set = realloc(set, (*size)*sizeof(mySpec*));
            set[*size - 1] = spec;
        }
    }

    // Keep the point in file where the reading stopped
    *offset = ftell(fp);
    fclose(fp);

    return set;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ BAG OF WORDS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// Turn a set of texts to bow
void text_to_bow(mySpec** set, int setSize, BoWords** boWords){
    // For each spec of the set (either training or testing)
    for(int i=0; i<setSize; i++){
        spec_to_bow(set[i], *boWords);
    }
    (*boWords)->specsSum = setSize;
}

// Turn a text to bow
void spec_to_bow(mySpec* spec, BoWords* boWords){
    char*   sentence = NULL;

    // For each property of the spec
    for(int i=0; i<(spec->propNum); i++){
        // Add key's words to bow hashtable
        sentence = strdup(spec->properties[i]->key);
        sentence_to_bow(sentence, spec, boWords);

        free(sentence);
        sentence = NULL;

        specValue*   currVal = spec->properties[i]->values;
        while(currVal != NULL){
            // Add each value's words to bow hashtable
            sentence = strdup(currVal->value);
            sentence_to_bow(sentence, spec, boWords);

            free(sentence);
            sentence = NULL;

            currVal = currVal->next;
        }
    }
}

// Turn a sentence to bow
void sentence_to_bow(char* sentence, mySpec* spec, BoWords* boWords){
    int     hash = 0;
    char*   word = NULL;

    // For each word in the sentence
    while((word = strtok_r(sentence," ",&sentence)) != NULL){
        // Check if the word should be used for the matching
        word = checkWord(word);
        // If it should, add it to bow hashtable
        if(word != NULL){
            hash = hash1(word);
            bow_add(boWords, word, spec, hash);
            // Increase the number of words found in current spec
            (spec->numofWords)++;

            free(word);
            word = NULL;
        }
    }
}

// Check if a word should be added in bow
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

// Mark the most significant words in bow
void set_mostSignificantWords(BoWords* bow, int mostSign){
    char*   word = NULL;
    MBH*    heap = NULL;

    mbh_init(&heap, mostSign);

    bow_get_signWords(bow, heap);

    while((word = mbh_extract_root(heap)) != NULL){
        bow_set_significance(bow, word);
        free(word);
    }

    mbh_delete(&heap);
}

// Remove all insignificant words from bow
void keep_mostSignificantWords(BoWords* bow){

    bow_keep_signWords(bow);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ TRAINING ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

logM** make_models_array(BoWords* bow, mySpec** set, matchesInfo* matches, int set_size){
    
    // make array
    logM** modelsT = malloc(matches->entries*sizeof(logM*));

    // train_per_clique
    myMatches* clique = matches->head;
    int i = 0;
    while(clique != NULL){

        modelsT[i] = logistic_create();
        // if(i < 500){
            train_per_clique(clique, set, set_size, bow, modelsT[i]);
            
            printf("Clique %d trained Succefully %d times !!\n", i, modelsT[i]->trained_times);
        // }
        i++;
        clique = clique->next;

        // ABORT METHOD
        // if(modelsT[i-1]->trained_times < 2){
        //     printf("Aborting ..\n");
        //     while(i > 0){
        //         logistic_destroy(modelsT[--i]);
        //     }
        //     free(modelsT);
        //     return NULL;
        // }
    }

    printf("Trained for %d different Matches Groups !!\n", i);

    return modelsT;
}

logM* make_model_vec(BoWords* bow, mySpec** train_set, int set_size){

    // Signal handling
    struct sigaction    act;
    sigset_t            block_mask;
    received_signal = 0;
    sigemptyset(&(act.sa_mask));
	act.sa_flags = 0;
    act.sa_handler = sig_int_quit_handler;
    sigemptyset(&block_mask);
	sigaddset(&block_mask,SIGINT);
	sigaddset(&block_mask,SIGQUIT);

    // Create & init model
    logM* model = logistic_create();

    // Train_per_Spec
    int check = train_per_spec_vec(train_set, set_size, bow, model);

    // If a termination signal was recieved
    if(received_signal == 1 || check == -1){
        logistic_destroy(model);
        return NULL;
    }

    return model;
}

logM* make_model_spars(BoWords* bow, mySpec** train_set, int set_size){

    // Signal handling
    struct sigaction    act;
    sigset_t            block_mask;
    received_signal = 0;
    sigemptyset(&(act.sa_mask));
	act.sa_flags = 0;
    act.sa_handler = sig_int_quit_handler;
    sigemptyset(&block_mask);
	sigaddset(&block_mask,SIGINT);
	sigaddset(&block_mask,SIGQUIT);

    // Create & init model
    logM* model = logistic_create();

    // Train_per_Spec
    int check = train_per_spec_spars(train_set, set_size, bow, model);

    // If a termination signal was recieved
    if(received_signal == 1 || check == -1){
        logistic_destroy(model);
        return NULL;
    }

    return model;
}


float* vectorization(mySpec* spec, BoWords* bow, int* vectorSize){

    // printf("mpjhke\n");
    *vectorSize = 0;
    float*  vector = malloc((bow->entries)*sizeof(float));

    // Get a vector of words' calculated values about a text (spec)
    bow_vectorize(bow,&vector,vectorSize,spec);

    return vector;
}

float** make_it_spars(mySpec** set, int set_size, BoWords* bow, int* spars_size, int** labels, int* labels_size){
    *spars_size = 0;
    *labels_size = 0;
    float** spars = NULL;
    int row = 0;
    int col = 0;

    float*** all_spars = malloc(set_size*sizeof(float**));
    int* all_spars_sizes = malloc(set_size*sizeof(int));
    int i = 0;
    while(i < set_size){
        col = 0;
        all_spars[i] = NULL;
        all_spars_sizes[i] = 0;
        bow_to_spars(bow, &all_spars[i], &all_spars_sizes[i], &row, &col, set[i]);
        i++;
    }

    i = 0;
    while(i<set_size){
        int z = i + 1;
        while(z < set_size){
            if(all_spars[i] == NULL && all_spars[z] == NULL){
                z++;
                continue;           
            }
            int tag = isPair(set[i], set[z]);
            if(tag != -1){
                float** temp = spars_concat_col(all_spars[i], all_spars[z], all_spars_sizes[i], all_spars_sizes[z], bow->entries);
                int temp_size = all_spars_sizes[z] + all_spars_sizes[i];
                spars_concat_row(&spars, temp, spars_size, temp_size, row);

                (*labels) = realloc((*labels), ((*labels_size)+1)*sizeof(int));
                (*labels)[*labels_size] = tag;
                (*labels_size) = (*labels_size) + 1;

                row++;
                while(temp_size > 0){
                    free(temp[--temp_size]);
                }
                free(temp);
            }
            z++;
        }
        i++;
    }
    // printf("spars_size: %d\n", *spars_size);
    // printf("s0 %.4f, %.4f, %.4f\n", spars[0][0], spars[0][1], spars[0][2]);
    
    int i1 = 0;
    while(i1 < set_size){
        int i2 = 0;
        while(i2 < all_spars_sizes[i1]){
            free(all_spars[i1][i2++]);
        }
        free(all_spars[i1++]);
    }
    free(all_spars);
    free(all_spars_sizes);

    return spars;
}

void print_spars(float** spars, int size){
    int i = 0;
    printf("spars_size: %d\n", size);
    while(i < size){
        printf("s%d [0]: %.4f, [1]: %.4f, [2]: %.4f\n", i, spars[i][0], spars[i][1], spars[i][2]);
        i++;
    }
}

int train_per_spec_vec(mySpec** train_set, int set_size, BoWords* bow, logM* model){

    // Signal handling
    struct sigaction    act;
    sigset_t            block_mask;
    received_signal = 0;
    sigemptyset(&(act.sa_mask));
	act.sa_flags = 0;
    act.sa_handler = sig_int_quit_handler;
    sigemptyset(&block_mask);
	sigaddset(&block_mask,SIGINT);
	sigaddset(&block_mask,SIGQUIT);

        //  Init values to pass
    int vector_cols = 0;
    int count_pairs = 0;

    float** all_vectors = NULL;
    float** pairsVector = NULL;
    int* labels = NULL;

    make_vectors(train_set, set_size, bow, &pairsVector, &all_vectors, &labels, &count_pairs, &vector_cols);

    if(received_signal == 1){
        //  FREE MEM
        while(count_pairs > 0)
            free(pairsVector[--count_pairs]);
        free(pairsVector);
        for(int i=0; i < set_size; )
            free(all_vectors[i++]);
        free(all_vectors);
        free(labels);
        return -1;
    }

    /*       !!!        EXPLAIN STRUCTS - VARS        !!!
    pairsVector = { {}-{}, ..., {}-{} } -> all Vectors by pairs
    labels = {tag, tag, ..., tag} -> all pairs tags
    count_labales = Sum of Labels - Must be equal to count_pairs
    count_pairs = Sum of every possible pair found - Equal to count_labels
    vectorSise = Sum of dimensions - pairesVector's num of cols
    */


    // TRAIN MODEL
    int check = logistic_fit(model, count_pairs, 2*vector_cols, pairsVector, labels);

    if(received_signal == 1 || check == -1){
        //  FREE MEM
        while(count_pairs > 0)
            free(pairsVector[--count_pairs]);
        free(pairsVector);
        for(int i=0; i < set_size; )
            free(all_vectors[i++]);
        free(all_vectors);
        free(labels);
        return -1;
    }

    // PRINT STATS FOR TESTING
    printf("trained times: %d\n", model->trained_times);

    //  FREE MEM
    while(count_pairs > 0){
        free(pairsVector[--count_pairs]);
    }

    free(pairsVector);

    int i = 0;
    while(i < set_size){
        free(all_vectors[i++]);
    }
    free(all_vectors);

    free(labels);

    return 0;
}

float** spars_concat_col(float** spar1, float** spar2, int size1, int size2, int to_add){
    float** spar3 = malloc((size1+size2)*sizeof(float*));
    int i = 0;
    while(i < size1+size2){
        spar3[i] = malloc(3*sizeof(float));
        if(i < size1){
            spar3[i][0] = spar1[i][0];
            spar3[i][1] = spar1[i][1];
            spar3[i][2] = spar1[i][2];
        }
        else{
            spar3[i][0] = spar2[i-size1][0];
            spar3[i][1] = spar2[i-size1][1] + (float) to_add;
            spar3[i][2] = spar2[i-size1][2];
        }
        i++;
    }
    return spar3;
}

void spars_concat_row(float*** spar1, float** spar2, int* size1, int size2, int new_row){
    // printf("cur_row: %d, size1: %d, size2: %d\n", new_row, *size1, size2);

        (*spar1) = realloc(*spar1, ((*size1)+size2)*sizeof(float*));

    int i = 0;
    while(i < size2){
        (*spar1)[(*size1)+i] = malloc(3*sizeof(float));

        (*spar1)[(*size1)+i][0] = (float) new_row;
        (*spar1)[(*size1)+i][1] = spar2[i][1];
        (*spar1)[(*size1)+i][2] = spar2[i][2];        

        i++;
    }

    (*size1) = (*size1) + size2;
}

int train_per_spec_spars(mySpec** train_set, int set_size, BoWords* bow, logM* model){

    // Signal handling
    struct sigaction    act;
    sigset_t            block_mask;
    received_signal = 0;
    sigemptyset(&(act.sa_mask));
	act.sa_flags = 0;
    act.sa_handler = sig_int_quit_handler;
    sigemptyset(&block_mask);
	sigaddset(&block_mask,SIGINT);
	sigaddset(&block_mask,SIGQUIT);

        //  Init values to pass
    float** spars = NULL;
    int spars_size = 0;
    int* labels = NULL;
    int labels_size;

    // printf("!1\n");
    spars = make_it_spars(train_set, set_size, bow, &spars_size, &labels, &labels_size);
    int size = spars_size;
    // printf("s0, %.4f, %.4f, %.4f\n", spars[0][0], spars[0][1], spars[0][2]);
    // print_spars(spars, spars_size);
    // printf("!2\n");

    if(received_signal == 1){
        //  FREE MEM;
        for(int i=0; i < spars_size; )
            free(spars[i++]);
        free(labels);
        return -1;
    }

    /*       !!!        EXPLAIN STRUCTS - VARS        !!!
    pairsVector = { {}-{}, ..., {}-{} } -> all Vectors by pairs
    labels = {tag, tag, ..., tag} -> all pairs tags
    count_labales = Sum of Labels - Must be equal to count_pairs
    count_pairs = Sum of every possible pair found - Equal to count_labels
    vectorSise = Sum of dimensions - pairesVector's num of cols
    */


    // TRAIN MODEL
    int check = logistic_fit_spars(model, spars_size, spars, labels, labels_size, 2*bow->entries);

    if(received_signal == 1 || check == -1){
        //  FREE MEM;
        while(spars_size > 0){
            free(spars[--spars_size]);
        }
        free(spars);

        free(labels);
        return -1;
    }

    // PRINT STATS FOR TESTING
    printf("\ttrained times: %d\n", model->trained_times);

    //  FREE MEM
    while(size > 0){
        free(spars[--size]);
    }

    free(spars);


    free(labels);

    return 0;
}


void make_tests(BoWords* bow, logM* model, mySpec** test_set, int set_size){

            //  Init values to pass
    int vector_cols = 0;
    int count_pairs = 0;

    float** all_vectors = NULL;
    float** pairsVector = NULL;
    int* labels = NULL;

    make_vectors(test_set, set_size, bow, &pairsVector, &all_vectors, &labels, &count_pairs, &vector_cols);

    int* predicts = logistic_predict(model, pairsVector, count_pairs, vector_cols);
    printf("\tAccuracy at test_set: %.4f\n", logistic_score(model, predicts, labels, count_pairs));

        //  FREE MEM
    while(count_pairs > 0){
        free(pairsVector[--count_pairs]);
    }

    free(pairsVector);

    int i = 0;
    while(i < set_size){
        free(all_vectors[i++]);
    }
    free(all_vectors);

    free(predicts);
    free(labels);
}

void make_tests_spars(BoWords* bow, logM* model, mySpec** test_set, int set_size){

        //  Init values to pass
    float** spars = NULL;
    int spars_size = 0;
    int* labels = NULL;
    int labels_size;


    spars = make_it_spars(test_set, set_size, bow, &spars_size, &labels, &labels_size);
    int size = spars_size;
    int* predicts = logistic_predict_spars(model, spars, spars_size, 2*bow->entries, labels_size);

    printf("\tAccuracy at test_set: %.4f\n", logistic_score(model, predicts, labels, labels_size));

        //  FREE MEM
    while(size > 0){
        free(spars[--size]);
    }

    free(spars);


    free(labels);

    free(predicts);
}

void make_vectors(mySpec** set, int set_size, BoWords* bow, float*** pairsVector, float*** all_vectors, int** labels, int* count_pairs, int* vector_cols){
        //  INIT PASSED VALUES IN CASE THEY ARE NOT !
    *vector_cols = 0;
    *pairsVector = NULL;
    *labels = NULL;
    *count_pairs = 0;
    *all_vectors = NULL;

        // Create vectors for all specs to be passed
    *all_vectors = malloc(set_size*sizeof(float*));
    int make_vectors = 0;
    while(make_vectors < set_size){
        (*all_vectors)[make_vectors] = vectorization(set[make_vectors], bow, vector_cols);
        make_vectors++;
    }


        // Pass all specs by pairs (All with All (no doubles) )

    int tag = -1;

    int passed_specs = 0;
    while(passed_specs < set_size){

        int check_specs = passed_specs + 1;
        while(check_specs < set_size){

            tag = isPair(set[passed_specs], set[check_specs]);
            if(tag != -1){
                float* combined = concat_specVectors((*all_vectors)[passed_specs], (*all_vectors)[check_specs], *vector_cols);
                *pairsVector = concat_pairsVectors(*pairsVector, combined, *count_pairs);
                
                *labels = concat_tags(*labels, tag, *count_pairs);
                
                (*count_pairs) += 1;
            }
            check_specs++;
        }
        passed_specs++;
    }
}

int isPair(mySpec* spec1, mySpec* spec2){
            // CHECK IF THEY ARE IN THE SAME MATCH > RETURN 1
    myMatches* match = spec1->matches;
    int i = 0;
    while(i < match->specsCount){
        if(match->specsTable[i] == spec2)
            return 1;
        i++;
    }

            // CHECK IF THEY ARE NEGATIVES  > RETURN 0
    i = 0;
    nNode* node = match->negs->head;

    while(node != NULL){
        int check_negs = 0;
        while(check_negs < node->matchptr->specsCount){
            if(node->matchptr->specsTable[check_negs] == spec2)
                return 0;
            check_negs++;
        }
        node = node->next;
    }

            // ELSE > RETURN -1
    return -1;
}

int* concat_tags(int* table, int tag, int size){
    table = realloc(table, (size+1)*sizeof(int));
    table[size] = tag;
    return table;
}

float* concat_specVectors(float* vec1, float* vec2, int size){
        // find which is bigger
    float sum1 = 0.0;
    float sum2 = 0.0;
    
    int i = 0;
    while(i < size){
        sum1 += vec1[i];
        sum2 += vec2[i];
        i++;
    }

    int place1 = 0;
    int place2 = size;

    if(sum1 < sum2){    //swap
        place1 = size;
        place2 = 0;
    }

    float* newVec = malloc(2*size*sizeof(float));

    memcpy(&newVec[place1], vec1, size*sizeof(float));
    memcpy(&newVec[place2], vec2, size*sizeof(float));

    return newVec;
}

float** concat_pairsVectors(float** main_vec, float* to_add, int size){
    main_vec = realloc(main_vec, (size+1)*sizeof(float*));
    main_vec[size] = to_add;

    return main_vec;
}

void train_per_clique(myMatches* clique, mySpec** trainSet, int trainSize, BoWords* bow, logM* model){
    int     tag , vectorSize;
    float**  vector = NULL;
    nNode*  negMatch = NULL;

    //tags array
    int* labels = malloc(trainSize*sizeof(int));
    
    // vectors array
    vector = malloc(trainSize*sizeof(float*));
    int specs_in = 0;   // measures true vector_size

    // gather qlique's specs
    for(int i=0; i<trainSize; i++){
        tag = -1;
        if(trainSet[i]->matches == clique){
            tag = 1;
        }
        else{
            negMatch = trainSet[i]->matches->negs->head;
            while(negMatch != NULL){
                if(negMatch->matchptr == clique){
                    tag = 0;
                    break;
                }

                negMatch = negMatch->next;
            }
        }
        if(tag == -1)
            continue;

        labels[specs_in] = tag;
        vectorSize = 0;
        vector[specs_in++] = vectorization(trainSet[i],bow,&vectorSize);
        
 
    }

    // pass clique, vector, tag to the model !!
    // logistic_fit(model, specs_in, vectorSize, vector, labels, clique);

    // FREE MEM
    while(specs_in > 0){
        free(vector[--specs_in]);
    }
    free(vector);

    free(labels);

}

// ~~~~~~~~~~~~~~~~~~~~~~~~ SIGNALS ~~~~~~~~~~~~~~~~~~~~~~~~~~

void sig_int_quit_handler(int signo)
{
	if(signo == SIGINT || signo == SIGQUIT)
		received_signal = 1;
}
