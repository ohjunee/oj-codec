#include "Functions.h"

Node* createFirstNode(Node* head)
{
	Node* newNode = (Node*)malloc(sizeof(Node));	// 새 노드 생성
	newNode->next = head->next;
	head->next = newNode;

	return newNode;
}

void removeLastNode(Node* head)
{
	Node* frontNode = 0;
	Node* curr = head->next;

	while (curr->next != NULL)
	{
		frontNode = curr;
		curr = curr->next;
	}
	frontNode->next = curr->next;

	free(curr->ref_img);
	free(curr);
}