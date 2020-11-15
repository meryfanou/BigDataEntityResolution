/* mySpec.h */
#ifndef MYSPEC_H
#define MYSPEC_H
#include "myMatches.h"

// -----------------------------

typedef struct mySpec mySpec;
typedef struct specInfo specInfo;
typedef struct specValue specValue;
typedef struct myMatches myMatches;

struct mySpec{
	myMatches* matches;
	char* specID;
	specInfo** properties;
	int propNum;
};

struct specInfo{
	char* key;
	specValue* values;
};

struct specValue{
	char* value;
	specValue* next;
};

// ------------------------------

mySpec* specInit();
mySpec* specCreate(char*, specInfo**, int);
void deleteSpec(mySpec*);
void updateSpecMatches(mySpec*, myMatches*);
int swapSpecsMatches(mySpec*, mySpec*);
void printSpec(mySpec*);

void specAddInfo(specInfo*, specInfo*);
void specDelInfo(specInfo*);


#endif