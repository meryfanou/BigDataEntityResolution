//typedef struct hashTable hashTable;
//typedef struct specInfo specInfo;

/* functs.h
	Functs for main*/

#include "myHash.h"
#include "myMatches.h"
#include "mySpec.h"

extern int received_signal;

int readDataset(DIR*, char*, hashTable**, matchesInfo*);	// reads from dataset and parse json form properly

specInfo** readFile(FILE*, int*, specInfo**);	// called by readDataset on each inner file

int readCSV(char* , hashTable* , matchesInfo*);	//reads csv, creates matches at hash values

void sig_int_quit_handler(int);