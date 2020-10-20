/* mySpec.c */

#include "mySpec.h"

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
	specNode myNode = specNodeInit();
	
	myNode->spec = spec;
	myNode->next = myList->head;
	myList->head = myNode;

	myList->count++;
}
