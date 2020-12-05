#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/bowords.h"


BoWords* bow_create(int HashSize, int BucSize){
	// ALLOCARE AND RETURN NEW HASH_TABLE

	BoWords* newTable = malloc(sizeof(BoWords));

	newTable->myTable = malloc(HashSize*sizeof(Bucket*));
	int i = 0;
	while(i < HashSize){
		newTable->myTable[i] = NULL;
		i++;		
	}

	newTable->bucSize = BucSize;
	newTable->tableSize = HashSize;

	newTable->maxRecs = (BucSize - sizeof(Bucket*)) / sizeof(Record);
	newTable->entries = 0;
	
	return newTable;
}