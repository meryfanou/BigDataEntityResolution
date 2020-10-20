/* mySpec.h */

// -----------------------------

typedef struct mySpec mySpec;
typedef struct specListNode specNode;
typedef struct specList specList;

struct mySpec{
	char* specID;
};

struct specListNode{
	mySpec* spec;
	specListNode* next;
};

struct specList{
	int count;
	specListNode* head;
};

// ------------------------------

specList* specListInit();
specNode* specNodeInit();
mySpec* specInit(char*);

void specAdd(specList*, mySpec*);
void deleteList(specList*);
void deleteNodes(specNode*);
void deleteSpec(mySpec*);