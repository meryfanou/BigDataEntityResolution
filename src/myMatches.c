/* myMatches.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <error.h>
#include <unistd.h>
#include <math.h>
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
	if(spec == NULL)
		return NULL;
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

	//spec->matches = newMatch;

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

	newMatch->negativeMatches = NULL;
	newMatch->negative_count = 0;
	
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
		// Update matches Info
	curMatch->specsCount++;

}

void deleteMatches(matchesInfo* myInfo, myMatches* match){	// free mem
	if(match == NULL)
		return;
		// Fix list pointers
	if(match->prev != NULL)
		match->prev->next = match->next;
	if(match->next != NULL)
		match->next->prev = match->prev;

	if(match->specsTable != NULL)
		free(match->specsTable);


	if(match->negativeMatches != NULL){
		free(match->negativeMatches);
	}

	if(myInfo != NULL){
		if(myInfo->head == 	match)
			myInfo->head = match->next;
		free(match);

		myInfo->entries--;
	}else{
		free(match);
	}

	// match = NULL;
}

void deleteInfo(matchesInfo* myInfo){ 	// free mem
	int count = myInfo->entries;
	while(count > 0){
		// printf("delete Info count: %d\n", count);
		myMatches* temp = myInfo->head;
		myInfo->head = myInfo->head->next;

		deleteMatches(myInfo, temp);
		
		count--;
	}

	free(myInfo);
}

void mergeMatches(matchesInfo* myInfo, myMatches* match1, myMatches* match2){

	// !! MERGE AT MATCH_1 !!

	if(match1 == NULL || match2 == NULL)
		return;

	// Combine matches Tables
	combineMatchesTables(match1, match2);


	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

	// Combine Negatives tables
	combineNegativeTables(match1, match2);


		// Delete match2
	// deleteMatches(myInfo, match2);  //Note: deleteMatches() DOES fix the list pointers !!
}

void combineMatchesTables(myMatches* match1, myMatches* match2){
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
}

void combineNegativeTables(myMatches* match1, myMatches* match2){

		// change match2 negatives matches to point at match1 &&  add match2 negatives to match1
	int neg1count = 0;
	myMatches* temp = NULL;
	while(neg1count < match2->negative_count){
		temp = match2->negativeMatches[neg1count++];
		
		if(temp == match1){
			// printf("GAMW TA DATA MOU GAMW AAAAAAAAAAAAAAAAAAAAAAA\n");
			continue;
		}
			// modify temps pointers

				// add match1 at temp negatives table ( !!!! 2 SIDE OPERATION !!!! )
		updateNegativeMatches(temp, match1);
		
				// remove match2 pointer from temp negatives table
		temp->negativeMatches = removeCell(temp->negativeMatches, temp->negative_count, match2);
		temp->negative_count -= 1;
	}
}

void updateNegativeMatches(myMatches* match1, myMatches* match2){
	int exist = findMatchinNegatives(match1->negativeMatches, match1->negative_count, match2);
	if( exist == -1){
		return;
	}

	match1->negativeMatches = realloc(match1->negativeMatches, (match1->negative_count+1)*sizeof(myMatches*));
	match1->negativeMatches[match1->negative_count] = match2;
	match1->negative_count++;

	match2->negativeMatches = realloc(match2->negativeMatches, (match2->negative_count+1)*sizeof(myMatches*));
	match2->negativeMatches[match2->negative_count] = match1;
	match2->negative_count++;

}

void printMatchesList(matchesInfo* myInfo){		// testing funct - prints matches list
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

	// CHECK IF FNAME == NULL
	// CASE 1: CREATE DIR AND FILE -> PRINT TO FILE
	// CASE 2: PRINT TO STDOUT


	FILE* fpout;
	if(fname != NULL){
		flag = 1;

		int len = strlen(PATH) + 1 + strlen(fname) + 1;

			// CHECK IF DIR ALREADY EXISTS - CREATE IT IF IT DOESNT
			// !!!!! DIRS NAME IS DEFINED AT PATH !!

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

			// CREATE FILE WITH NAME: FNAME INT TARGET DIR
		fpout = fopen(target, "w+");

		free(target);
	}	
	else{
		fpout = stdout;
	}
		
		// start printing

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
			fprintf(fpout, "\n\t~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~\n\n");
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


int findMatchinNegatives(myMatches** arr, int counts, myMatches* target){
	if(arr == NULL || target == NULL || counts == 0){
		return -1;
	}

	int i = 0;
	// printf("counts: %d\n", counts);
	while(i < counts){
		// printf("i: %d\n", i);
		if(arr[i] == target){
			return i;
		}

		i++;
	}

	return -1;
}

myMatches** removeCell(myMatches** array, int entries, myMatches* target){
	myMatches** newArray = malloc((entries-1)*sizeof(myMatches*));

	int i = 0;
	while(i < entries-1){
		if(newArray[i] != target){
			newArray[i] = array[i];
		}
		i++;
	}

	printf("Array Got Smaller !!\n");

	free(array);
	return newArray;
}


void split_train_n_test(matchesInfo* allMatches, mySpec*** trainSet, mySpec*** testSet, float percentage, int* trainSize, int* testSize){

	myMatches	*match = allMatches->head;

	// A percentage of all the specs will be used for the training set
	int			trainingNum = (int)ceil((allMatches->entries)*percentage);
	// The rest will be used for the testing set
	int			testingNum = allMatches->entries - trainingNum;

	*trainSet = NULL;
	*testSet = NULL;

	*trainSize = 0;
	*testSize = 0;

	int	currTrain, currTest;
	// For each set of specs, where all specs match with each other
	while(match != NULL){
		// A percentage of the specs will be used for the training set
		currTrain = (int)ceil((match->specsCount)*percentage);
		currTrain = (currTrain >= trainingNum) ? (trainingNum) : (currTrain);

		if(currTrain > 0){
			*trainSet = realloc(*trainSet, ((*trainSize)+currTrain)*sizeof(mySpec*));
		}
		// 'Copy' the chosen specs to the training set
		for(int i=0; i<currTrain; i++){
			(*trainSet)[i+(*trainSize)] = match->specsTable[i];
		}
		trainingNum -= currTrain;
		(*trainSize) += currTrain;

		// If there are specs left for the testing set
		if(match->specsCount > 1 || trainingNum == 0){
			// The rest of the specs will be used for the testing set
			currTest = match->specsCount - currTrain;
			currTest = (currTest >= testingNum) ? (testingNum) : (currTest);

			if(currTest > 0){
				*testSet = realloc(*testSet, ((*testSize)+currTest)*sizeof(mySpec*));
				// 'Copy' the chosen specs to the testing set
				for(int i=currTrain; i<(currTrain+currTest); i++){
					(*testSet)[i-currTrain+(*testSize)] = match->specsTable[i];
				}
				testingNum -= currTest;
				(*testSize) += currTest;
			}
		}

		match = match->next;
	}
}

