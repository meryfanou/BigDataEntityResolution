/* mySpec.h */
#ifndef MYSPEC_H
#define MYSPEC_H
#include "myMatches.h"

// -----------------------------

typedef struct mySpec mySpec;
//typedef struct specListNode specNode;
//typedef struct specList specList;
typedef struct myMatches myMatches;

struct mySpec{
	myMatches* matches;
	char* specID;
	char*** properties;
	int propNum;
};

// ------------------------------

mySpec* specInit(char*, char****, int);
void deleteSpec(mySpec*);
void updateSpecMatches(mySpec*, myMatches*);

void printSpec(mySpec*);

//void printList(specList*); // FOR TESTING
//void printSpecMatches(mySpec*); 		// TESTING

#endif