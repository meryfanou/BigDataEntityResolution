/* mySpec.h */
#include "myMatches.h"
// -----------------------------

typedef struct mySpec mySpec;
typedef struct specListNode specNode;
typedef struct specList specList;

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