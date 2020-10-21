/* mySpec.c */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "mySpec.h"

mySpec* specInit(char* id){
	mySpec* newSpec = malloc(sizeof(mySpec));
	newSpec->specID = strdup(id);
	newSpec->matches = NULL;
}

void updateSpecMatches(mySpec* spec, myMatches* matches){
	spec->matches = matches;
}

specList* specListInit(){
	specList* newList = malloc(sizeof(specList));
	newList->count = 0;

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
}

void deleteNodes(specNode* myNode){
	if(myNode->next != NULL)
		deleteNodes(myNode->next);

	deleteSpec(myNode->spec);
	free(myNode);
}

void deleteSpec(mySpec* spec){

	//
	//	ADD FREES FOR CHAR* / MALLOCS etc
	//

	free(spec->specID);

}