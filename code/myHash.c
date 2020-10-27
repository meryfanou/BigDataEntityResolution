#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "myHash.h"


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
			// printf("cell: %d\n",i);
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

void hash_add(hashTable* table, char* key, list_node* lNode, int hash){

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
	if((existingRec = search_bucket(tempBuc, key)) != NULL)
		flag = 1;
	
	while(tempBuc->next != NULL && flag == 0){	/// FIND LAST BUCKET
		tempBuc = tempBuc->next;				/// + SEARCH FOR REC IN BUCS
		if((existingRec = search_bucket(tempBuc, key)) != NULL){
			flag = 1;
			break;
		}
	}

	if(flag == 1){
		record_add(existingRec, lNode);
	}
	else{
		if(tempBuc->cur >= table->maxRecs){			/// CHECK IF ITS FULL
			tempBuc->next = bucket_create(table->bucSize);
			tempBuc = tempBuc->next;
		}

		bucket_add(tempBuc, key, lNode);					/// ADD AT BUCKET
	}

}

void hash_print(hashTable* t){
	int i = 0;
	while(i < t->tableSize){
		if(t->myTable[i] != NULL){
			// printf("Tcell: %d\n",i);
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

void bucket_add(bucket* buc, char* key, list_node* p){
	if(buc->rec == NULL){
		buc->rec = record_create(key, p);
		buc->cur++;
		return;
	}

	record* tempRec = buc->rec;					/// FIND LAST ENTRY
	while(tempRec->next != NULL){
		tempRec = tempRec->next;
	}

	tempRec->next = record_create(key, p);				/// ADD RECORD
	buc->cur++;

}

record* search_bucket(bucket* b, char* key){
	record* temp = b->rec;

	while(temp != NULL){
		if(strcmp(temp->key, key) == 0)
			return temp;
		temp = temp->next;
	}

	return NULL;
}

void bucket_print(bucket* b){
	printf("\t>buc:\n");
	record* temp = b->rec;
	while(temp != NULL){
		record_print(temp);
		temp = temp->next;
	}
	if(b->next != NULL){
		bucket_print(b->next);
	}
	// printf("\n");
}

void bucket_destroy(bucket* buc){				/// FREE BUC
	// printf("buc->entries %d\n", buc->cur);
	if(buc->next != NULL)
		bucket_destroy(buc->next);

	record_destroy(buc->rec);
	// printf("elante\n");
	free(buc);

}


record* record_create(char* key, list_node* p){
	record* newRec = malloc(sizeof(record));

	newRec->next = NULL;
	newRec->entries = 0;
	newRec->key = strdup(key);
	newRec->myTree = NULL;
	// printf("APO CREATE\n");
	record_add(newRec, p);
	// !!!!!!!!!!!!!!!
	// newRec->treePtr = tree_add(newRec);
	// !!!!!!!!!!!!!!!

	return newRec;
}

void record_add(record* rec, list_node* p){
	// printf("My match !!, rec key: %s\n", rec->key);
	rec->entries++;

	if(rec->myTree == NULL){
		rec->myTree = RBT_create();
	}

	RBT_add(rec->myTree, p);

}

void record_print(record* rec){
	printf("\t\t>key: %s\n", rec->spec->specID);

}

void record_destroy(record* rec){				/// FREE REC
	// printf("(skata)\n");
	if(rec->next != NULL){
		// printf("???\n");
		record_destroy(rec->next);
	}
	free(rec);

}