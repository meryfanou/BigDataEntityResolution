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

extern int received_signal;

int readDataset(DIR*, char*, hashTable**, matchesInfo*);	// reads from dataset and parse json form properly

specInfo** readFile(FILE*, int*, specInfo**);	// called by readDataset on each inner file

int readCSV(char* , hashTable* , matchesInfo*);	//reads csv, creates matches at hash values

void text_to_bow(mySpec**, int, BoWords**);
void spec_to_bow(mySpec*, BoWords*);
char* checkWord(char*);
void sentence_to_bow(char*, mySpec*, BoWords*);

void sig_int_quit_handler(int);

#endif