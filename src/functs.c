#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <signal.h>
#include <ctype.h>
#include <sys/stat.h>
#include <errno.h>
#include <error.h>
#include <unistd.h>
#include <pthread.h>
#include "../include/functs.h"
#include "../include/mySpec.h"
#include "../include/myHash.h"
#include "../include/boWords.h"
#include "../include/mbh.h"

#define PATH "./Outputs"

#define MAX_TRAIN_SIZE_PER_THREAD 500
#define MAX_TRAIN_SIZE_PER_THREAD_TEST 100

int received_signal = 0;

pthread_mutex_t mtx_print = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t plz = PTHREAD_MUTEX_INITIALIZER;


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

char* shuffleCSV(char* path){
    char extension[5];
    char* shuffled_name = malloc(strlen(path) + strlen("_shuffled") + 5);

    // Temporarily create a shuffled csv file
    strcpy(shuffled_name, path);
    for(int i=4; i>=0; i--)
        extension[4-i] = shuffled_name[strlen(shuffled_name)-i];
    shuffled_name[strlen(shuffled_name)-4] = '\0';
    strcat(shuffled_name, "_shuffled");
    strcat(shuffled_name, extension);

    // Call bash command to omit csv's first line with info
    // command: tail -n +2 old_csv > new_csv
    char* command = malloc(strlen("tail -n +2 ") + strlen(path) + strlen(" > ") + strlen(shuffled_name) + 1);
    strcpy(command, "tail -n +2 ");
    strcat(command, path);
    strcat(command, " > ");
    strcat(command, shuffled_name);

    if(system(command) == -1){
        perror("system");
        free(shuffled_name);
        free(command);
        return NULL;
    }

    free(command);
    // Call bash command to shuffle the old csv file and store it in the new one
    // command: shuf new_csv -o new_csv
    command = malloc(strlen("shuf ") + strlen(shuffled_name) + strlen(" -o ") + strlen(shuffled_name) + 1);
    strcpy(command, "shuf ");
    strcat(command, shuffled_name);
    strcat(command, " -o ");
    strcat(command, shuffled_name);

    if(system(command) == -1){
        perror("system");
        free(shuffled_name);
        free(command);
        return NULL;
    }

    free(command);

    return shuffled_name;
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
        if(strcmp(isMatch, "1") == 0){  //case keys match

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

mySpec** get_testSet(char* path, hashTable* hashT, int* testSize, long int* offset, int lines, matchesInfo* allMatches){
    return get_set(path, hashT, testSize, offset, lines, 't', allMatches);
}

mySpec** get_validationSet(char* path, hashTable* hashT, int* testSize, long int* offset, int lines, matchesInfo* allMatches){
    return get_set(path, hashT, testSize, offset, lines, 'v', allMatches);
}

mySpec** get_set(char* path, hashTable* hashT, int* size, long int* offset, int lines, char setType, matchesInfo* allMatches){

    FILE*   fp = fopen(path, "r");
    if(fp == NULL){
        perror("fopen");
        return NULL;
    }

    mySpec* spec1 = NULL;
    mySpec* spec2 = NULL;
    mySpec** set = NULL;
    *size = 0;

    // Start reading from where it stopped before
    fseek(fp, *offset, SEEK_SET);

    int count = 0;
    int failed = 0;
    int passed = 0;
    int skipped = 0;
    char line[100];
    // Until expected number of lines is read or end of file is reached
    while(lines > 0 && fgets(line, sizeof(line), fp) != NULL){
        // If it is the testing set, read the expected number of lines
        // If it is the validation set, read all the lines left in dataset W
        if(setType == 't' || (setType == 'v' && lines > 1))
            lines--;
        
        // Get csv info-keys
        char* key1 = strtok(line, ",\n");
        char* key2 = strtok(NULL, ",\n");
        char* isMatch = strtok(NULL, ",\n");

	if(key1 == NULL || key2 == NULL || isMatch == NULL)
	    continue;

        // Find spec with key1 in hash table, return it only if it has not been already used in the set
        spec1 = findRecord_forSet(hashT, key1, setType);
        // If the spec was found, add it to the set
        if(spec1 != NULL){
           (*size)++;
            set = realloc(set, (*size)*sizeof(mySpec*));
            set[*size - 1] = spec1;
        }

        // Find spec with key2 in hash table, return it only if it has not been already used in the set
        spec2 = findRecord_forSet(hashT, key2, setType);
        // If the spec was found, add it to the set
        if(spec2 != NULL){
            (*size)++;
            set = realloc(set, (*size)*sizeof(mySpec*));
            set[*size - 1] = spec2;
        }

        if(spec1 == NULL || spec2 == NULL){
            failed++;
            count++;
            continue;
        }

                    //~~~~~~~~~ MERGE MATCHES + FIX POINTERS
        if(strcmp(isMatch, "1") == 0){  //case keys match
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

        count++;
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

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~ VECTORS METHOD ~~~~~~~~~~~~~~~~~~~~~~~~~~~
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

float* vectorization(mySpec* spec, BoWords* bow, int* vectorSize){

    // printf("mpjhke\n");
    *vectorSize = 0;
    float*  vector = malloc((bow->entries)*sizeof(float));

    // Get a vector of words' calculated values about a text (spec)
    bow_vectorize(bow,&vector,vectorSize,spec);

    return vector;
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

int* concat_tags(int* table, int tag, int size){
    table = realloc(table, (size+1)*sizeof(int));
    table[size] = tag;
    return table;
}

float make_tests(BoWords* bow, logM* model, mySpec** test_set, int set_size){

            //  Init values to pass
    int vector_cols = 0;
    int count_pairs = 0;

    float** all_vectors = NULL;
    float** pairsVector = NULL;
    int* labels = NULL;

    make_vectors(test_set, set_size, bow, &pairsVector, &all_vectors, &labels, &count_pairs, &vector_cols);

    int* predicts = logistic_predict(model, pairsVector, count_pairs, vector_cols);
    // printf("\tAccuracy at test_set: %.4f\n", logistic_score(model, predicts, labels, count_pairs));
    float acc =  logistic_score(model, predicts, labels, count_pairs);


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

    return acc;
}


// ~~~~~~~~~~~~~~~~~~~~~~~~ SPARS METHOD ~~~~~~~~~~~~~~~~~~~~~~~~~

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
    // printf("\ttrained times: %d\n", model->trained_times);

    //  FREE MEM
    while(size > 0){
        free(spars[--size]);
    }

    free(spars);


    free(labels);

    return 0;
}


float make_tests_spars(BoWords* bow, logM* model, mySpec** test_set, int set_size){

        //  Init values to pass
    float** spars = NULL;
    int spars_size = 0;
    int* labels = NULL;
    int labels_size;


    spars = make_it_spars(test_set, set_size, bow, &spars_size, &labels, &labels_size);
    int size = spars_size;
    int* predicts = logistic_predict_spars(model, spars, spars_size, 2*bow->entries, labels_size);

    // printf("\tAccuracy at test_set: %.4f\n", logistic_score(model, predicts, labels, labels_size));
    float acc = logistic_score(model, predicts, labels, labels_size);

        //  FREE MEM
    while(size > 0){
        free(spars[--size]);
    }

    free(spars);


    free(labels);

    free(predicts);

    return acc;
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


// ~~~~~~~~~~~~~~~~~~~~~~~ DATA_LIST METHOD ~~~~~~~~~~~~~~~~~~~~~~

logM* make_model_spars_list(hashTable* hashT, BoWords* bow, mySpec** train_set, int set_size, jobSch* Scheduler){

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
    // int check = train_per_spec_spars_list(train_set, set_size, bow, model);
    // int check = train_per_spec_spars_list_one_by_one(train_set, set_size, bow, model);

    int check = train_per_spec_spars_list_threads(hashT, train_set, set_size, bow, model, Scheduler);
        
    // If a termination signal was recieved
    if(received_signal == 1 || check == -1){
        logistic_destroy(model);
        return NULL;
    }

    return model;
}

int train_per_spec_spars_list(mySpec** train_set, int set_size, BoWords* bow, logM* model){
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
    dataI* info_list = dataI_create(2*bow->entries);

    make_it_spars_list(train_set, set_size, bow, info_list, 1);

    if(received_signal == 1){
        //  FREE MEM;
        dataI_destroy(info_list);
        return -1;
    }

    // TRAIN MODEL
    int check = logistic_fit_dataList(model, info_list);

    if(received_signal == 1 || check == -1){
        //  FREE MEM;
        dataI_destroy(info_list);
        return -1;
    }

    // PRINT STATS FOR TESTING
    printf("\ttrained times: %d\n", model->trained_times);

    //  FREE MEM
    dataI_destroy(info_list);

    return 0;
}

void make_it_spars_list(mySpec** set, int set_size, BoWords* bow, dataI* info_list, int use_tag){
    // FIND PAIRS AND CONCAT THEIR SPARS
    int i = 0;
    while(i<set_size){
        int z = i + 1;
        while(z < set_size){
            if(set[i]->mySpars == NULL && set[z]->mySpars == NULL){
                z++;
                continue;
            }
            int tag = isPair(set[i], set[z]);
            if((use_tag == 1 && tag != -1 ) || use_tag == -1){

                float** temp = spars_concat_col(set[i]->mySpars, set[z]->mySpars, set[i]->spars_size, set[z]->spars_size, bow->entries);
                int temp_size = set[z]->spars_size + set[i]->spars_size;
                dataI_push(info_list, set[i], set[z], temp, temp_size, tag);
            }
            z++;
        }
        i++;
    }
}


float make_it_spars_list_threads(mySpec** set, int set_size, logM* model, BoWords* bow, int use_tag, jobSch* Scheduler){

    int info_size = 1;
    dataI** info_array = malloc(sizeof(dataI*)*1);
    info_array[0] = dataI_create(2*bow->entries);

    // FIND PAIRS AND CONCAT THEIR SPARS
    int i = 0;
    while(i<set_size){
        int z = i + 1;
        while(z < set_size){
            if(set[i]->mySpars == NULL && set[z]->mySpars == NULL){
                z++;
                continue;
            }
            int tag = isPair(set[i], set[z]);
            if((use_tag == 1 && tag != -1 ) || use_tag == -1){

                float** temp = spars_concat_col(set[i]->mySpars, set[z]->mySpars, set[i]->spars_size, set[z]->spars_size, bow->entries);
                int temp_size = set[z]->spars_size + set[i]->spars_size;
                dataI_push(info_array[info_size-1], set[i], set[z], temp, temp_size, tag);
                if(info_array[info_size-1]->all_pairs == MAX_TRAIN_SIZE_PER_THREAD_TEST){
                    info_array = realloc(info_array, sizeof(dataI*)*(info_size+1));
                    info_array[info_size] = dataI_create(2*bow->entries);

                    t_Info_test* thread_info = make_info_test(model, info_array[info_size-1]);
                    jobSch_subbmit(Scheduler, &logistic_predict_proba_dataList, thread_info, "test");
                    jobSch_Start(Scheduler);
                    
                    info_size++;
                }
            }
            z++;
        }
        i++;
    }

    if(info_array[info_size-1]->all_pairs != 0){
        t_Info_test* thread_info = make_info_test(model, info_array[info_size-1]);
        jobSch_subbmit(Scheduler, &logistic_fit_dataList, thread_info, "test");
        jobSch_Start(Scheduler);
    }

    jobSch_waitAll(Scheduler);

        // calc acc
    float acc = 0.0;
    int cell = 0;
    while(cell < info_size){
       acc += logistic_score_dataList(model, info_array[cell++]);
    }

    acc /= info_size;

        // free mem
    while(info_size > 0){
        dataI_destroy(info_array[--info_size]);
    }
    free(info_array);

    return acc;
}

float make_tests_spars_list(BoWords* bow, logM* model, mySpec** test_set, int set_size){
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
    dataI* info_list = dataI_create(2*bow->entries);

    make_it_spars_list(test_set, set_size, bow, info_list, 1);

    if(received_signal == 1){
        //  FREE MEM;
        dataI_destroy(info_list);
        return 0;
    }

    logistic_predict_proba_dataList(model, info_list);

    // PRINT STATS FOR TESTING
    // printf("\tAccuracy at test_set: %.4f\n", logistic_score_dataList(model, info_list));

    float acc = logistic_score_dataList(model, info_list);

    //  FREE MEM
    dataI_destroy(info_list);

    return acc;
}

float make_tests_spars_list_threads(BoWords* bow, logM* model, mySpec** test_set, int set_size, jobSch* Scheduler){
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

    float acc = make_it_spars_list_threads(test_set, set_size, model, bow, 1, Scheduler);

    if(received_signal == 1){
        //  FREE MEM;
        return 0;
    }

    // logistic_predict_proba_dataList(model, info_list);
    // float acc = logistic_score_dataList(model, info_list);

    return acc;
}

int train_per_spec_spars_list_one_by_one(mySpec** train_set, int set_size, BoWords* bow, logM* model){
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
    dataI* info_list = dataI_create(2*bow->entries);

    make_it_spars_list_plus_train(model, train_set, set_size, bow, info_list, 1);

    if(received_signal == 1){
        //  FREE MEM;
        dataI_destroy(info_list);
        return -1;
    }

    // PRINT STATS FOR TESTING
    // printf("\ttrained times: %d\n", model->trained_times);

    //  FREE MEM
    dataI_destroy(info_list);

    return 0;
}

int train_per_spec_spars_list_threads(hashTable* hashT, mySpec** train_set, int set_size, BoWords* bow, logM* model, jobSch* Scheduler){
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

    make_it_spars_list_threads_plus_train(hashT, model, train_set, set_size, bow, 1, Scheduler);
    if(received_signal == 1)
        return -1;
    return 0;
}
    

void make_it_spars_list_plus_train(logM* model, mySpec** set, int set_size, BoWords* bow, dataI* info_list, int use_tag){

    // FIND PAIRS AND CONCAT THEIR SPARS
    int  i = 0;
    while(i<set_size){
        if(received_signal == 1)
            break;
        int z = i + 1;
        while(z < set_size){
            if(received_signal == 1)
                break;
            if(set[i]->mySpars == NULL && set[z]->mySpars == NULL){
                z++;
                continue;
            }
            int tag = isPair(set[i], set[z]);
            if((use_tag == 1 && tag != -1 ) || use_tag == -1){

                float** temp = spars_concat_col(set[i]->mySpars, set[z]->mySpars, set[i]->spars_size, set[z]->spars_size, bow->entries);
                int temp_size = set[z]->spars_size + set[i]->spars_size;
                dataI_push(info_list, set[i], set[z], temp, temp_size, tag);

                // train one by one
                if(model->trained_times == 0){
                    logistic_fit_dataList(model, info_list);
                }
                else{
                    logistic_refit_dataList(model, info_list);
                }

                // empty list
                dataN_destroy(info_list, info_list->head);
            }
            z++;
        }
        i++;
    }

}

void make_it_spars_list_threads_plus_train(hashTable* hashT, logM* model, mySpec** set, int set_size, BoWords* bow, int use_tag, jobSch* Scheduler){

    threads_list* list = t_list_create(MAX_TRAIN_SIZE_PER_THREAD, Scheduler, model);
    
    // FIND PAIRS AND CONCAT THEIR SPARS
    int i = 0;
    while(i<set_size){
        if(received_signal == 1)
            break;
        int z = i + 1;
        while(z < set_size){
            if(received_signal == 1)
                break;
            if(set[i]->mySpars == NULL && set[z]->mySpars == NULL){
                z++;
                continue;
            }
            int tag = isPair(set[i], set[z]);
            if((use_tag == 1 && tag != -1 ) || use_tag == -1){

                float** temp = spars_concat_col(set[i]->mySpars, set[z]->mySpars, set[i]->spars_size, set[z]->spars_size, bow->entries);
                int temp_size = set[z]->spars_size + set[i]->spars_size;
                t_list_push(list, set[i], set[z], temp, temp_size, tag, 2*bow->entries);
                
            }
            z++;
        }
        i++;
    }

        // SUBBMIT REMAINING JOBS
    if(received_signal != 1 && list->tail->node->all_pairs > 0){
        t_Info_train* thread_info = make_info_train(model, list->tail->node);
            /// sumbbit job
        jobSch_subbmit(Scheduler, &logistic_fit_dataList, thread_info, "train");
        jobSch_Start(Scheduler);
    }

    jobSch_waitAll(Scheduler);

    printf("egine to prwto train\n");

    list->point = list->entries;

    //     // RE-TRAIN
    int count_retrain = 0;
    while(count_retrain < 1){
        retrain_with_all(hashT, list, model, Scheduler);
        list->point = list->entries;   
        printf("teleiwse to ola me ola\n");
        t_list_subbmit_all(list);
        count_retrain++;
    }


    t_list_destroy(list);

}


void retrain_with_all(hashTable* hashT, threads_list* list, logM* model, jobSch* Scheduler){

    int i = 0;
    while(i < hashT->tableSize){

        bucket* tempB = hashT->myTable[i];
        while(tempB != NULL){
            record* tempR = tempB->rec;
            while(tempR != NULL){
                if(tempR->spec->matches->specsCount <= 1 && tempR->spec->matches->negs->entries == 0){
                    tempR = tempR->next;
                    continue;
                }

                t_Info_retrain* thread_info = make_info_retrain(model, tempR, tempB, list);
                jobSch_subbmit(Scheduler, &get_all_bucket_pairs, thread_info, "retrain");
                jobSch_Start(Scheduler);

                bucket* tempB_2 = tempB;
                bucket* keep = tempB_2;
                int count = 0;
                while(tempB_2 != NULL){
                    if(count == 4){
                        t_Info_retrain* thread_info = make_info_retrain(model, tempR, tempB_2, list);
                        jobSch_subbmit(Scheduler, &get_all_bucket_pairs, thread_info, "retrain");
                        jobSch_Start(Scheduler);
                        count = 0;
                        keep = tempB_2;
                    }
                    tempB_2 = tempB_2->next;
                    count++;
                }
                
                if(count != 4){
                    t_Info_retrain* thread_info = make_info_retrain(model, tempR, keep, list);
                    jobSch_subbmit(Scheduler, &get_all_bucket_pairs, thread_info, "retrain");
                    jobSch_Start(Scheduler);
                }

                int i_2 = i+1;
                while(i_2 < hashT->tableSize){
                    tempB_2 = hashT->myTable[i_2];// printf("vazw jobs\n");
                    count = 0;
                    keep = tempB_2;
                    while(tempB_2 != NULL){
                        if(count == 4){
                            t_Info_retrain* thread_info = make_info_retrain(model, tempR, tempB_2, list);
                            jobSch_subbmit(Scheduler, &get_all_bucket_pairs, thread_info, "retrain");
                            jobSch_Start(Scheduler);
                            count = 0;
                            keep = tempB_2;
                        }
                        tempB_2 = tempB_2->next;
                        count++;
                    }
                    if(count != 4){
                        t_Info_retrain* thread_info = make_info_retrain(model, tempR, keep, list);
                        jobSch_subbmit(Scheduler, &get_all_bucket_pairs, thread_info, "retrain");
                        jobSch_Start(Scheduler);
                    }

                    i_2++;
                }
                tempR = tempR->next;
            }
            tempB = tempB->next;
        }
        i++;
    }
    printf("kanw wait ta threads\n");
    jobSch_waitAll(Scheduler);
    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    printf("ME TROLLAREI O KWDIKAS MOY\n");
    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
}


void get_all_bucket_pairs(logM* model, record* myrec, bucket* mybuc, threads_list* list){

    record* tempRec = NULL;
    bucket* tempBuck = NULL;
    if(myrec->next != NULL){
        tempRec = myrec->next;
        tempBuck = mybuc;
    }
    else if(mybuc != NULL){
        if(mybuc->next != NULL){
            tempBuck = mybuc->next;
            tempRec = tempBuck->rec;
        }
    }

    if(tempRec == NULL)
        return;

    int count = 0;

    int dimensions = list->head->node->dimensions;
    while(tempRec != NULL){
        if(tempRec->spec->matches->specsCount > 1 || tempRec->spec->matches->negs->entries != 0){

            dataI* check_pr = dataI_create(dimensions);
            float** temp = spars_concat_col(myrec->spec->mySpars, tempRec->spec->mySpars, myrec->spec->spars_size, tempRec->spec->spars_size, dimensions);
            dataI_push(check_pr, myrec->spec, tempRec->spec, temp, myrec->spec->spars_size + tempRec->spec->spars_size, -1);

            if(temp != NULL){

                logistic_predict_proba_dataList(model, check_pr);

                // printf("proba: %f\n",check_pr->head->proba );            
                int tag = -1;
                if(check_pr->head->proba >= 0.85 && check_pr->head->proba < 1.0){
                    tag = 1;
                }
                else if( check_pr->head->proba <= 0.15 && check_pr->head->proba > 0.0){
                    tag = 0;
                }
                if(tag != -1){
                    float** temp2 = spars_concat_col(myrec->spec->mySpars, tempRec->spec->mySpars, myrec->spec->spars_size, tempRec->spec->spars_size, dimensions);
                    check_info_list(list, myrec->spec, tempRec->spec, temp2);
                }
            }
            dataI_destroy(check_pr);
        }

        if(count == 4)
            break;

            // get next rec
        if(tempRec->next != NULL){
            tempRec = tempRec->next;
        }
        else if(tempBuck!= NULL){
            if(tempBuck->next != NULL){
                tempBuck = tempBuck->next;
                tempRec = tempBuck->rec;
                count++;
            }
            else
                tempRec = NULL;
        }
        else
            tempRec = NULL;

    }
}

void check_info_list(threads_list* list, mySpec* spec1, mySpec* spec2, float** spars){

    int i = 0;
    int found = 0;
    // printf("checking ar\n");
    // printf("size: %d\n", myar->info_size);
    threads_node* temp = list->head;
    while(i < list->point){
        // printf("%d / %d\n", i, myar->info_size);
        dataN* node = temp->node->head;
        while(node != NULL){

            if(node->spec1 == spec1 && node->spec2 == spec2){
                found = 1;
                break;
            }
            else if(node->spec1 == spec2 && node->spec2 == spec1){
                found = 1;
                break;
            }
            node = node->next;
        }
        if(found == 1)
            break;

        temp = temp->next;
        i++;
    }

    if(found == 0){
        // printf("kanw push sth lista\n");
        int dimensions = list->head->node->dimensions;
        t_list_push(list, spec1, spec2, spars, spec1->spars_size+spec2->spars_size, -1, dimensions);
    }
}



// ~~~~~~~~~~~~~~~~~~~~~~~~ TRAIN_PER_CLIQUE ~~~~~~~~~~~~~~~~~~~~~

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

    }

    printf("Trained for %d different Matches Groups !!\n", i);

    return modelsT;
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


// ~~~~~~~~~~~~~~~~~~~~~~~ MAKE HASH TO SPARS ~~~~~~~~~~~~~~~~~~~~

void hash_to_spars(hashTable* hashT, BoWords* bow){
    int i = 0;
    int row = 0;
    int col = 0;
    while(i < hashT->tableSize){
        // printf("i: %d\n", i);
        bucket* tempBuc = hashT->myTable[i];
        while(tempBuc != NULL){
            // printf("buc\n");
            record* tempRec = tempBuc->rec;
            while(tempRec != NULL){
                // printf("rec\n");
                bow_to_spars(bow, &tempRec->spec->mySpars, &tempRec->spec->spars_size, &row, &col, tempRec->spec);
                tempRec = tempRec->next;
            }
            tempBuc = tempBuc->next;
        }
        i++;
    }
}

// ~~~~~~~~~~~~~~~~~~~~~~~ ALL WITH ALL FUNCTS ~~~~~~~~~~~~~~~~~~~






// void all_with_all(hashTable* hashT, logM* model, BoWords* bow){
//              // Signal handling
//     struct sigaction    act;
//     sigset_t            block_mask;
//     received_signal = 0;
//     sigemptyset(&(act.sa_mask));
// 	act.sa_flags = 0;
//     act.sa_handler = sig_int_quit_handler;
//     sigemptyset(&block_mask);
// 	sigaddset(&block_mask,SIGINT);
// 	sigaddset(&block_mask,SIGQUIT);

//     int len = strlen(PATH) + 1 + strlen("strong_matches_DEF") + 1;

//     if(chdir(PATH) == -1){
//         if(mkdir(PATH, S_IRWXU|S_IRWXG|S_IROTH)){ 
//             error(EXIT_FAILURE, errno, "Failed to create directory");
//         }
//     }
//     else{
//         chdir("..");
//     }

//     char* target = malloc(len);
//     memset(target, 0 , len);

//     strcat(target, PATH);
//     strcat(target, "/");
//     strcat(target, "strong_matches_DEF");

//         // CREATE FILE WITH NAME: FNAME INT TARGET DIR
//     FILE* fpout = NULL;
//     fpout = fopen(target, "w");
//     fclose(fpout);

//     myThreads* threads = myThreads_Init(hashT->tableSize);

//     int cur_i = 0;
//     while(cur_i < hashT->tableSize){
//         if(received_signal == 1)
//             break;

//             // Fill struct for each thread
//         t_Info* myInfo = malloc(sizeof(t_Info));
//         myInfo->bow = bow;
//         myInfo->cell = cur_i;
//         myInfo->hashT = hashT;
//         myInfo->model = model;
//         myInfo->target = strdup(target);

//             // FOR EVERY CELL OF THE HASH_TABLE CREATE A THREAD
//         pthread_create(&threads->t_Nums[cur_i], NULL, &all_with_all_ThreadsStart, myInfo);
//         threads->active++;

//         cur_i++;
//     }

//     // sleep(5);
//     myThreads_Destroy(threads);
//     pthread_mutex_destroy(&mtx_print);
//     free(target);
// }

// void* all_with_all_ThreadsStart(void* info){
//     // Start of all printing threads
//     t_Info* myInfo = (t_Info*) info;

//     // printf("mpla: %d\n", myInfo->cell);

//     bucket* tempBuc = myInfo->hashT->myTable[myInfo->cell];

//     while(tempBuc != NULL){
//         if(received_signal == 1)
//             break;

//         record* tempRec = tempBuc->rec;

//         while(tempRec != NULL){
//             if(received_signal == 1)
//                 break;

//             // FOR EVERY RECORD INSIDE THIS CELL IN HASH_TABLE FIND ALL MATCHES
//             one_with_all(myInfo->hashT, myInfo->model, myInfo->bow, tempRec, tempBuc, myInfo->cell, myInfo->target);

//             tempRec = tempRec->next;
//         }
//         tempBuc = tempBuc->next;
//     }

//     free(myInfo->target);
//     free(myInfo);
//     pthread_exit(NULL);
// }

// void one_with_all(hashTable* hashT, logM* model, BoWords* bow, record* rec, bucket* buc, int cur_cell, char* target){
//     record* keep_next_rec = rec;
//     bucket* keep_next_buc = buc;
//     int keep_next_i = cur_cell;

//     FILE* fpout = fopen(target, "a");
//     if(fpout == NULL){
//         printf("Error !! Cant Open Output File ~ one_with_all\n");
//         return;
//     }

//     dataI* info_list = dataI_create(2*bow->entries);
//     mySpec** specA = malloc(2*sizeof(mySpec*));

//     // GET ALL NEXT RECORDS
//     specA[0] = rec->spec;
//     keep_next_rec = get_me_next(hashT, &keep_next_i, &keep_next_buc, &keep_next_rec);
//     while(keep_next_rec != NULL){
//         specA[1] = keep_next_rec->spec;
//         if(received_signal == 1)
//             break;

//         make_it_spars_list(specA, 2, bow, info_list, -1);
//         logistic_predict_proba_dataList(model, info_list);

//                 // Check if specs created a match (only reason not is 2 empty spars !!)
//         if(info_list->all_pairs == 0){
//             keep_next_rec = get_me_next(hashT, &keep_next_i, &keep_next_buc, &keep_next_rec);
//             continue;
//         }

//         if(info_list->all_pairs == 0){
//             keep_next_rec = get_me_next(hashT, &keep_next_i, &keep_next_buc, &keep_next_rec);
//             continue;
//         }

//                 // Check if their match is strong and print them at the file
//         if(info_list->head->predict == 0){
//             if(info_list->head->proba - info_list->head->predict <= 0.01){
//                 pthread_mutex_lock(&mtx_print);
//                 fseek(fpout, 0, SEEK_END);
//                 fprintf(fpout, "%s, %s, %d\n", specA[0]->specID, specA[1]->specID, info_list->head->predict);
//                 pthread_mutex_unlock(&mtx_print);
//             }
//         }
//         else{
//             if(info_list->head->predict - info_list->head->proba <= 0.01){
//                 pthread_mutex_lock(&mtx_print);
//                 fseek(fpout, 0, SEEK_END);
//                 fprintf(fpout, "%s, %s, %d\n", specA[0]->specID, specA[1]->specID, info_list->head->predict);
//                 pthread_mutex_unlock(&mtx_print);
           
//             }
//         }

//         keep_next_rec = get_me_next(hashT, &keep_next_i, &keep_next_buc, &keep_next_rec);
//         dataN_destroy(info_list, info_list->head);
//     }

//     fclose(fpout);
//     free(specA);
//     dataI_destroy(info_list);
// }

// record* get_me_next(hashTable* hashT, int* cur_buc, bucket** buc, record** rec){
//         // get next rec if exist
//     if((*rec)->next != NULL){
//         return (*rec)->next;
//     }
//         // change buc if needed
//     else if((*buc)->next != NULL){
//         (*buc) = (*buc)->next;
//         return (*buc)->rec;
//     }
//         // change hash cell if needed
//     else if(*cur_buc < hashT->tableSize-1){
//         (*cur_buc) += 1;
//         while((*cur_buc) < hashT->tableSize){   // skip empty cells
//             (*buc) = hashT->myTable[(*cur_buc)];
//             if((*buc) != NULL){
//                return hashT->myTable[(*cur_buc)]->rec;
//             }
//             (*cur_buc) += 1;
//         }
//     }
//     return NULL;
// }


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~ SIGNALS ~~~~~~~~~~~~~~~~~~~~~~~~~~

void sig_int_quit_handler(int signo)
{
	if(signo == SIGINT || signo == SIGQUIT)
		received_signal = 1;
}
