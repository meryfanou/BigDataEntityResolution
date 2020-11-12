//typedef struct hashTable hashTable;
//typedef struct specInfo specInfo;

/* functs.h
	Functs for main*/

#include "myHash.h"
#include "myMatches.h"
#include "mySpec.h"

void readDataset(DIR*, char*, hashTable**, matchesInfo*);

specInfo** readFile(FILE*, int*, specInfo**);

int readCSV(char* , hashTable* , matchesInfo*); //reads csv, creates matches at hash values

int swapSpecsMatches(mySpec*, mySpec*);
