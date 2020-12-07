/* myMatches.h */
#ifndef MYMATCHES_H
#define MYMATCHES_H
#include "mySpec.h"

// -------------------------

typedef struct myMatches myMatches;
typedef struct matchesInfo matchesInfo;
typedef struct mySpec mySpec;
struct myMatches{
	int specsCount;
	mySpec** specsTable;

	myMatches** negativeMatches;
	int negative_count;

	myMatches* next;
	myMatches* prev;
};

struct matchesInfo{
	int entries;
	myMatches* head;


};

// -------------------------

matchesInfo* matchesInfoInit();
void deleteInfo(matchesInfo*);
myMatches* matchesAdd(matchesInfo*, mySpec*);
void mergeMatches(matchesInfo*, myMatches*, myMatches*);

void combineMatchesTables(myMatches*, myMatches*);

void updateNegativeMatches(myMatches*, myMatches*);
void combineNegativeTables(myMatches*, myMatches*);

void printMatchesList(matchesInfo*); 		// TESTING
void extractMatches(matchesInfo*, char*);


myMatches* myMatchesInit();
void deleteMatches(matchesInfo*, myMatches*);
void pushMatch(myMatches*, mySpec*);

int findMatchinNegatives(myMatches**, int, myMatches*);

myMatches** removeCell(myMatches**, int, myMatches*);

void split_train_test_valid(matchesInfo*, mySpec***, mySpec***, mySpec***, int*, int*, int*, float, float);

#endif 