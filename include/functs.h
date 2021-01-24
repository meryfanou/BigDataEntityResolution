/* functs.h
	Functs for main*/
#ifndef FUNCTS_H
#define FUNCTS_H
#include <dirent.h>
#include "myHash.h"
#include "myMatches.h"
#include "mySpec.h"
#include "boWords.h"
#include "math.h"
#include "logistic.h"
#include "myThreads.h"
#include "jobScheduler.h"

extern int received_signal;

// ~~~~~~~~~~~~~~~~ READ DATA ~~~~~~~~~~~~~~~~

int readDataset(DIR*, char*, hashTable**, matchesInfo*);	// reads from dataset and parse json form properly

specInfo** readFile(FILE*, int*, specInfo**);				// called by readDataset on each inner file

char* shuffleCSV(char*);

int readCSV(char* , hashTable* , matchesInfo*, float, long int*);	//reads part of csv (for the training set), creates matches at hash values

// ~~~~~~~~~~~~~~ SETS OF SPECS ~~~~~~~~~~~~~~

mySpec** get_trainSet(matchesInfo*, int*);							// Create the training set from the given cliques
mySpec** get_testSet(char*, hashTable*, int*, long int*, int, matchesInfo*);		// Create the testing set (distinct) while reading the csv
mySpec** get_validationSet(char*, hashTable*, int*, long int*, int, matchesInfo*);	// Create the validation set (distinct) while reading the csv
mySpec** get_set(char*, hashTable*, int*, long int*, int, char, matchesInfo*);		// Used for creating the testing + validation sets

// ~~~~~~~~~~~~~~ BAG OF WORDS ~~~~~~~~~~~~~~~

void text_to_bow(mySpec**, int, BoWords**);					// Turn a text to bow

void set_mostSignificantWords(BoWords*, int);			// Mark the most significant words in bow
void keep_mostSignificantWords(BoWords*);				// Remove all insignificant words from bow

// ~~~~~~~~~~~~~~ MAKE HASH SPARS ~~~~~~~~~~~~~

void hash_to_spars(hashTable*, BoWords*);

// ~~~~~~~~~~~~~~~~ TRAINING ~~~~~~~~~~~~~~~~
	// !! vec
logM* make_model_vec(BoWords*, mySpec**, int);
float* vectorization(mySpec*, BoWords*, int*);
float* concat_specVectors(float*, float*, int);
float** concat_pairsVectors(float**, float*, int);
int* concat_tags(int*, int, int);
int train_per_spec_vec(mySpec**, int, BoWords*, logM*);
void make_vectors(mySpec**, int, BoWords*, float***, float***, int**, int*, int*);

	//  !! spars
float** make_it_spars(mySpec**, int, BoWords*, int*, int**, int*);
void print_spars(float**, int);
int train_per_spec_spars(mySpec**, int, BoWords*, logM*);
float** spars_concat_col(float**, float**, int, int, int);
void spars_concat_row(float***, float**, int*, int, int);
logM* make_model_spars(BoWords*, mySpec**, int);

	// !! spars list
logM* make_model_spars_list(hashTable* hashT, BoWords*, mySpec**, int, jobSch*);
int train_per_spec_spars_list(mySpec**, int, BoWords*, logM*);
void make_it_spars_list(mySpec**, int, BoWords*, dataI*, int);


int train_per_spec_spars_list_one_by_one(mySpec**, int, BoWords*, logM*);
int train_per_spec_spars_list_threads(hashTable*, mySpec**, int, BoWords*, logM*, jobSch*);

void make_it_spars_list_plus_train(logM*, mySpec**, int, BoWords*, dataI*, int);
void make_it_spars_list_threads_plus_train(hashTable*, logM*, mySpec**, int, BoWords*, int, jobSch*);
float make_it_spars_list_threads(mySpec**, int, logM*, BoWords*, int, jobSch*);

void retrain_with_all(hashTable*, info_ar*, logM*, jobSch*);
void get_all_bucket_pairs(logM*, record*, bucket*, info_ar*);
void check_info_array(info_ar*, mySpec*, mySpec*, float**);


int isPair(mySpec*, mySpec*);


/// ~~~~~~~~~~~~~~~~ TESTING ~~~~~~~~~~~~~~~~

float make_tests(BoWords*, logM*, mySpec**, int);
float make_tests_spars(BoWords*, logM*, mySpec**, int);
float make_tests_spars_list(BoWords*, logM*, mySpec**, int);
float make_tests_spars_list_threads(BoWords*, logM*, mySpec**, int, jobSch*);

/* NOT USED */
void train_per_clique(myMatches*, mySpec**, int, BoWords*, logM*);
logM** make_models_array(BoWords*, mySpec**, matchesInfo*, int);

void all_with_all(hashTable*, logM*, BoWords*);
void one_with_all(hashTable*, logM*, BoWords*, record*, bucket*, int, char*);
record* get_me_next(hashTable*, int*, bucket**, record**);
void create_threads(myThreads*);
void* all_with_all_ThreadsStart(void*);


// ~~~~~~~~~~~~~~~~ SIGNALS ~~~~~~~~~~~~~~~~

void sig_int_quit_handler(int);


// ~~~~~~~~~~~~~~~~ FREE MEM ~~~~~~~~~~~~~~~~

#define FREE_MEM(pathX,pathW,outputFileMatches,outputFileNegs,allMatches,hashT,trainSet,testSet,validSet,bow,model,scheduler)   \
{																								\
	if(pathX != NULL)																			\
		free(pathX);																			\
	if(pathW != NULL)																			\
		free(pathW);																			\
																								\
	if(outputFileMatches != NULL)																\
        free(outputFileMatches);																\
	if(outputFileNegs != NULL)																	\
		free(outputFileNegs);																	\
																								\
	if(allMatches != NULL)																		\
		deleteInfo((matchesInfo*)allMatches);													\
	if(hashT != NULL)																			\
		hash_destroy((hashTable*)hashT);														\
																								\
	if(trainSet != NULL)																		\
		free(trainSet);																			\
																								\
	if(testSet != NULL)																			\
		free(testSet);																			\
																								\
	if(validSet != NULL)																		\
		free(validSet);																			\
																								\
	if(bow != NULL)																				\
		bow_destroy((BoWords*)bow);																\
																								\
	if(model != NULL)																			\
		logistic_destroy((logM*)model);															\
																								\
	if(scheduler != NULL)																			\
		jobSch_Destroy((jobSch*)scheduler);															\
}


#endif