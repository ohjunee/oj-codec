#pragma once
#include "structure.h"
#include "input.h"

void		checkFile	(DATA* file, int argc, char* argv[], bool Bool);
void		Open_File	(DATA* file, int argc, char* argv[]);
void		Close_File	(DATA* file);
void		ImageLoad	(BUFFER* img, DATA* file, Node* POC);
void		ImageSave	(BUFFER* img, DATA* file);
void		Padding		(BUFFER* img, DATA* file);

void		encoder		(BUFFER* img, DATA* file, Node* head);
void		Read_YCbCr	(BUFFER* img, DATA* file, Node* POC);
void		Partitioning(BUFFER* img, DATA* file, Node* POC);


/* INTER */
void		checkFirstFrame		(BUFFER* img, uint32& upleft_px);
void		INTER_Prediction		(BUFFER* img, DATA* file, Node* POC, uint32& upleft_px);
void		PSNR					(DATA* file, char* argv[]);
void		removeLastNode		(Node* head);

Node*	createFirstNode		(Node* head);


// 2차원 메모리 동적할당
template <typename T>
T** memory_calloc2D(uint16 row, uint16 col)
{
	T** pp = 0;

	pp = (T**)calloc(row, sizeof(T*));

	*pp = (T*)calloc(row * col, sizeof(T));

	for (int j = 1; j < row; j++)
	{
		pp[j] = pp[j - 1] + col;
	}

	return pp;
}