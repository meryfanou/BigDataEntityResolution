/* functs.h
	Functs for main*/

#include "myHash.h"
#include "myMatches.h"
#include "mySpec.h"

int readCSV(char* , hashTable* , matchesInfo*); //reads csv, creates matches at hash values
char*** readFile(FILE*, int*, char***);

void swapSpecsMatches(mySpec*, mySpec*);