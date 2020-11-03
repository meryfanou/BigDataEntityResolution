/* mySpec.c */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "../include/mySpec.h"

mySpec* specInit(char* id, char**** properties, int propNum){
	mySpec* newSpec = malloc(sizeof(mySpec));

	newSpec->specID = strdup(id);
	newSpec->matches = NULL;
	newSpec->propNum = propNum;

	newSpec->properties = (char***)malloc(propNum*sizeof(char**));
	for(int i=0; i<propNum; i++){
		newSpec->properties[i] = (char**)malloc(2*sizeof(char*));
		newSpec->properties[i][0] = strdup((*properties)[i][0]);
		newSpec->properties[i][1] = strdup((*properties)[i][1]);
	}

	return newSpec;
}

void updateSpecMatches(mySpec* spec, myMatches* matches){
	spec->matches = matches;
}

/*
specList* specListInit(){
	specList* newList = malloc(sizeof(specList));
	
	newList->count = 0;
	newList->head = NULL;

	return newList;
}

specNode* specNodeInit(){
	specNode* newNode = malloc(sizeof(specNode));

	newNode->next = NULL;
	newNode->spec = NULL;
}

void specAdd(specList* myList, mySpec* spec){
	specNode* myNode = specNodeInit();
	
	myNode->spec = spec;
	myNode->next = myList->head;
	myList->head = myNode;

	myList->count++;
}

void deleteList(specList* myList){
	deleteNodes(myList->head);
	free(myList);
}

void deleteNodes(specNode* myNode){
	if(myNode->next != NULL)
		deleteNodes(myNode->next);

	deleteSpec(myNode->spec);
	free(myNode);
}
*/

void deleteSpec(mySpec* spec){

	//
	//	ADD FREES FOR CHAR* / MALLOCS etc
	//
	free(spec->specID);

	for(int i=0; i<spec->propNum; i++){
		free(spec->properties[i][0]);
		free(spec->properties[i][1]);
		free(spec->properties[i]);
	}
	free(spec->properties);

	free(spec);
}

void printSpec(mySpec* spec){
	printf("specID: %s\n", spec->specID);

	printf("Properties:\n");
	for(int i=0; i<spec->propNum; i++){
		printf("%s:\t%s\n", spec->properties[i][0], spec->properties[i][1]);
	}
}


/*
void printList(specList* myList){
	specNode* temp = NULL;

	printf("~ Specs\n");
	printf("--------------------------------\n");

	temp = myList->head;
	while(temp != NULL){
		printf("|\tspecID: %s\n", temp->spec->specID);
		temp = temp->next;
	}

	printf("--------------------------------\n");
	printf("\n\n");
}
*/



// void printSpecMatches(mySpec* spec){
// 	int countSpecsMatches = spec->specsCount;
// 	printf("Group %d specIDs:\n", countEntries);
// 	printf("\t-> ");
// 	while(countSpecsMatches > 0){
// 		printf("%s, ", temp->specsTable[--countSpecsMatches]->specID);
// 	}
// 	printf("\n");
// }