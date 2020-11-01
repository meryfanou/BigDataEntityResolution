// tets hash

#include <stdio.h>
#include "../include/myHash.h"
#include "../include/mySpec.h"

int main(void){
	mySpec* spec = specInit("ebay.com//2020", NULL, 0);

	hashTable* hashT = hash_create(6, 100);
	hash_add(hashT, spec, hash1(spec->specID));

	hash_print(hashT);
	hash_destroy(hashT);
}