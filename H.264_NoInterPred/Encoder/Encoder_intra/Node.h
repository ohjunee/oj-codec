#include "input.h"

typedef struct Reference_Picture {

	Ref *left, *right;

	uint8* Ref_Image;

}Ref;

typedef struct Node {

	uint8	freq;
	int		symbol;

	Node *left, *right;

}Node;

bool isEndNode					(Node* node);
void initialize					(Data* file, Node** tail, int n_symbol);
void Exponetial_Golomb_coding	(Data* file, uint8& n_symbol, uint8 p_size, int type);


#if TEST_HUFFMAN == 1
void Tree_memory_free1D	(Data* file);
void Tree_memory_alloc1D(Data* file, uint8 size);

void free_Tree	(Node* node);

void QuickSort	(Node** nodeArr, int left, int right);
void swap		(Node **left, Node **right);

Node* create_HuffmanTree(Node** nodeArr, uint8& n_symbol);
Node**createNextFloor	(Node* newNode, Node** nodeArr, uint8 arrLen);

void Tree_traversal		(Node* node, Data* file, uint8 symbol, int codeword[], int index);

void Extract_symbol		(Data* file, uint8 p_size, uint8& index);
void Huffman_coding		(Data* file, uint8 n_symbol, uint8 p_size);

void write_AbsValue		(Data* file, int codeword[], int index);
void Send_Huffman_Table	(Data* file, uint8 n_symbol, uint8 p_size);
void Send_Symbol		(Data* file, uint8 n_symbol, uint8 p_size);
void Fixed_Length_Coding(Data* file, uint8 n_symbol, uint8 p_size, int type);
#endif
