/* myMatches.c */
#include "myMatches.h"

matchesInfo* matchesInfoInit(){
		// Init new Info List
	matchesInfo* newInfo = malloc(sizeof(matchesInfo));

	newInfo->entries = 0;
	newInfo->head = NULL;

	return newInfo;
}

void matchesAdd(matchesInfo* myInfo, mySpec* spec){

		// Init new Match
	myMatches* newMatch = myMatchesInit();
	pushMatch(newMatch, spec);

		// Insert on Head
	myInfo->head->prev = newMatch;
	newMatch->next = myInfo->head;
	myInfo->head = newMatch;

		// Update Info Stats
	myInfo->entries++;
}


myMatches* myMatchesInit(){
		// Init new Match group
	myMatches* newMatch = malloc(sizeof(myMatches));

	newMatch->specsCount = 0;
	newMatch->specsTable = NULL;
	
	newMatch->next = NULL;
	newMatch->prev = NULL;

	return newMatch;
}

void pushMatch(myMatches* curMatch, mySpec* spec){
		//Add new Spec to this-> match group
	
		// Alloc mem
	if(curMatch-> specsCount == 0){
		curMatch->specsTable = malloc(sizeof(mySpec*));
	}
	else{
		mySpec** temp = realloc(curMatch->specsTable, sizeof(mySpec*)*curMatch->specsCount+1);
		if(temp == NULL){
			printf("Error at realloc !!");
			//
			//	FREE MEM AND EXIT
			//
			return;
		}	
		curMatch->specsTable = temp;
	}

		// Add spec to match-table
	curMatch->specsTable[curMatch->specsCount] = spec;
		// Update matches Info
	curMatch->specsCount++;

}

void deleteMatches(myMatches* match){
	int count = match->specsCount;
	while(count > 0){
		free(match->specsTable[--count]);
	}
}

void deleteInfo(matchesInfo* myInfo){
	int count = myInfo->entries;
	while(count > 0){
		
		myMatches* temp = myInfo->head;
		myInfo->head = myInfo->head->next;
		
		deleteMatches(temp);
		
		count--;
	}
}



//
//	TO DO:
//		 	- DELETE METHODS
//			- MERGE METHOD
//