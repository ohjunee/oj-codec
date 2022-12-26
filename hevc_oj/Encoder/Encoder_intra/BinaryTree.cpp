#include "Buffer.h"
#include "Node.h"


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
#if TEST_HUFFMAN == 1
void write_AbsValue(Data* file, int codeword[], int length)
{
	int buffer = 0;

	for (int i = 0; i < length; i++)
	{
		buffer = codeword[i];
		file->output = file->output << 1 | buffer;
		file->length++;

		if (file->length == 8)
		{
			fwrite(&file->output, sizeof(uint8), 1, file->fp_out);
			file->output = 0;
			file->length = 0;
		}
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


void Tree_traversal(Node* node, Data* file, uint8 symbol, int codeword[], int length)
{
	/* 현재 노드의 자식노드 탐색 */

	//왼쪽 자식노드가 존재하면
	if (node->left != NULL)
	{
		codeword[length] = 0;
		Tree_traversal(node->left, file, symbol, codeword, length + 1);
	}

	//오른쪽 자식노드가 존재하면
	if (node->right != NULL)
	{
		codeword[length] = 1;
		Tree_traversal(node->right, file, symbol, codeword, length + 1);
	}

	// 현재 노드의 자식노드가 NULL인 경우 꼬리노드이므로, 해당 심볼값을 출력하고 부호어출력
	else
	{
		if (symbol == node->symbol)
		{
			write_AbsValue(file, codeword, length);
		}
	}
}

bool isEndNode(Node* node)
{
	bool result = (node->left == NULL) && (node->right == NULL);

	return result;
}

void Extract_symbol(Data* file, uint8 p_size, uint8& index)
{
	uint8 absErr;

	for (int i = 0; i < p_size*p_size; i++)
	{
		absErr = file->input_sequence[i] = fabs(file->PredErr[i]);

		//첫번째 심볼값은 0번째 인덱스에 저장
		if (index == 0)
		{
			file->symbol[0] = absErr;
			file->freq[0]++;

			index++;
		}

		//두번째 부터는 앞에 값과 비교
		else
		{
			for (int i = 0; i < index; i++)
			{
				if (file->symbol[i] == absErr)		//현재 입력으로 들어오는 심볼값이랑 이전에 있던 심볼값이랑 같은 경우 비교를 중지
				{
					file->freq[i]++;
					break;
				}

				if (i == index - 1)						//마지막원소까지 비교했는데도 같은 값이 없으면 현재 인덱스에 심볼값 저장
				{
					file->symbol[index++] = absErr;
				}
			}
		}
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