/* mySpec.h */
#ifndef MYSPEC_H
#define MYSPEC_H
// #include "myMatches.h"

// -----------------------------

typedef struct mySpec mySpec;
typedef struct specListNode specNode;
typedef struct specList specList;
typedef struct myMatches myMatches;

struct mySpec{
	myMatches* matches;
	char* specID;
};

struct specListNode{
	mySpec* spec;
	specNode* next;
};

struct specList{
	int count;
	specNode* head;
};

// ------------------------------

specList* specListInit();
specNode* specNodeInit();
mySpec* specInit(char*);

void specAdd(specList*, mySpec*);
void deleteList(specList*);
void deleteNodes(specNode*);
void deleteSpec(mySpec*);
void updateSpecMatches(mySpec*, myMatches*);

void printList(specList*); // FOR TESTING
// void printSpecMatches(mySpec*); 		// TESTING

#endif