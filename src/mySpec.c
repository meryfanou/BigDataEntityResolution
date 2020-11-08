/* mySpec.c */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "../include/mySpec.h"


mySpec* specInit(){
	mySpec	*spec = malloc(sizeof(mySpec));

	spec->matches = NULL;
	spec->specID = NULL;
	spec->properties = NULL;
	spec->propNum = 0;

	return spec;
}

mySpec* specCreate(char* id, specInfo** properties, int propNum){
	mySpec* newSpec = specInit();

	newSpec->specID = strdup(id);
	newSpec->propNum = propNum;
	newSpec->properties = malloc(propNum*sizeof(specInfo*));

	for(int i=0; i<propNum; i++){
		(newSpec->properties)[i] = malloc(sizeof(specInfo));
		specAddInfo((newSpec->properties)[i], properties[i]);
	}

	return newSpec;
}

void updateSpecMatches(mySpec* spec, myMatches* matches){
	spec->matches = matches;
}

void deleteSpec(mySpec* spec){
	if(spec->specID != NULL)
		free(spec->specID);

	for(int i=0; i<spec->propNum; i++){
		specDelInfo((spec->properties)[i]);
		free((spec->properties)[i]);
	}
	if(spec->properties != NULL)
		free(spec->properties);

	//if(spec->matches != NULL)
	//	free(spec->matches);

	free(spec);
	spec = NULL;
}

void printSpec(mySpec* spec){
	printf("specID: %s\n", spec->specID);

	printf("Properties:\n");
	for(int i=0; i<spec->propNum; i++){
		printf("%s:\t%s\n", (spec->properties)[i]->key, (spec->properties[i])->values->value);
	}
}

void specAddInfo(specInfo *info, specInfo *newInfo){
	specValue	*head = NULL, *current = NULL, *prev = NULL, *newValues = NULL;

	info->key = strdup(newInfo->key);

	newValues = newInfo->values;
	while(newValues != NULL){
		current = malloc(sizeof(specValue));
		current->value = strdup(newValues->value);
		current->next = newValues->next;

		if(head == NULL)
			head = current;
		if(prev != NULL)
			prev->next = current;

		prev = current;
		newValues = newValues->next;
	}

	info->values = head;
}

void specDelInfo(specInfo *info){
	free(info->key);

	specValue	*current = info->values;
	specValue	*temp;
	while(current != NULL){
		temp = current->next;
		free(current->value);
		free(current);

		current = temp;
	}
}