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
void train_per_clique(myMatches*, mySpec**, int, BoWords*);


// ~~~~~~~~~~~~~~~~ SIGNALS ~~~~~~~~~~~~~~~~

void sig_int_quit_handler(int);

#endif