#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <signal.h>
#include "../include/mySpec.h"
#include "../include/myMatches.h"
#include "../include/myHash.h"
#include "../include/myMatches.h"
#include "../include/functs.h"
#include "../include/boWords.h"
#include "../include/logistic.h"

#define DATASET_X "../camera_specs/2013_camera_specs/"
#define DATASET_W "../sigmod_large_labelled_dataset.csv"

#define HASH_SIZE 500
#define BUC_SIZE 100

#define TRAIN_PERC 0.6
#define TEST_PERC 0.2

#define MOST_SIGN 1000

int main(int argc, char** argv){

    // ./main (-o (file_name)) (-l medium or -l <path_to_W>) (-p <path_to_X>)

    char*   path_X = strdup(DATASET_X);
    char*   path_W = strdup(DATASET_W);

    // By default, use model with spars implementation
    char    choose_model = 's';

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ READ ARGUMENTS
    char* outputFileMatches = NULL;
    char* outputFileNegs = NULL;
    if(argc != 0){
        int i = 0;
        while(i < argc){
            if(strcmp(argv[i], "-o") == 0){
                if(argv[i+1] != NULL)
                    outputFileMatches = strdup(argv[i+1]);
            }
            else if(strcmp(argv[i], "-n") == 0){
                if(argv[i+1] != NULL)
                    outputFileNegs = strdup(argv[i+1]);
            }
            else if(strcmp(argv[i], "-labels") == 0 || strcmp(argv[i], "-l") == 0){
                if(strcmp(argv[i+1], "medium") == 0 || strcmp(argv[i+1], "m") == 0){
                    char* temp = "../sigmod_medium_labelled_dataset.csv";
                    free(path_W);
                    path_W = strdup(temp);
                }
                else{
                    free(path_W);
                    path_W = strdup(argv[i+1]);
                }
            }
            else if(!strcmp(argv[i], "-path") || !strcmp(argv[i], "-p")){
                free(path_X);
                path_X = strdup(argv[i+1]);
            }
            else if(!strcmp(argv[i], "-model") || !strcmp(argv[i], "-m")){
                // 's' for spars, 'v' for vector
                choose_model = argv[i+1][0];
            }
            
            i++;
        }
    }

    printf("path: %s\n", path_X);

    DIR                 *datasetX = NULL;
    struct sigaction    act;
    sigset_t            block_mask;


    sigemptyset(&(act.sa_mask));
	act.sa_flags = 0;
    act.sa_handler = sig_int_quit_handler;
	if(sigaction(SIGINT,&act,NULL) < 0 || sigaction(SIGQUIT,&act,NULL) < 0)
	{
		perror("sigaction");
        free(path_X);
        free(path_W);
		exit(-1);
	}
    sigemptyset(&block_mask);
	sigaddset(&block_mask,SIGINT);
	sigaddset(&block_mask,SIGQUIT);

    printf("\nBuilding Hash ...\n");
    hashTable* hashT = hash_create(HASH_SIZE, BUC_SIZE);
    matchesInfo* allMatches = matchesInfoInit();

    // Check for termination signal
    if(received_signal == 1){
        printf("\nCleaning Memory ...\n");

        int* null = NULL;
        FREE_MEM(path_X,path_W,outputFileMatches,outputFileNegs,allMatches,hashT,null,null,null,null,null);

        printf("Exiting after receiving termination signal..\n");
        exit(-2);
    }

    // Open datasetX
    if((datasetX = opendir(path_X)) == NULL){
        perror("opendir");
        printf("\nCleaning Memory ...\n");

        int* null = NULL;
        FREE_MEM(path_X,path_W,outputFileMatches,outputFileNegs,allMatches,hashT,null,null,null,null,null);
        exit(-3);
    }

    // Read specs from dataset X and store them using hashT
    // If a termination signal was received, return 1. If an error occured, return negative value. Otherwise return 0
    int check = readDataset(datasetX, path_X, &hashT, allMatches);

    if(received_signal == 1 || check != 0){
        printf("\nCleaning Memory ...\n");

        int* null = NULL;
        FREE_MEM(path_X,path_W,outputFileMatches,outputFileNegs,allMatches,hashT,null,null,null,null,null);

        closedir(datasetX);

        if(check == 1)
            printf("Exiting after receiving termination signal..\n");

        exit(-4);
    }

    closedir(datasetX);

    printf("       \t\t.. DONE !!\n");

    char* shuffled = shuffleCSV(path_W);
    if(shuffled == NULL){
        printf("\nCleaning Memory ...\n");

        free(shuffled);
        int* null = NULL;
        FREE_MEM(path_X,path_W,outputFileMatches,outputFileNegs,allMatches,hashT,null,null,null,null,null);

        exit(-5);
    }

    printf("Reading CSV ...\n");
    // If a termination signal was received, return 1. If an error occured, return negative value. Otherwise return number of lines read
    long int offset = 0;
    check = readCSV(shuffled, hashT, allMatches, TRAIN_PERC, &offset);

    if(remove(shuffled) == -1){
        perror("remove");
        printf("\nCleaning Memory ...\n");

        free(shuffled);
        int* null = NULL;
        FREE_MEM(path_X,path_W,outputFileMatches,outputFileNegs,allMatches,hashT,null,null,null,null,null);

        exit(-5);
    }

    free(shuffled);

    if(received_signal == 1 || check < 0){
        printf("\nCleaning Memory ...\n");

        int* null = NULL;
        FREE_MEM(path_X,path_W,outputFileMatches,outputFileNegs,allMatches,hashT,null,null,null,null,null);

        if(check == 1)
            printf("Exiting after receiving termination signal..\n");

        exit(-5);
    }

    int train_lines = check;
    printf("       \t\t.. DONE !!\n");

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    //~~~~~~~~~~~~~~~~ SEPERATE SPECS TO TRAINING, TESTING AND VALIDATION SETS
    mySpec** trainSet = NULL;
	mySpec** testSet = NULL;
	mySpec** validSet = NULL;

    int trainSize, testSize, validSize;

    int test_lines = TEST_PERC*(train_lines / TRAIN_PERC);
    int valid_lines = test_lines;

    printf("\nCreating Training, Testing and Validation Sets..\n");
    trainSet = get_trainSet(allMatches, &trainSize);
    // Get test set and update allMatches with the its specs
    testSet = get_testSet(path_W, hashT, &testSize, &offset, test_lines, allMatches);
    // Get validation set and update allMatches with the its specs
    validSet = get_validationSet(path_W, hashT, &validSize, &offset, valid_lines, allMatches);

    // Check for termination signal or other errors
    if(received_signal == 1 || trainSet == NULL || testSet == NULL || validSet == NULL){
        printf("\nCleaning Memory ...\n");

        int* null = NULL;
        FREE_MEM(path_X,path_W,outputFileMatches,outputFileNegs,allMatches,hashT,trainSet,testSet,validSet,null,null);

        if(received_signal == 1)
            printf("Exiting after receiving termination signal..\n");
        exit(-2);
    }

    // printf("%d\n",trainSize);
    // printf("%d\n",testSize);
    // printf("%d\n",validSize);

    printf("       \t\t.. DONE !!\n\n");

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    //~~~~~~~~~~~~~~~~~~~~~~ EXTARCT PAIRS
    printf("Extracting positive & negative matches..\n");
    extractMatches(allMatches, outputFileMatches);
    extractNegatives(allMatches, outputFileNegs);
    printf("       \t\t.. DONE !!\n\n");

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    //~~~~~~~~~~~~~~~~~~~~~~ PREPARE THE INPUT FOR THE MODEL
    printf("\nBuilding BoW..\n");
    BoWords*    bow = bow_create(HASH_SIZE, BUC_SIZE);

    hash_to_bow(hashT, bow);

    // text_to_bow(trainSet, trainSize, &bow);
    // text_to_bow(testSet, testSize, &bow);
    // text_to_bow(validSet, validSize, &bow);

    // Check for termination signal
    if(received_signal == 1){
        printf("\nCleaning Memory ...\n");

        int* null = NULL;
        FREE_MEM(path_X,path_W,outputFileMatches,outputFileNegs,allMatches,hashT,trainSet,testSet,validSet,bow,null);

        printf("Exiting after receiving termination signal..\n");
        exit(-2);
    }

    printf("       \t\t.. DONE !!\n\n");

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    //~~~~~~~~~~~~~~~~~~~~~~ TF-IDF
    printf("\nApplying TF-IDF..\n");
    tfidf* mytf = tfidf_create();
    tfidf_set(mytf, -1, -1);    // (model, maxTexts, maxWords to scan)
    tfidf_apply(mytf, bow);
    tfidf_destroy(mytf);

    // Check for termination signal
    if(received_signal == 1){
        printf("\nCleaning Memory ...\n");

        int* null = NULL;
        FREE_MEM(path_X,path_W,outputFileMatches,outputFileNegs,allMatches,hashT,trainSet,testSet,validSet,bow,null);

        printf("Exiting after receiving termination signal..\n");
        exit(-2);
    }

    printf("       \t\t.. DONE !!\n\n");

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    //~~~~~~~~~~~~~~~~~~~~~~ Mark the most significant words and remove the rest from bow
    printf("\nChoosing %d most significant words..\n",MOST_SIGN);
    set_mostSignificantWords(bow, MOST_SIGN);
    keep_mostSignificantWords(bow);

    // Check for termination signal
    if(received_signal == 1){
        printf("\nCleaning Memory ...\n");

        int* null = NULL;
        FREE_MEM(path_X,path_W,outputFileMatches,outputFileNegs,allMatches,hashT,trainSet,testSet,validSet,bow,null);

        printf("Exiting after receiving termination signal..\n");
        exit(-2);
    }

    printf("       \t\t.. DONE !!\n\n");

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    //~~~~~~~~~~~~~~~~~~~~~~ CREATE & TRAIN LOGISTIC MODEL >
    printf("\nTraining Logistic Model ..\n");
    // logM** modelsT = make_models_array(bow, *trainSet, allMatches, trainSize);
    logM* model = NULL;
    if(choose_model == 'v')
        model = make_model_vec(bow, trainSet, trainSize);
    else if(choose_model == 's')
        model = make_model_spars_list(bow, trainSet, trainSize);
        // model = make_model_spars(bow, trainSet, trainSize);

    // Check for termination signal
    if(received_signal == 1 || model == NULL){
        printf("\nCleaning Memory ...\n");

        FREE_MEM(path_X,path_W,outputFileMatches,outputFileNegs,allMatches,hashT,trainSet,testSet,validSet,bow,model);

        printf("Exiting after receiving termination signal..\n");
        exit(-2);
    }

    printf("       \t\t.. DONE !!\n\n");

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    //~~~~~~~~~~~~~~~~~~~~~ USE TEST_SET FOR PREDICTIONS
    printf("\nTesting Logistic Model ..\n");

    if(choose_model == 'v')
        make_tests(bow, model, testSet, testSize);
    else if(choose_model == 's')
        make_tests_spars_list(bow, model, testSet, testSize);
        // make_tests_spars(bow, model, testSet, testSize);

    // Check for termination signal
    if(received_signal == 1){
        printf("\nCleaning Memory ...\n");

        FREE_MEM(path_X,path_W,outputFileMatches,outputFileNegs,allMatches,hashT,trainSet,testSet,validSet,bow,model);

        printf("Exiting after receiving termination signal..\n");
        exit(-2);
    }

    printf("       \t\t.. DONE !!\n\n");

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    printf("Extract ..\n");
    logistic_extract(model);
    printf("       \t\t.. DONE !!\n\n");

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    // ΔΟΚΙΜΑΣΕ ΜΕ ΑΥΤΑ ΑΝ ΚΑΝΕΙΣ ΣΟΧΛΙΑ ΤΑ ΑΠΟ ΠΑΝΩ !!!!!!!!!
    // logM* model = logistic_create();
    // weights_set(model->finalWeights, 2*bow->entries);
    // model->weights_count = 2*bow->entries;
    all_with_all_gamwtokeratomoumesa(hashT, model, bow);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


printf("All Specs: %d\n", hashT->entries);
myMatches* gamw = allMatches->head;
int mpla = 0;
while(gamw!=NULL){
    mpla += gamw->specsCount;
    gamw = gamw->next;
}
printf("Specs in Matches: %d\n", mpla);
printf("TrainSize: %d\n", trainSize);
printf("All matches: %d\n", allMatches->entries);

    //~~~~~~~~~~~~~~~~~~~~~~ FREE MEM
    printf("\nCleaning Memory...\n");

    FREE_MEM(path_X,path_W,outputFileMatches,outputFileNegs,allMatches,hashT,trainSet,testSet,validSet,bow,model);

    printf("       \t\t.. DONE !!\n\n");

    return 0;
}