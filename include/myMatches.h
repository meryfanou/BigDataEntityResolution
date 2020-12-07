/* myMatches.h */
#ifndef MYMATCHES_H
#define MYMATCHES_H
#include "mySpec.h"

// -------------------------

typedef struct myMatches myMatches;
typedef struct matchesInfo matchesInfo;
typedef struct mySpec mySpec;
typedef struct neg_list nlist;
typedef struct neg_node nNode;

struct myMatches{
	int specsCount;
	mySpec** specsTable;

	myMatches** negativeMatches;
	int negative_count;

	nlist* negs;

	myMatches* next;
	myMatches* prev;
};

struct matchesInfo{
	int entries;
	myMatches* head;
};

struct neg_list{
	nNode* head;
	nNode* tail;
	int entries;
};

struct neg_node{
	myMatches* matchptr;
	nNode* next;
	nNode* prev;
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
myMatches** addCell(myMatches**, int, myMatches*);

void printMatchNeg(matchesInfo*);

nlist* create_nlist();
void add_nlist(nlist*, myMatches*);
void remove_nlist(nlist*, myMatches*);
nNode* seek_nlist(nlist*, myMatches*);
void destroy_nlist(nlist*);

nNode* create_nNode(myMatches*);
void destroy_nNode(nNode*);


#endif