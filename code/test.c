#include <stdio.h>
#include "mySpec.h"
#include "myMatches.h"

int main(void){
	printf("Hello !!\n");

	// Create Matches List
	matchesInfo* testMatchesList = matchesInfoInit();

	// Create Specs List
	specList* testSpecsList = specListInit();


	// TEST SPEC INIT
	mySpec* testSpec1 = specInit("ebay.com//324");

		// Add spec-match
		myMatches* keep = matchesAdd(testMatchesList, testSpec1);

		// Update Spec's matches
		testSpec1->matches = keep;


	// Add Spec to specList
	specAdd(testSpecsList, testSpec1);

	// Prints
	printList(testSpecsList);
	printMatchesList(testMatchesList);

	// Free Mem
	deleteList(testSpecsList);

	deleteInfo(testMatchesList);


}