#include "input.h"

#if TEST_HUFFMAN == 1
typedef struct Node {

	uint8	freq;
	uint8	symbol;

	Node *left, *right;

}Node;

void QuickSort	(Node** nodeArr, int left, int right);
void swap		(Node **left, Node **right);

Node*	create_HuffmanTree	(Node** nodeArr, uint8& n_symbol);
Node**	createNextFloor		(Node* newNode, Node** nodeArr, uint8 arrLen);

void initialize			(Data* data, Node** tail, int n_symbol);
void Tree_traversal		(Data* file, Node* node);

void Tree_memory_alloc1D(Data* file);
void Tree_memory_free1D	(Data* file);

void free_Tree	(Node* node);
bool isEndNode	(Node* node);

void Read_Sign	(Data* file, uint8 p_size);
void Read_AbsVal(Data* file, uint8 p_size);
void Read_Symbol(Data* file, uint8 p_size);

void Read_Huffman_Table		(Data* file, uint8 p_size);
void Huffman_decoding		(Data* file, uint8 p_size);
void Fixed_Length_Decoding	(Data* file, uint8 p_size, int type);


#else
void Read_Sign					(Data* file, uint8 p_size);
#endif

void Exponetial_Golomb_Decoding	(Data* file, uint8 p_size, int type);
