#include "Buffer.h"
#include "Node.h"
#include "input.h"


#if TEST_HUFFMAN == 1

void initialize(Data* file, Node** tail, int n_symbol)
{
	for (int i = 0; i < n_symbol; i++)
	{
		Node* node = (Node*)malloc(sizeof(Node));

		node->freq = file->freq[i];				//발생횟수
		node->symbol = file->symbol[i];				//심볼값

		node->left = NULL;
		node->right = NULL;

		tail[i] = node;
	}
}

Node* create_HuffmanTree(Node** nodeArr, uint8& n_symbol)
{
	Node* result;

	n_symbol--;

	/* 노드 정렬 */
	QuickSort(nodeArr, 0, n_symbol);								// 노드를 심볼값의 발생확률에 대해 오름차순으로 정렬

	Node** newLayer;												// 이진트리에서 새로 만들어진 노드 층

	Node* newNode = (Node*)malloc(sizeof(Node));					// 합쳐진 노드 메모리 할당

	newNode->left = nodeArr[0];
	newNode->right = nodeArr[1];

	newNode->freq = nodeArr[0]->freq + nodeArr[1]->freq;			// 합쳐진 노드의 빈도 수
	newNode->symbol = 0;

	/* 이진트리에서 새로 만들어진 노드 층 */
	newLayer = createNextFloor(newNode, nodeArr, n_symbol);
	result = newNode;

	if (n_symbol > 1)
	{
		/* 노드 합치기 */
		result = create_HuffmanTree(newLayer, n_symbol);
	}

	return result;
}

Node** createNextFloor(Node* newNode, Node** nodeArr, uint8 arrLen)
{
	Node** newNodeArr = (Node**)malloc(sizeof(Node*) * arrLen);

	newNodeArr[0] = newNode;

	for (int i = 1; i < arrLen; i++)
	{
		newNodeArr[i] = nodeArr[i + 1];
	}
	return newNodeArr;
}

void Huffman_decoding(Data* file, uint8 p_size)
{
	uint8	n_symbol = file->n_symbol;
	uint8	start_index = 0;

	int*	codeword = (int*)calloc(sizeof(int), p_size*p_size);

	Node*	head = NULL;											// 머리노드 생성, 자식노드는 모두 NULL로 초기화
	Node**	tail = (Node**)malloc(sizeof(Node*)*n_symbol);

	initialize(file, tail, n_symbol);

	head = create_HuffmanTree(tail, n_symbol);						// 머리 노드를 허프만 트리와 연결

//	fread(&file->output, sizeof(uint8), 1, file->fp_in);			// 재귀함수 바깥에서 fread

	while (file->index < p_size*p_size)
	{
		Tree_traversal(file, head);					// 전위 순회하여 왼쪽 꼬리노드부터 탐색
	}

	free_Tree(head);												// 전위 순회방식으로 메모리 순차 해제
	free(codeword);
}

void Tree_traversal(Data* file, Node* node)
{
	//끝노드가 아니라면 비트단위로 값을 가져온다.
	if (!isEndNode(node))
	{
		file->direction = file->output << (file->length % 8) & (uint8)128 ? 1 : 0;
	}

	//비트값이 0이고 왼쪽 자식노드가 존재하면
	if (!file->direction && node->left != NULL)
	{
		file->length++;

		if (file->length % 8 == 0)
		{
			fread(&file->output, sizeof(uint8), 1, file->fp_in);
			file->length = 0;
		}

		Tree_traversal(file, node->left);
	}

	//비트값이 1이고 오른쪽 자식노드가 존재하면
	if (file->direction && node->right != NULL)
	{
		file->length++;

		if (file->length % 8 == 0)
		{
			fread(&file->output, sizeof(uint8), 1, file->fp_in);
			file->length = 0;
		}
		Tree_traversal(file, node->right);
	}

	// 끝노드인 경우 양자화 변환계수 버퍼에 끝노드의 심볼값을 저장
	if (isEndNode(node))
	{
		file->Recon_DCT_blk[file->index++] *= node->symbol;
		file->direction = 0;
	}
}

bool isEndNode(Node* node)
{
	bool result = (node->left == NULL) && (node->right == NULL);

	return result;
}

void Tree_memory_alloc1D(Data* file)
{
	file->symbol = (uint8*)calloc(sizeof(uint8), file->n_symbol);
	file->freq = (uint8*)calloc(sizeof(uint8), file->n_symbol);
}

void Tree_memory_free1D(Data* file)
{
	free(file->symbol);
	free(file->freq);
}

void free_Tree(Node* node)
{
	if (node->left != NULL)
	{
		free_Tree(node->left);
		node->left = NULL;
	}

	if (node->right != NULL)
	{
		free_Tree(node->right);
		node->right = NULL;
	}

	if (isEndNode(node))// 꼬리노드인 경우, 해당 심볼값을 출력하고 부호어출력
	{
		free(node);
	}
}

void QuickSort(Node** nodeArr, int left, int right)
{
	int L = left;
	int R = right;

	Node* pivot = nodeArr[(L + R) / 2];

	do
	{
		while (nodeArr[L]->freq < pivot->freq)
			L++;
		while (nodeArr[R]->freq > pivot->freq)
			R--;

		if (L <= R)
		{
			swap(&nodeArr[L], &nodeArr[R]);
			L++; R--;
		}

	} while (L <= R);

	//피봇 기준으로

	//왼쪽 정렬: R이 음수가 되면, 왼쪽 배열은 공집합이므로 정렬x
	if (left < R)
		QuickSort(nodeArr, left, R);

	//오른쪽 정렬: L이 분할된 배열의 오른쪽 끝에 도달하면 정렬x
	if (right > L)
		QuickSort(nodeArr, L, right);
}

void swap(Node **left, Node **right)
{
	Node* tmp = *left;
	*left = *right;
	*right = tmp;
}
#endif