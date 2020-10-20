/* myMatches.h */

// -------------------------

typedef struct myMatches myMatches;
typedef struct matchesInfo matchesInfo;

struct myMatches{
	int specsCount;
	mySpec** specsTable;

	myMatches* next;
	myMatches* prev;
}

struct matchesInfo{
	int entries;
	myMatches* head;
}

// -------------------------


matchesInfo* matchesInfoInit();
void matchesAdd(matchesInfo*, mySpec*);

myMatches* myMatchesInit();
void pushMatch(myMatches*, mySpec*);

void deleteMatches(myMatches*);
void deleteInfo(matchesInfo*);

void mergeMatches(myMatches*, myMatches*);
