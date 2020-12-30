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
#include "../include/pretty_prints.h"

#define DATASET_X "../camera_specs/2013_camera_specs/"
#define DATASET_W "../sigmod_large_labelled_dataset.csv"

#define HASH_SIZE 500
#define BUC_SIZE 100

#define TRAIN_PERC 0.6
#define TEST_PERC 0.2

#define MOST_SIGN 1000

 /*     
  !!  RUN COMMAND:
  !! ./main (-o (file_name)) (-n (file_name)) (-l medium or -l <path_to_W>) (-p <path_to_X>) (-m model)
*/


int main(int argc, char** argv){


    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ SET DEFAULT PATHS, MODEL METHODS, FILE NAMES

    char*   path_X = strdup(DATASET_X);
    char*   path_W = strdup(DATASET_W);

    char    choose_model = 's';    // By default, use model with spars implementation
    
    char* outputFileMatches = NULL;
    char* outputFileNegs = NULL;


    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ READ ARGUMENTS

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


    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ SET SIGNALS

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


    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ SET PRETTY_PRINTS

    ppa* pp  = ppa_create("LARGE");
    ppa_print_start(pp, "PROGRAM STATUS");

    /*
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        PART 1:
                > CREATE HASH
                > READ CSV
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    */


    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ CREATE HASH - INIT MATCHES_LIST

    ppa_add_line_left(pp, "Building Hash ..");

    hashTable* hashT = hash_create(HASH_SIZE, BUC_SIZE);
    matchesInfo* allMatches = matchesInfoInit();

                // Open datasetX
    DIR                 *datasetX = NULL;
    if((datasetX = opendir(path_X)) == NULL){
        perror("opendir");
        ppa_add_line_right(pp, "ERROR", RED);
        ppa_add_line_left(pp, "Cleaning Memory ..");

        int* null = NULL;
        FREE_MEM(path_X,path_W,outputFileMatches,outputFileNegs,allMatches,hashT,null,null,null,null,null);
        ppa_add_line_right(pp, "DONE", GRN);

        exit(-3);
    }

                // Read specs from dataset X and store them using hashT
                // If a termination signal was received, return 1. If an error occured, return negative value. Otherwise return 0
    int check = readDataset(datasetX, path_X, &hashT, allMatches);
    closedir(datasetX);

    ppa_add_line_right(pp, "DONE", GRN);


    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Check for signals

    if(received_signal == 1 || check != 0){
        printf("\b\b");
        ppa_add_line_left(pp, "Cleaning Mem..");
        
        int* null = NULL;
        FREE_MEM(path_X,path_W,outputFileMatches,outputFileNegs,allMatches,hashT,null,null,null,null,null);

        ppa_add_line_right(pp, "DONE", GRN);
        
        if(check == 1)
            ppa_print_end(pp, "Exiting after receiving signal ..");
        
        ppa_destroy(pp);
        exit(-4);
    }


    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ SHUFFLE CSV

    char* shuffled = shuffleCSV(path_W);
    if(shuffled == NULL){
        ppa_add_line_left(pp, "Cleaning Memory");
        ppa_add_line_right(pp, "DONE", GRN);

        free(shuffled);
        int* null = NULL;
        FREE_MEM(path_X,path_W,outputFileMatches,outputFileNegs,allMatches,hashT,null,null,null,null,null);
        
        ppa_print_end(pp, "Exiting at shuffle");
        ppa_destroy(pp);

        exit(-5);
    }


    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ READ CSV

    ppa_add_line_left(pp, "Reading CSV ..");

                // If a termination signal was received, return 1. If an error occured, return negative value. Otherwise return number of lines read
    long int offset = 0;
    check = readCSV(shuffled, hashT, allMatches, TRAIN_PERC, &offset);

    if(remove(shuffled) == -1){
        perror("remove");
        ppa_add_line_right(pp, "Error", RED);

        ppa_add_line_left(pp, "Cleaning Memory ..");

        free(shuffled);
        int* null = NULL;
        FREE_MEM(path_X,path_W,outputFileMatches,outputFileNegs,allMatches,hashT,null,null,null,null,null);
        ppa_add_line_right(pp, "DONE", GRN);

        ppa_print_end(pp, "Exiting ~ cant read csv");
        ppa_destroy(pp);

        exit(-5);
    }

    free(shuffled);
    int train_lines = check;
    ppa_add_line_right(pp, "DONE", GRN);


    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Check for signals

    if(received_signal == 1 || check < 0){
        printf("\b\b");
        ppa_add_line_left(pp, "Cleaning Mem..");


        int* null = NULL;
        FREE_MEM(path_X,path_W,outputFileMatches,outputFileNegs,allMatches,hashT,null,null,null,null,null);

        ppa_add_line_right(pp, "DONE", GRN);

        if(check == 1)
            ppa_print_end(pp, "Exiting after receiving signal ..");

        ppa_destroy(pp);
        exit(-5);
    }



    /*
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    PART 2:
            > CREATE TRAINING, TESTING & VAL SETS
            > CREATE NEGATIVES AND POSITVES MACTHES
            > BUILD BOW, APPLY TF-IDF, KEEP N MOOST SIGNIFFICANT WORDS
            > CREATE AND TRAIN MODEL
            > ALL_WITH_ALL METHOD
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    */


    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ SEPERATE SPECS TO TRAINING, TESTING AND VALIDATION SETS

    mySpec** trainSet = NULL;
	mySpec** testSet = NULL;
	mySpec** validSet = NULL;

    int trainSize, testSize, validSize;

    int test_lines = TEST_PERC*(train_lines / TRAIN_PERC);
    int valid_lines = test_lines;

    ppa_add_line_left(pp, "Creating Sets for train/test/val ..");


    trainSet = get_trainSet(allMatches, &trainSize);    

            // Get test set and update allMatches with the its specs
    testSet = get_testSet(path_W, hashT, &testSize, &offset, test_lines, allMatches);
    
            // Get validation set and update allMatches with the its specs
    validSet = get_validationSet(path_W, hashT, &validSize, &offset, valid_lines, allMatches);

    ppa_add_line_right(pp, "DONE", GRN);


    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Check for signals or other errors

    if(received_signal == 1 || trainSet == NULL || testSet == NULL || validSet == NULL){
        printf("\b\b");
        ppa_add_line_left(pp, "Cleaning Memory ..");

        int* null = NULL;
        FREE_MEM(path_X,path_W,outputFileMatches,outputFileNegs,allMatches,hashT,trainSet,testSet,validSet,null,null);

        ppa_add_line_right(pp, "DONE", GRN);
        
        if(received_signal == 1)
            ppa_print_end(pp, "Exiting after receiving signal ..");

        ppa_destroy(pp);
        exit(-2);
    }


    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ EXTARCT PAIRS

    ppa_add_line_left(pp, "Extracing Pos / Neg matches ..");
    extractMatches(allMatches, outputFileMatches);
    extractNegatives(allMatches, outputFileNegs);
    ppa_add_line_right(pp, "DONE", GRN);


    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ BUILD BOW

    ppa_add_line_left(pp, "Building BoW ..");
    BoWords*    bow = bow_create(HASH_SIZE, BUC_SIZE);

    hash_to_bow(hashT, bow);


    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Check for signals

    if(received_signal == 1){
        printf("\b\b");
        ppa_add_line_right(pp, "Error", RED);
        ppa_add_line_left(pp, "Cleaning Memory");

        int* null = NULL;
        FREE_MEM(path_X,path_W,outputFileMatches,outputFileNegs,allMatches,hashT,trainSet,testSet,validSet,bow,null);

        ppa_add_line_right(pp, "DONE", GRN);

        ppa_print_end(pp, "Exiting after receiving signal ..");
        ppa_destroy(pp);

        exit(-2);
    }

    ppa_add_line_right(pp, "DONE", GRN);


    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ APPLY TF-IDF

    ppa_add_line_left(pp, "Applying TF-IDF ..");
    tfidf* mytf = tfidf_create();
    tfidf_set(mytf, -1, -1);    // (model, maxTexts, maxWords to scan)
    tfidf_apply(mytf, bow);
    tfidf_destroy(mytf);


    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Check for signals

    if(received_signal == 1){
        printf("\b\b");
        ppa_add_line_right(pp, "Error", RED);
        ppa_add_line_left(pp, "Cleaning Memory ..");

        int* null = NULL;
        FREE_MEM(path_X,path_W,outputFileMatches,outputFileNegs,allMatches,hashT,trainSet,testSet,validSet,bow,null);

        ppa_add_line_right(pp, "DONE", GRN);

        ppa_print_end(pp, "Exiting after receiving signal ..");
        ppa_destroy(pp);

        exit(-2);
    }

    ppa_add_line_right(pp, "DONE", GRN);


    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Mark most significant words and remove the rest from bow
    
    ppa_add_line_left(pp, "Choosing most sing words ..");
    set_mostSignificantWords(bow, MOST_SIGN);
    keep_mostSignificantWords(bow);


    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Check for signals

    if(received_signal == 1){
        printf("\b\b");
        ppa_add_line_right(pp, "Error", RED);
        ppa_add_line_left(pp, "Cleaning Memory ..");


        int* null = NULL;
        FREE_MEM(path_X,path_W,outputFileMatches,outputFileNegs,allMatches,hashT,trainSet,testSet,validSet,bow,null);

        ppa_add_line_right(pp, "DONE", GRN);

        ppa_print_end(pp, "Exiting after receiving signal ..");
        ppa_destroy(pp);
        exit(-2);
    }

    ppa_add_line_right(pp, "DONE", GRN);


    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ CREATE & TRAIN LOGISTIC MODEL

    ppa_add_line_left(pp, "Training Logistic Model ..");
    
            // !! Uncomment to choose method of train

    logM* model = NULL;
    if(choose_model == 'v'){
        model = make_model_vec(bow, trainSet, trainSize);
    }
    else if(choose_model == 's'){
        model = make_model_spars_list(bow, trainSet, trainSize);
        // model = make_model_spars(bow, trainSet, trainSize);
    }


    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Check for signals

    if(received_signal == 1 || model == NULL){
        printf("\b\b");
        ppa_add_line_right(pp, "Error", RED);
        ppa_add_line_left(pp, "Cleaning Memory ..");

        FREE_MEM(path_X,path_W,outputFileMatches,outputFileNegs,allMatches,hashT,trainSet,testSet,validSet,bow,model);

        ppa_add_line_right(pp, "DONE", GRN);

        ppa_print_end(pp, "Exiting after receiving signal ..");
        ppa_destroy(pp);

        exit(-2);
    }

    char numbuftr[10];
    memset(numbuftr, 0, 10);

    sprintf(numbuftr, "%d", model->size_totrain);

    int mssg_train_len = strlen("DONE ~ Trained Size: ") + strlen(numbuftr) + 1;
    char* mssg_train = malloc(mssg_train_len);
    memset(mssg_train, 0, mssg_train_len);
    strcat(mssg_train, "DONE ~ Trained Size: ");
    strcat(mssg_train, numbuftr);
    ppa_add_line_right(pp, mssg_train, GRN);
    free(mssg_train);


    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ USE TEST_SET FOR PREDICTIONS

    ppa_add_line_left(pp, "Testing Logistic Model ..");

    float acc = 0.0;
    if(choose_model == 'v'){
        acc = make_tests(bow, model, testSet, testSize);
    }
    else if(choose_model == 's'){
        acc = make_tests_spars_list(bow, model, testSet, testSize);
        //acc = make_tests_spars(bow, model, testSet, testSize);
    }

    char numbuftst[10];
    memset(numbuftst, 0, 10);

    sprintf(numbuftst, "%.4f", acc);

    int mssg_test_len = strlen("DONE ~ Accuracy at Test: ") + strlen(numbuftst) + 1;
    char* mssg_test = malloc(mssg_test_len);
    memset(mssg_test, 0, mssg_test_len);
    strcat(mssg_test, "DONE ~ Accuracy at Test: ");
    strcat(mssg_test, numbuftst);
    ppa_add_line_right(pp, mssg_test, GRN);
    free(mssg_test);


    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Check for signals

    if(received_signal == 1){
        printf("\b\b");
        ppa_add_line_left(pp, "Cleaning Memory ..");


        FREE_MEM(path_X,path_W,outputFileMatches,outputFileNegs,allMatches,hashT,trainSet,testSet,validSet,bow,model);

        ppa_add_line_right(pp, "DONE", GRN);

        ppa_print_end(pp, "Exiting after receiving signal ..");
        ppa_destroy(pp);

        exit(-2);
    }


    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ EXTRACT MODEL_VALUES

    ppa_add_line_left(pp, "Extract Model ..");
    logistic_extract(model);
    ppa_add_line_right(pp, "DONE", GRN);


    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ALL_WITH_ALL_METHOD !!!!!!! VERY SLOW !!!!

    ppa_add_line_left(pp, "All_with_all ..");
    // all_with_all(hashT, model, bow);
    ppa_add_line_right(pp, "DONE", GRN);


    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ PRINT STATS

    // printf("TrainSize: %d\n", trainSize);
    // printf("All matches: %d\n", allMatches->entries);


    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ FREE MEM
    
    ppa_add_line_left(pp, "Cleaning Memory ..");
    FREE_MEM(path_X,path_W,outputFileMatches,outputFileNegs,allMatches,hashT,trainSet,testSet,validSet,bow,model);
    ppa_add_line_right(pp, "DONE", GRN);

    ppa_print_end(pp, "PROJECT_2 <> ALL DONE");
    ppa_destroy(pp);

    return 0;
}