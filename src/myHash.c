#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/myHash.h"


hashTable* hash_create(int HashSize, int BucSize){
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
	
	// printf("MaxRecs: %d\n", newTable->maxRecs);
	// printf("BucSize: %d\n", BucSize);
	// printf("tableSize: %d\n", HashSize);
	// printf("SizeOf bucket*: %lu\n", sizeof(bucket*));
	// printf("SizeOf record: %lu\n", sizeof(record));
	
	return newTable;
}

void hash_destroy(hashTable* table){
	int i = 0;
	while(i < table->tableSize){
		if(table->myTable[i] != NULL){
			bucket_destroy(table->myTable[i]);
		}
		i++;
	}

	free(table->myTable);
	free(table);

}


int hash1(char* key){
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

	int cell = hash % (table->tableSize);		/// FIND HASH TABLE CELL
	if(cell < 0)
		cell = 0;
	// printf("cell: %d, tSize: %d, maxRecs/buc: %d\n", cell, table->tableSize, table->maxRecs);

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
	}
	else{
		if(tempBuc->cur >= table->maxRecs){			/// CHECK IF ITS FULL
			tempBuc->next = bucket_create(table->bucSize);
			tempBuc = tempBuc->next;
		}

		bucket_add(tempBuc, newSpec);					/// ADD AT BUCKET
	}

}

void hash_print(hashTable* t){
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

record* search_bucket(bucket* b, char* key){
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

void bucket_print(bucket* b){
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

void bucket_destroy(bucket* buc){				/// FREE BUC
	if(buc->next != NULL)
		bucket_destroy(buc->next);

	record_destroy(buc->rec);
	free(buc);

}


record* record_create(mySpec* spec){
	record* newRec = malloc(sizeof(record));

	newRec->spec = spec;
	newRec->next = NULL;

	return newRec;
}

void record_print(record* rec){
	printf("\t\t>key: %s\n", rec->spec->specID);

	// printSpec(rec->spec);
}

void record_destroy(record* rec){				/// FREE REC
	if(rec->next != NULL){
		record_destroy(rec->next);
	}

	deleteSpec(rec->spec);
	free(rec);
}

mySpec* findRecord_byKey(hashTable* table, char* key){
	int cell = hash1(key) % (table->tableSize);		/// FIND HASH TABLE CELL
	if(cell < 0)
		cell = 0;
	// printf("cell: %d, tSize: %d, maxRecs/buc: %d\n", cell, table->tableSize, table->maxRecs);

	if(table == NULL){
		printf("edw\n");
		return NULL;
	}

	if(table->myTable[cell] == NULL){
		printf("edw edw\n");
		return NULL;
	}


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