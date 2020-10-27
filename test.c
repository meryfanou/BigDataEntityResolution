#include <stdio.h>
#include "mySpec.h"
#include "myMatches.h"

int main(void){
	printf("Hello !!\n");

	// Create Matches List
	matchesInfo* testMatchesList = matchesInfoInit();

	// Create Specs List
	//specList* testSpecsList = specListInit();


	// TEST SPEC INIT
	mySpec* testSpec1 = specInit("ebay.com//324");
	mySpec* testSpec2 = specInit("ebay.com//400");
	mySpec* testSpec3 = specInit("ebay.com//700");


		// Add spec-match
		myMatches* keep = matchesAdd(testMatchesList, testSpec1);
		myMatches* keep2 = matchesAdd(testMatchesList, testSpec2);
		myMatches* keep3 = matchesAdd(testMatchesList, testSpec3);

		// Update Spec's matches
		testSpec1->matches = keep;
		testSpec2->matches = keep2;
		testSpec3->matches = keep3;

	// Add Spec to specList
	specAdd(testSpecsList, testSpec1);
	specAdd(testSpecsList, testSpec2);
	specAdd(testSpecsList, testSpec3);

	// Prints
	printList(testSpecsList);
	printMatchesList(testMatchesList);

	// Merge Matches 1,3
	mergeMatches(testMatchesList, testSpec1->matches, testSpec3->matches);
	printMatchesList(testMatchesList);

	// Free Mem
	deleteList(testSpecsList);

	deleteInfo(testMatchesList);


}