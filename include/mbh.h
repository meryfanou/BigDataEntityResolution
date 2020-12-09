#ifndef MBH_H
#define MBH_H

#include "boWords.h"

typedef struct MBH MBH;
typedef struct MBHNode MBHNode;

// Min Binary Heap
struct MBH{
	MBHNode*    root;
	MBHNode*    max;
    int         numofNodes;
    int         maxNodes;
};

// Heap's node
struct MBHNode{
    char*       word;
	int		    key;
	MBHNode*    left;
	MBHNode*    right;
	MBHNode*    parent;
};

void swap(MBHNode*, MBHNode*);			// Exchange two nodes' info
MBHNode* add_mbh_node(MBHNode*, char*, int);
void delete_mbh_node(MBH*,MBHNode*,int);

/*------------------------------------------------------------------*/

void mbh_init(MBH**, int);              // Initialize heap, passing the maximum number of nodes in the heap
int mbh_insert(MBH*, char*, int);
void mbh_delete(MBH**);
void mbh_heapify(MBHNode*);	    		// Heapify heap starting from a node and going upwards till the root
void mbh_inverse_heapify(MBHNode*);		// Heapify heap starting from a node till a leaf
char* mbh_extract_root(MBH*);	        // Delete heap's root and heapify the heap
MBHNode* mbh_find_max(MBHNode*);		// Find node with the minimum key in heap


#endif