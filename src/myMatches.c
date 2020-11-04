/* myMatches.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <error.h>
#include <unistd.h>
#include "../include/myMatches.h"

#define PATH "./Outputs"

matchesInfo* matchesInfoInit(){
		// Init new Info List
	matchesInfo* newInfo = malloc(sizeof(matchesInfo));
	if(newInfo == NULL){
		printf("ERROR ~ Cant allocate MEM ( matchesInfoInit() )\n");
		return NULL;
	}

	newInfo->entries = 0;
	newInfo->head = NULL;

	return newInfo;
}

myMatches* matchesAdd(matchesInfo* myInfo, mySpec* spec){
		// CHECK IF SPEC ALREADY HAS MATCHES
	if(spec->matches != NULL)
		return spec->matches;

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

	spec->matches = newMatch;

	return newMatch;
}


myMatches* myMatchesInit(){
		// Init new Match group
	myMatches* newMatch = malloc(sizeof(myMatches));
	if(newMatch == NULL){
		printf("ERROR ~ Cant allocate MEM ( myMatchesInit() )\n");
		return NULL;
	}

	newMatch->specsCount = 0;
	newMatch->specsTable = NULL;
	
	newMatch->next = NULL;
	newMatch->prev = NULL;

	return newMatch;
}

void pushMatch(myMatches* curMatch, mySpec* spec){
		//Add new Spec to this-> match group
	
		// Alloc mem
	if(curMatch->specsCount == 0){
		curMatch->specsTable = malloc(sizeof(mySpec*));
		if(curMatch->specsTable == NULL){
			printf("ERROR ~ Cant allocate MEM ( pushMatch() )\n");
			return;
		}
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
	// memcpy(curMatch->specsTable[curMatch->specsCount], spec, 1*sizeof(mySpec*));
		// Update matches Info
	curMatch->specsCount++;

}

void deleteMatches(myMatches* match){
		// Fix list pointers
	if(match->prev != NULL)
		match->prev->next = match->next;
	if(match->next != NULL)
		match->next->prev = match->prev;

	if(match->specsTable != NULL)
		free(match->specsTable);
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

	// printf("\tMERGIND ... ");
	// !! MERGE AT MATCH_1 !!

	if(match1 == NULL || match2 == NULL)
		return;
	// Check if match 2 (to be deleteed after) is head
	// if its is, swap
	// if(match2->prev == NULL){
	// 	mergeMatches(myInfo, match2, match1);
	// 	return;
	// }


	// Combine matches Tables

		// Count total specs
	int totalCounts = match1->specsCount + match2->specsCount;

		// Realloc Mem at match1
	// printf("\ttotalCounts (merge): %d, count1: %d, count2: %d", totalCounts, match1->specsCount, match2->specsCount);
	mySpec** tempTable = realloc(match1->specsTable, totalCounts*sizeof(mySpec*));
	if(tempTable == NULL){
		printf("Error at realloc (merge) !!\n");
		return;
	}
	match1->specsTable = tempTable;

		// Copy Specs from match2 -> match1
	
	// memcpy(match1 + match1->specsCount*(sizeof(mySpec*)), match2, match2->specsCount*(sizeof(mySpec*)));
	// match1->specsCount += match2->specsCount;
	
	int i = 0;
	while(match1->specsCount < totalCounts){
		match1->specsTable[match1->specsCount] = match2->specsTable[i];
		match1->specsCount++;
		i++;
	}
	// printf("   -> count after merge: %d\n", match1->specsCount);

		// Delete match2
	deleteMatches(match2);  //Note: deleteMatches() DOES fix the list pointers !!
	myInfo->entries--;

	// printf(" .. DONE !!\n");
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

void extractMatches(matchesInfo* allMatches, char* fname){

	int totalPairs = 0;		// count total pairs (just for fun)
	int flag = 0;

	FILE* fpout;
	if(fname != NULL){
		flag = 1;

		int len = strlen(PATH) + 1 + strlen(fname) + 1;

		if(chdir(PATH) == -1){
			if(mkdir(PATH, S_IRWXU|S_IRWXG|S_IROTH)){ 
	    	  error(EXIT_FAILURE, errno, "Failed to create directory");
	   		}
	   	}
	   	else{
	   		chdir("..");
	   	}

		char* target = malloc(len);
		memset(target, 0 , len);

		strcat(target, PATH);
		strcat(target, "/");
		strcat(target, fname);

		fpout = fopen(target, "w+");

		free(target);
	}	
	else{
		fpout = stdout;
	}
		
	myMatches* tempMatches = allMatches->head;

	while(tempMatches != NULL){
		if(tempMatches->specsCount > 1){
			int printed = 0;
			while(printed < tempMatches->specsCount-1){

				int inside = printed + 1;
				while(inside < tempMatches->specsCount){
					fprintf(fpout, "%s, %s\n", tempMatches->specsTable[printed]->specID, tempMatches->specsTable[inside]->specID);

					inside++;
					totalPairs++;
				}
				printed++;
			}
			fprintf(fpout, "\t~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~\n");
		}
		tempMatches = tempMatches->next;
	}

			// STATS PRINTS (EXTRA) - COMMENT IF U LIKE
	fprintf(fpout, "\t~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~\n");
	fprintf(fpout, "Total Matches (Groups): %d\n", allMatches->entries);
	fprintf(fpout, "Total Pairs: %d\n", totalPairs);

	if(flag == 1)
		fclose(fpout);

}