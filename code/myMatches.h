/* myMatches.h */

// -------------------------

typedef struct myMatches myMatches;
typedef struct matchesInfo matchesInfo;
typedef struct mySpec mySpec;

struct myMatches{
	int specsCount;
	mySpec** specsTable;

	myMatches* next;
	myMatches* prev;
};

struct matchesInfo{
	int entries;
	myMatches* head;
};

// -------------------------


matchesInfo* matchesInfoInit();
myMatches* matchesAdd(matchesInfo*, mySpec*);

myMatches* myMatchesInit();
void pushMatch(myMatches*, mySpec*);

void deleteMatches(myMatches*);
void deleteInfo(matchesInfo*);

void mergeMatches(myMatches*, myMatches*);
