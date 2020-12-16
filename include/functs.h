//typedef struct hashTable hashTable;
//typedef struct specInfo specInfo;

/* functs.h
	Functs for main*/
#ifndef FUNCTS_H
#define FUNCTS_H
#include "myHash.h"
#include "myMatches.h"
#include "mySpec.h"
#include "boWords.h"
#include "math.h"
#include "logistic.h"

extern int received_signal;

// ~~~~~~~~~~~~~~~~ READ DATA ~~~~~~~~~~~~~~~~

int readDataset(DIR*, char*, hashTable**, matchesInfo*);	// reads from dataset and parse json form properly

specInfo** readFile(FILE*, int*, specInfo**);			// called by readDataset on each inner file

int readCSV(char* , hashTable* , matchesInfo*);			//reads csv, creates matches at hash values

// ~~~~~~~~~~~~~~ BAG OF WORDS ~~~~~~~~~~~~~~~

void text_to_bow(mySpec**, int, BoWords**);				// Turn a set of texts to bow
void spec_to_bow(mySpec*, BoWords*);					// Turn a text to bow
void sentence_to_bow(char*, mySpec*, BoWords*);			// Turn a sentence to bow
char* checkWord(char*);									// Check if a word should be added in bow

void set_mostSignificantWords(BoWords*, int);			// Mark the most significant words in bow
void keep_mostSignificantWords(BoWords*);				// Remove all insignificant words from bow

// ~~~~~~~~~~~~~~~~ TRAINING ~~~~~~~~~~~~~~~~

float* vectorization(mySpec*, BoWords*, int*);
float*** spars_array(mySpec*, BoWords*, int*, int*, int*);

void train_per_spec(mySpec**, int, BoWords*, logM*);
float* concat_specVectors(float*, float*, int);
float** concat_pairsVectors(float**, float*, int);
int* concat_tags(int*, int, int);
int isPair(mySpec*, mySpec*);
logM* make_model(BoWords*, mySpec**, int);

void make_vectors(mySpec**, int, BoWords*, float***, float***, int**, int*, int*);

/// ~~~~~~~~~~~~~~~~ TESTING ~~~~~~~~~~~~~~~~
void make_tests(BoWords*, logM*, mySpec**, int);

/* NOT USED */
void train_per_clique(myMatches*, mySpec**, int, BoWords*, logM*);
logM** make_models_array(BoWords*, mySpec**, matchesInfo*, int);

// ~~~~~~~~~~~~~~~~ SIGNALS ~~~~~~~~~~~~~~~~

void sig_int_quit_handler(int);

#endif