#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../include/mbh.h"

// Exchange info between two nodes of the heap, without changing their pointers
void swap(MBHNode* node1, MBHNode* node2){
	char*   temp_w;
	float     temp_k;

	temp_w = strdup(node1->word);
	temp_k = node1->key;

	free(node1->word);
	node1->word = strdup(node2->word);
	node1->key = node2->key;
	
	free(node2->word);
	node2->word = strdup(temp_w);
	node2->key = temp_k;

	free(temp_w);
}

MBHNode* add_mbh_node(MBHNode* heapNode, char* word, float key){
	float	diff = key - heapNode->key;

	MBHNode	*current;
	// If current heapNode has the same key as the new node we are adding, new node will be added as heapNode's left child
	// New node will inherit heapNode's left subtree
	if(diff == 0.0){
		MBHNode* temp;

		current = (MBHNode*)malloc(sizeof(MBHNode));
		current->word = strdup(word);
		current->key = key;
		current->right = NULL;
		current->parent = heapNode;
		temp = heapNode->left;
		heapNode->left = current;
		current->left = temp;
		if(temp != NULL)
			temp->parent = current;
	}
	// Check left subtree
	// If there is none, add new node as heapNode's left child
	else if(diff < 0.0)
	{
		if(heapNode->left != NULL)
			return add_mbh_node(heapNode->left,word,key);

		heapNode->left = (MBHNode*)malloc(sizeof(MBHNode));
		current = heapNode->left;
		current->word = strdup(word);
		current->key = key;
		current->left = NULL;
		current->right = NULL;
		current->parent = heapNode;
	}
	// Check right subtree
	// If there is none, add new node as heapNode's right child
	else
	{
		if(heapNode->right != NULL)
			return add_mbh_node(heapNode->right,word,key);

		heapNode->right = (MBHNode*)malloc(sizeof(MBHNode));
		current = heapNode->right;
		current->word = strdup(word);
		current->key = key;
		current->left = NULL;
		current->right = NULL;
		current->parent = heapNode;
	}

	// Heapify starting from new node's parent
	mbh_heapify(heapNode);

	// Return new node
	return current;
}

void delete_mbh_node(MBH* heap, MBHNode* heapNode, int heapify)
{
	if(heap != NULL && heapNode != NULL)
	{
		int			isLeft = 0;
		MBHNode*	parent = heap->max->parent;

		if(parent == NULL){
			if(heapNode != heap->max){
				printf("!%d %p %p %p\n",heap->numofNodes,heapNode,heap->max,heap->root);
				return;
			}

			free(heapNode->word);
			free(heapNode);
			heap->root = NULL;
			heap->max = NULL;
			(heap->numofNodes)--;
			return;
		}

		if(parent->left == heap->max)
			isLeft = 1;

		swap(heapNode, heap->max);
		free(heap->max->word);
		free(heap->max);

		if(isLeft)
			parent->left = NULL;
		else
			parent->right = NULL;

		if(heapify){
			mbh_inverse_heapify(heapNode);
			heap->max = mbh_find_max(heap->root);
		}

		(heap->numofNodes)--;
	}
}

/*----------------------------------------------------------------------------------------------*/

void mbh_init(MBH** heap, int maxNodes)
{
	if(*heap == NULL)
	{
		*heap = (MBH*)malloc(sizeof(MBH));
		(*heap)->root = NULL;
		(*heap)->max = NULL;
        (*heap)->numofNodes = 0;
        (*heap)->maxNodes = maxNodes;
	}
}

int mbh_insert(MBH* heap, char* word, float key)
{
	if(heap == NULL)
		return -1;

    if(heap->numofNodes == heap->maxNodes){
        if(key <= heap->root->key){
            return 1;
		}
        else{
            char*	word = mbh_extract_root(heap);
			if(word != NULL)
				free(word);
		}
    }

	// If heap is empty
	if(heap->root == NULL)
	{
		heap->root = (MBHNode*)malloc(sizeof(MBHNode));
		heap->root->word = strdup(word);
		heap->root->key = key;
		heap->root->left = NULL;
		heap->root->right = NULL;
		heap->root->parent = NULL;

		// Heap's only element is also the max
		heap->max = heap->root;
	}
	else
	{
		add_mbh_node(heap->root,word,key);
		heap->max = mbh_find_max(heap->root);
	}

    (heap->numofNodes)++;

    return 0;
}

void mbh_delete(MBH** heap)
{
	while((*heap)->numofNodes > 0)
		delete_mbh_node(*heap, (*heap)->root, 0);

	free(*heap);
	*heap = NULL;
}

// Heapify heap starting from a node and going upwards till the root
void mbh_heapify(MBHNode* heapNode)
{
	// If current node has a left child with smaller key, swap them
	if(heapNode->left != NULL && heapNode->key > heapNode->left->key)
		swap(heapNode,heapNode->left);
	// If current node has a right child with smaller key, swap them
	else if(heapNode->right != NULL && heapNode->key > heapNode->right->key)
		swap(heapNode,heapNode->right);

	// If current node has a parent, check if it is heapified too
	if(heapNode->parent != NULL)
		mbh_heapify(heapNode->parent);
}

// Heapify heap starting from a node till a leaf
void mbh_inverse_heapify(MBHNode *heapNode)
{
	// If current node has a left child with smaller key, swap them and call recursively for the left child
	if(heapNode->left != NULL && heapNode->key > heapNode->left->key)
	{
		swap(heapNode,heapNode->left);
		mbh_inverse_heapify(heapNode->left);
	}
	// If current node has a right child with smaller key, swap them and call recursively for the right child
	else if(heapNode->right != NULL && heapNode->key > heapNode->right->key)
	{
		swap(heapNode,heapNode->right);
		mbh_inverse_heapify(heapNode->right);
	}
}

// Delete heap's root and heapify the heap
char* mbh_extract_root(MBH *heap)
{
	if(heap->root != NULL)
	{
        char*   word = strdup(heap->root->word);

        // Max is the last node to be extracted as root
		if(heap->root == heap->max)
		{
//			if(heap->numofNodes == 20)
//				printf("!3\n");
			delete_mbh_node(heap,heap->root,0);
//			if(heap->root == NULL)
//				printf("!2\n");
			return word;
		}

//		// Element with maximum key will be the new root
//		swap(heap->root,heap->max);
//		// Max's old node will be deleted
		delete_mbh_node(heap,heap->root,1);
//		if(heap->root == NULL)
//			printf("!1\n");
//		// Find suitable place for max element
//		mbh_inverse_heapify(heap->root);
//		// Update heap's max pointer
//		heap->max = mbh_find_max(heap->root);

		return word;
	}

	return NULL;
}

// Find node with the minimum key in heap
MBHNode* mbh_find_max(MBHNode* heapNode)
{
	MBHNode* max = heapNode;
	MBHNode *max_l, *max_r;

	// Get max in left subtree and compare it with max kept so far
	if(heapNode->left != NULL)
	{
		max_l = mbh_find_max(heapNode->left);
		if(max_l->key >= max->key)
			max = max_l;
	}
	// Get max in right subtree and compare it with max kept so far
	if(heapNode->right != NULL)
	{
		max_r = mbh_find_max(heapNode->right);
		if(max_r->key >= max->key)
			max = max_r;
	}

	return max;
}