#ifndef MBH_H
#define MBH_H

#include "boWords.h"

typedef struct MBH MBH;
typedef struct MBHNode MBHNode;

// Min Binary Heap
struct MBH{
	MBHNode*    root;
	MBHNode*    max;		// Node with the maximum key ~ always a leaf
    int         numofNodes;
    int         maxNodes;
};

// Heap's node
struct MBHNode{
    char*       word;
	float		key;
	MBHNode*    left;
	MBHNode*    right;
	MBHNode*    parent;
};

// ~~~~~~~~~~~~~~~~~ MBHNode ~~~~~~~~~~~~~~~~~

void swap(MBHNode*, MBHNode*);			// Exchange two nodes' info, without moving the actual nodes
MBHNode* add_mbh_node(MBHNode*, char*, float);
void delete_mbh_node(MBH*,MBHNode*,int);	// Delete a node from the heap
											// 3rd argument: if 1 ~> heapify after deletion | if 0 ~> do nothing

// ~~~~~~~~~~~~~~~~~~~ MBH ~~~~~~~~~~~~~~~~~~~

void mbh_init(MBH**, int);              // Initialize heap, passing the maximum number of nodes in the heap
int mbh_insert(MBH*, char*, float);		// Create a new node in heap ~ return 0 if successfull
void mbh_delete(MBH**);
void mbh_heapify(MBHNode*);	    		// Heapify heap starting from a node and going upwards till the root
void mbh_inverse_heapify(MBHNode*);		// Heapify heap starting from a node till a leaf
char* mbh_extract_root(MBH*);	        // Delete heap's root and re-heapify the heap
MBHNode* mbh_find_max(MBHNode*);		// Find node with the maximum key in heap


#endif