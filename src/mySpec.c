/* mySpec.c */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "../include/mySpec.h"


mySpec* specInit(){			// MALLOC AND RETURN NEW SPEC NODE
	mySpec	*spec = malloc(sizeof(mySpec));

	spec->matches = NULL;
	spec->specID = NULL;
	spec->properties = NULL;
	spec->propNum = 0;

	return spec;
}

mySpec* specCreate(char* id, specInfo** properties, int propNum){	//SET NEW SPEC NODE
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

int swapSpecsMatches(mySpec* dest, mySpec* source){

    // SETTING EACH SPEC->MATCHES POINTER TO NEW MATCH GROUP

    if(dest == NULL || source == NULL)
        return -1;

    int i = 0;

    while(i < source->matches->specsCount){
        if(source->matches->specsTable[i] != source)
            source->matches->specsTable[i]->matches = dest->matches;
        i++;
    }

    return 1;
}

void deleteSpec(mySpec* spec){		// free mem
	if(spec->specID != NULL)
		free(spec->specID);

	for(int i=0; i<spec->propNum; i++){
		specDelInfo((spec->properties)[i]);
		free((spec->properties)[i]);
	}
	if(spec->properties != NULL)
		free(spec->properties);

	free(spec);
	spec = NULL;
}

void printSpec(mySpec* spec){	// printing specs info / testing funct
	printf("specID: %s\n", spec->specID);

	printf("Properties:\n");
	for(int i=0; i<spec->propNum; i++){
		printf("%s:\t%s\n", (spec->properties)[i]->key, (spec->properties[i])->values->value);
	}
}

void specAddInfo(specInfo *info, specInfo *newInfo){	// add info to spec
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

void specDelInfo(specInfo *info){	 // ~~~ FREE MEM
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