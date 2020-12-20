#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/myHash.h"


hashTable* hash_create(int HashSize, int BucSize){
	// ALLOCARE AND RETURN NEW HASH_TABLE

	hashTable* newTable = malloc(sizeof(hashTable));

	newTable->myTable = malloc(HashSize*sizeof(bucket*));
	int i = 0;
	while(i < HashSize){
		newTable->myTable[i] = NULL;
		i++;		
	}

	newTable->bucSize = BucSize;
	newTable->tableSize = HashSize;

	newTable->maxRecs = (BucSize - sizeof(bucket*)) / sizeof(record);
	newTable->entries = 0;
	
	return newTable;
}

void hash_destroy(hashTable* table){	// DESTROY HASH / FREE MEM
	if(table == NULL)
		return;

	int i = 0;
	while(i < table->tableSize){
		if(table->myTable[i] != NULL){
			bucket_destroy(table, table->myTable[i]);
			table->myTable[i] = NULL;
		}
		i++;
	}

	free(table->myTable);
	free(table);

}


int hash1(char* key){			// HASH FUNCTION
	int max = strlen(key);
	int temp = 0;
	int sum = 1;
	int extra = 0;

	while(temp < max){
		sum += (key[temp] - '0' + extra);
		extra += (key[temp] - '0') % (temp+1);
		temp++;
	}

	// printf("key: %s > sum: %d\n", key, sum);

	return sum;
}

void hash_add(hashTable* table, mySpec* newSpec, int hash){

	if(newSpec == NULL)
		return;

	int cell = hash % (table->tableSize);		/// FIND HASH TABLE CELL
	if(cell < 0)
		cell = 0;

	if(table->myTable[cell] == NULL){			/// CASE OF FIRST BUCKET
		table->myTable[cell] = bucket_create(table->bucSize);
	}

	bucket* tempBuc = table->myTable[cell];
	int flag = 0;		// 0 ~ new key / 1 ~ existing key
	record* existingRec = NULL;
	if((existingRec = search_bucket(tempBuc, newSpec->specID)) != NULL)
		flag = 1;
	
	while(tempBuc->next != NULL && flag == 0){	/// FIND LAST BUCKET
		tempBuc = tempBuc->next;				/// + SEARCH FOR REC IN BUCS
		if((existingRec = search_bucket(tempBuc, newSpec->specID)) != NULL){
			flag = 1;
			break;
		}
	}

	if(flag == 1){
		// specID duplicates should not exist!
		printf("Error - Duplicate key found -> Table not updated !!\n");
		return;
	}
	else{
		if(tempBuc->cur >= table->maxRecs){			/// CHECK IF ITS FULL
			tempBuc->next = bucket_create(table->bucSize);
			tempBuc = tempBuc->next;
		}

		bucket_add(tempBuc, newSpec);					/// ADD AT BUCKET
		table->entries++;
	}

}

void hash_print(hashTable* t){			// testing funct - prints hash
	printf("HASH_SIZE: %d\n", t->tableSize);
	int i = 0;
	while(i < t->tableSize){
		if(t->myTable[i] != NULL){
			printf("cell [%d]:\n", i);
			bucket_print(t->myTable[i]);
		}
		i++;
	}
	printf("\n");
}


bucket* bucket_create(int size){				/// INIT BUCKET
	bucket* newBucket = malloc(sizeof(bucket));
	newBucket->next = NULL;
	newBucket->rec = NULL;
	newBucket->cur = 0;

	return newBucket;
}

void bucket_add(bucket* buc, mySpec* newSpec){
	if(buc->rec == NULL){
		buc->rec = record_create(newSpec);
		buc->cur++;
		return;
	}

	record* tempRec = buc->rec;					/// FIND LAST ENTRY
	while(tempRec->next != NULL){
		tempRec = tempRec->next;
	}

	tempRec->next = record_create(newSpec);				/// ADD RECORD
	buc->cur++;

}

record* search_bucket(bucket* b, char* key){	// search bucket for target key
	if(b == NULL)
		return NULL;
	record* temp = b->rec;

	while(temp != NULL){
		if(strcmp(temp->spec->specID, key) == 0)
			return temp;
		temp = temp->next;
	}

	return NULL;
}

void bucket_print(bucket* b){					// testing funct - prints bucket
	printf("\t>buc: (%d)\n", b->cur);
	record* temp = b->rec;
	while(temp != NULL){
		record_print(temp);
		temp = temp->next;
	}
	if(b->next != NULL){
		bucket_print(b->next);
	}
}

void bucket_destroy(hashTable* hash, bucket* buc){		/// FREE BUC
	if(buc->next != NULL){
		bucket_destroy(hash, buc->next);
		buc->next = NULL;
	}

	if(buc->rec != NULL){
		record_destroy(hash, buc->rec);
		buc->rec = NULL;
	}

	free(buc);
	buc = NULL;
}


record* record_create(mySpec* spec){
	record* newRec = malloc(sizeof(record));

	newRec->spec = spec;
	newRec->visited = 'n';
	newRec->next = NULL;

	return newRec;
}

void record_print(record* rec){
	printf("\t\t>key: %s\n", rec->spec->specID);

	// printSpec(rec->spec);
}

void record_destroy(hashTable* hash, record* rec){				/// FREE REC
	if(rec->next != NULL){
		record_destroy(hash, rec->next);
		rec->next = NULL;
	}

	if(rec->spec != NULL){
		deleteSpec(rec->spec);
		if(hash != NULL)
			hash->entries--;
	}
	free(rec);
	rec = NULL;
}

mySpec* findRecord_byKey(hashTable* table, char* key){
	int cell = hash1(key) % (table->tableSize);		/// FIND HASH TABLE CELL
	if(cell < 0)
		cell = 0;
	// printf("cell: %d, tSize: %d, maxRecs/buc: %d\n", cell, table->tableSize, table->maxRecs);

	if(table == NULL)
		return NULL;

	if(table->myTable[cell] == NULL)
		return NULL;


	bucket* tempBuc = table->myTable[cell];
	// printf("cell: %d\n", cell);
	int flag = 0;		// 0 ~ not found / 1 ~ found
	record* existingRec = NULL;
	
	while(flag == 0){	/// FIND LAST BUCKET
		if((existingRec = search_bucket(tempBuc, key)) != NULL){
			flag = 1;
			break;
		}
		if(tempBuc->next == NULL)
			break;
		tempBuc = tempBuc->next;
	}

	if(flag == 0)
		return NULL;
	return existingRec->spec;
}

// Find a record for either the testing or the validation set
mySpec* findRecord_forSet(hashTable* table, char* key, char set){
	int cell = hash1(key) % (table->tableSize);		/// FIND HASH TABLE CELL
	if(cell < 0)
		cell = 0;

	if(table == NULL)
		return NULL;

	if(table->myTable[cell] == NULL)
		return NULL;

	bucket* tempBuc = table->myTable[cell];
	int flag = 0;		// 0 ~ not found / 1 ~ found
	record* existingRec = NULL;
	
	while(flag == 0){	/// FIND LAST BUCKET
		if((existingRec = search_bucket(tempBuc, key)) != NULL){
			flag = 1;
			break;
		}
		if(tempBuc->next == NULL)
			break;
		tempBuc = tempBuc->next;
	}

	// If 'key' was not found in the hashtable
	if(flag == 0)
		return NULL;

	// If it is for the testing set
	if(set == 't'){
		// If current spec has not been added to the testing set
		if(existingRec->visited == 'n'){
			// Mark it as visited for the testing set
			existingRec->visited = 't';
			return existingRec->spec;
		}

		return NULL;
	}

	// If it is for the validation set
	if(set == 'v'){
		// If current spec has not been added to the validation set
		if(existingRec->visited == 'n' || existingRec->visited == 't'){
			// Mark it as visited for the validation set
			existingRec->visited = 'v';
			return existingRec->spec;
		}

		return NULL;
	}

	return NULL;
}


void hash_to_bow(hashTable* hash, BoWords* bow){
        // For each spec of the set (either training or testing)
    int i = 0;
	while(i < hash->tableSize){
		bucket* buc = hash->myTable[i];
		while(buc != NULL){
			printf("hash %d\n",i);
			hash_bucket_to_bow(buc, bow);
			buc = buc->next;
		}
		i++;
	}
}

void hash_bucket_to_bow(bucket* buc, BoWords* bow){
	record* rec = buc->rec;
	while(rec != NULL){
		hash_record_to_bow(rec, bow);
		rec = rec->next;
	}
}

void hash_record_to_bow(record* rec, BoWords* bow){
    spec_to_bow(rec->spec, bow);
	bow->specsSum++;
}