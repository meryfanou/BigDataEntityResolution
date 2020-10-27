/* myMatches.c */
#include <stdio.h>
#include <stdlib.h>
#include "../include/myMatches.h"

matchesInfo* matchesInfoInit(){
		// Init new Info List
	matchesInfo* newInfo = malloc(sizeof(matchesInfo));

	newInfo->entries = 0;
	newInfo->head = NULL;

	return newInfo;
}

myMatches* matchesAdd(matchesInfo* myInfo, mySpec* spec){

		// Init new Match
	myMatches* newMatch = myMatchesInit();
	pushMatch(newMatch, spec);

		// Insert on Head
 	if(myInfo->entries != 0)
		myInfo->head->prev = newMatch;

	newMatch->next = myInfo->head;
	myInfo->head = newMatch;

		// Update Info Stats
	myInfo->entries++;

	return newMatch;
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
		mySpec** temp = realloc(curMatch->specsTable, sizeof(mySpec*)*(curMatch->specsCount+1));
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
	free(match->specsTable);

		// Fix list pointers
	myMatches* temp = match->prev;
	if(match->prev != NULL)
		match->prev->next = match->next;
	if(match->next != NULL)
		match->next->prev = temp;

	free(match);
}

void deleteInfo(matchesInfo* myInfo){
	int count = myInfo->entries;
	while(count > 0){
		// printf("delete Info count: %d\n", count);
		myMatches* temp = myInfo->head;
		myInfo->head = myInfo->head->next;
		
		deleteMatches(temp);
		
		count--;
	}
	free(myInfo);
}

void mergeMatches(matchesInfo* myInfo, myMatches* match1, myMatches* match2){

	// !! MERGE AT MATCH_1 !!

	// Check if match 2 (to be deleteed after) is head
	// if its is, swap
	if(match2->prev == NULL){
		mergeMatches(myInfo, match2, match1);
		return;
	}


	// Combine matches Tables

		// Count total specs
	int totalCounts = match1->specsCount + match2->specsCount;

		// Realloc Mem at match1
	mySpec** tempTable = realloc(match1->specsTable, totalCounts*sizeof(mySpec*));
	if(tempTable == NULL){
		printf("Error at realloc (merge) !!\n");
		return;
	}
	match1->specsTable = tempTable;

		// Copy Specs from match2 -> match1
	int i = 0;
	while(match1->specsCount < totalCounts){
		match1->specsTable[match1->specsCount] = match2->specsTable[i];
		match1->specsCount++;
		i++;
	}

		// Delete match2
	deleteMatches(match2);  //Note: deleteMatches() DOES fix the list pointers !!

	myInfo->entries--;
}


void printMatchesList(matchesInfo* myInfo){
	printf("~ Matches\n");
	printf("--------------------------------\n");

	myMatches* temp = myInfo->head;
	
	printf("Matches Entries: %d\n", myInfo->entries);
	int countEntries = 0;
	while(countEntries < myInfo->entries){
		int countSpecsMatches = temp->specsCount;
		printf("Group %d specIDs:\n", countEntries);
		printf("\t-> ");
		while(countSpecsMatches > 0){
			printf("%s, ", temp->specsTable[--countSpecsMatches]->specID);
		}
		printf("\n");
		countEntries++;
		temp = temp->next;
	}
	printf("--------------------------------\n");

printf("\n\n");
}