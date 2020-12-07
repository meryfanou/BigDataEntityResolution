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

	newMatch->negs = create_nlist();
	
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

	destroy_nlist(match->negs);

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
	deleteMatches(myInfo, match2);  //Note: deleteMatches() DOES fix the list pointers !!
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
	// COMBINE AT MATCH_1 !!!!!!!!

		// change match2 negatives matches to point at match1 &&  add match2 negatives to match1

	nNode* temp = match2->negs->head;

	while(temp != NULL){

		// List Way
		updateNegativeMatches(temp->matchptr, match1);
		remove_nlist(temp->matchptr->negs, match2);
		
		temp = temp->next;
	}
}

void updateNegativeMatches(myMatches* match1, myMatches* match2){
	/// ~~~~ LIST WAY
	nNode* exists = seek_nlist(match1->negs, match2);
	if(exists != NULL){ 	// matches already linked !!
		return;
	}

	add_nlist(match1->negs, match2);
	add_nlist(match2->negs, match1);
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

void printMatchNeg(matchesInfo* info){
	myMatches* temp = info->head;
	int count = 0;
	while(temp != NULL){
		if(temp->negs->entries > 2)
			printf("%d > %d\n", count, temp->negs->entries);

		count++;
		temp = temp->next;
	}
}

nlist* create_nlist(){
	nlist* newlist = malloc(sizeof(nlist));

	newlist->head = NULL;
	newlist->tail = NULL;
	newlist->entries = 0;

	return newlist;
}

void add_nlist(nlist* mylist, myMatches* match1){
	// printf("Apo add, entries: %d\n", mylist->entries);
	nNode* temp = seek_nlist(mylist, match1);

	if(temp != NULL){
		return;
	}

	if(mylist->entries == 0){
		// printf("\n\n!!!! EKANA GAMIDI ADD STO HEAD !!!!\n\n");
		mylist->head = create_nNode(match1);
		mylist->tail = mylist->head;
	}
	else{
		mylist->tail->next = create_nNode(match1);
		mylist->tail->next->prev = mylist->tail;
		mylist->tail = mylist->tail->next;
	}

	mylist->entries++;
	// printf("\t vghke apo add, entries: %d\n", mylist->entries);
}

void remove_nlist(nlist* mylist, myMatches* match1){
	nNode* temp = seek_nlist(mylist, match1);

	if(temp == NULL){
		return;
	}

		// Change ptrs
	if(mylist->head != temp){
		temp->prev->next = temp->next;
	}
	else{
		mylist->head = NULL;
	}
	temp->next->prev = temp->prev;

	mylist->entries--;

	destroy_nNode(temp);

	// printf("ekana gamhmeno remove\n");
}

nNode* seek_nlist(nlist* mylist, myMatches* match1){
	if(mylist == NULL)
		return NULL;

	if(mylist->entries == 0){
		return NULL;
	}

	// printf("seek > enrtires: %d\n", mylist->entries);
	
	nNode* seek = mylist->head;
	while(seek != NULL){
		// printf("mple\n");
		if(seek->matchptr == match1)
			return seek;
		seek = seek->next;
	}

	return NULL;
}

void destroy_nlist(nlist* mylist){
	nNode* temp = mylist->tail;
	while(temp != NULL){
		mylist->tail = temp->prev;
		destroy_nNode(temp);
		temp = mylist->tail;
	}

	free(mylist);
}

nNode* create_nNode(myMatches* match){
	nNode* newNode = malloc(sizeof(nNode));

	newNode->matchptr = match;
	newNode->prev = NULL;
	newNode->next = NULL;

	return newNode;
}

void destroy_nNode(nNode* myNode){
	free(myNode);
}

void split_train_test_valid(matchesInfo* allMatches, mySpec*** trainSet, mySpec*** testSet, mySpec*** validSet, int* trainSize, int* testSize, int* validSize, float trainPerc, float testPerc){

	myMatches	*match = allMatches->head;

	// A percentage of all the specs will be used for the training set
	int		trainingNum = (int)ceil((allMatches->entries)*trainPerc);
	// Another percentage will be used for the testing set
	int		testingNum = (int)ceil((allMatches->entries)*testPerc);
	// The rest will be used for the validation set
	int		validationNum = allMatches->entries - trainingNum - testingNum;

	*trainSet = NULL;
	*testSet = NULL;
	*validSet = NULL;

	*trainSize = 0;
	*testSize = 0;
	*validSize = 0;
	int	currTrain, currTest, currValid;
	// For each set of specs, where all specs match with each other
	while(match != NULL){
		// A percentage of the specs will be used for the training set
		currTrain = (int)ceil((match->specsCount)*trainPerc);
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
			// Another percentage of the specs will be used for the testing set
			currTest = (int)ceil((match->specsCount)*testPerc);
			currTest = (currTest > (match->specsCount - currTrain)) ? (match->specsCount - currTrain) : (currTest);
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

			// The rest of the specs will be used for the validation set
			currValid = match->specsCount - currTrain - currTest;
			currValid = (currValid >= validationNum) ? (validationNum) : (currValid);

			if(currValid > 0){
				*validSet = realloc(*validSet, ((*validSize)+currValid)*sizeof(mySpec*));
				// 'Copy' the chosen specs to the validation set
				for(int i=(currTrain+currTest); i<(currTrain+currTest+currValid); i++){
					(*validSet)[i-(currTrain+currTest)+(*validSize)] = match->specsTable[i];
				}
				validationNum -= currValid;
				(*validSize) += currValid;
			}
		}

		match = match->next;
	}
}

