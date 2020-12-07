#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/boWords.h"
#include "../include/myHash.h"


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

void bow_destroy(BoWords* table){	// DESTROY HASH / FREE MEM
	int i = 0;
	while(i < table->tableSize){
		if(table->myTable[i] != NULL){
			bow_bucket_destroy(table, table->myTable[i]);
			table->myTable[i] = NULL;
		}
		i++;
	}

	free(table->myTable);
	free(table);

}

void bow_add(BoWords* table, char* word, mySpec* text, int hash){

	if(word == NULL)
		return;

	int cell = hash % (table->tableSize);		/// FIND HASH TABLE CELL
	if(cell < 0)
		cell = 0;

	if(table->myTable[cell] == NULL){			/// CASE OF FIRST BUCKET
		table->myTable[cell] = bow_bucket_create(table->bucSize);
	}

	Bucket* tempBuc = table->myTable[cell];
	int flag = 0;		// 0 ~ new key / 1 ~ existing key
	Record* existingRec = NULL;
	if((existingRec = bow_search_bucket(tempBuc, word)) != NULL)
		flag = 1;
	
	while(tempBuc->next != NULL && flag == 0){	/// FIND LAST BUCKET
		tempBuc = tempBuc->next;				/// + SEARCH FOR REC IN BUCS
		if((existingRec = bow_search_bucket(tempBuc, word)) != NULL){
			flag = 1;
			break;
		}
	}

	if(flag == 1){
		// Update word's number of appearances in current text
		bow_record_update(existingRec, text);
	}
	else{
		if(tempBuc->cur >= table->maxRecs){			/// CHECK IF ITS FULL
			tempBuc->next = bow_bucket_create(table->bucSize);
			tempBuc = tempBuc->next;
		}

		bow_bucket_add(tempBuc, word, text);					/// ADD AT BUCKET
		table->entries++;
	}

}

void bow_print(BoWords* t){			// testing funct - prints hash
	printf("HASH_SIZE: %d\n", t->tableSize);
	int i = 0;
	while(i < t->tableSize){
		if(t->myTable[i] != NULL){
			printf("cell [%d]:\n", i);
			bow_bucket_print(t->myTable[i]);
		}
		i++;
	}
	printf("\n");
}


Bucket* bow_bucket_create(int size){				/// INIT BUCKET
	Bucket* newBucket = malloc(sizeof(bucket));
	newBucket->next = NULL;
	newBucket->rec = NULL;
	newBucket->cur = 0;

	return newBucket;
}

void bow_bucket_add(Bucket* buc, char* word, mySpec* text){
	if(buc->rec == NULL){
		buc->rec = bow_record_create(word, text);
		buc->cur++;
		return;
	}

	Record* tempRec = buc->rec;					/// FIND LAST ENTRY
	while(tempRec->next != NULL){
		tempRec = tempRec->next;
	}

	tempRec->next = bow_record_create(word, text);				/// ADD RECORD
	buc->cur++;

}

Record* bow_search_bucket(Bucket* b, char* key){	// search bucket for target key
	if(b == NULL)
		return NULL;
	Record* temp = b->rec;

	while(temp != NULL){
		if(strcmp(temp->word, key) == 0)
			return temp;
		temp = temp->next;
	}

	return NULL;
}

void bow_bucket_print(Bucket* b){					// testing funct - prints bucket
	printf("\t>buc: (%d)\n", b->cur);
	Record* temp = b->rec;
	while(temp != NULL){
		bow_record_print(temp);
		temp = temp->next;
	}
	if(b->next != NULL){
		bow_bucket_print(b->next);
	}
}

void bow_bucket_destroy(BoWords* hash, Bucket* buc){		/// FREE BUC
	if(buc->next != NULL){
		bow_bucket_destroy(hash, buc->next);
		buc->next = NULL;
	}

	if(buc->rec != NULL){
		bow_record_destroy(hash, buc->rec);
		buc->rec = NULL;
	}

	free(buc);
	buc = NULL;
}


Record* bow_record_create(char* word, mySpec* text){
	Record* newRec = malloc(sizeof(Record));

	newRec->next = NULL;
	newRec->word = strdup(word);
	newRec->texts = malloc(sizeof(TextInfo));
	newRec->texts[0].text = text;
	newRec->texts[0].numofInstances = 1;
	newRec->numofTexts = 1;

	return newRec;
}

void bow_record_print(Record* rec){
	printf("\t\t>word: %s\n", rec->word);

	for(int i=0; i<(rec->numofTexts); i++){
		printf("\t\t\t>appears %d times in %s\n",rec->texts[i].numofInstances,rec->texts[i].text->specID);
	}
}

void bow_record_destroy(BoWords* boWords, Record* rec){				/// FREE REC
	if(rec->next != NULL){
		bow_record_destroy(boWords, rec->next);
		rec->next = NULL;
	}

	free(rec->word);
	free(rec->texts);
	if(boWords != NULL)
		boWords->entries--;

	free(rec);
	rec = NULL;
}

void bow_record_update(Record* rec, mySpec* text){
	int	i = 0;

	for(; i<(rec->numofTexts); i++){
		// If the word has already appeared in current text
		if(rec->texts[i].text == text){
			(rec->texts[i].numofInstances)++;
			break;
		}
	}

	// If the word appears in current text for the first time
	if(i == rec->numofTexts){
		i = rec->numofTexts;

		(rec->numofTexts)++;
		rec->texts = realloc(rec->texts, (rec->numofTexts)*sizeof(TextInfo));
		rec->texts[i].text = text;
		rec->texts[i].numofInstances = 1;
	}
}