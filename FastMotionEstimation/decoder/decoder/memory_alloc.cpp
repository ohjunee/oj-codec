#include "Functions.h"
#pragma warning(disable: 6011)

// 2차원 메모리 동적할당
double** double_calloc2D(uint8 row, uint8 col)
{
	double** pp = 0;
	pp = (double**)calloc(row, sizeof(double*));

	if (pp == 0)
		return 0;

	*pp = (double*)calloc(row * col, sizeof(double));

	if ((*pp) == 0)
	{
		free(pp);
		return 0;
	}

	for (int j = 1; j < row; j++)
		pp[j] = pp[j - 1] + col;

	return pp;
}

uint8** uint8_calloc2D(uint8 row, uint8 col)
{
	uint8** pp = 0;

	pp = (uint8**)calloc(row, sizeof(uint8*));

	*pp = (uint8*)calloc(row * col, sizeof(uint8));

	for (int j = 1; j < row; j++)
	{
		pp[j] = pp[j - 1] + col;
	}

	return pp;
}

int** int_calloc2D(uint8 row, uint8 col)
{
	int** pp = 0;

	pp = (int**)calloc(row, sizeof(int*));


	if (pp == 0)
		return 0;


	*pp = (int*)calloc(row * col, sizeof(int));

	if ((*pp) == 0)
	{
		free(pp);
		return 0;
	}

	for (int j = 1; j < row; j++)
	{
		pp[j] = pp[j - 1] + col;
	}

	return pp;
}

uint32** uint32_calloc2D(uint8 row, uint8 col)
{
	uint32** pp = 0;

	pp = (uint32**)calloc(row, sizeof(uint32*));

	*pp = (uint32*)calloc(row * col, sizeof(uint32));

	for (int j = 1; j < row; j++)
	{
		pp[j] = pp[j - 1] + col;
	}

	return pp;
}

