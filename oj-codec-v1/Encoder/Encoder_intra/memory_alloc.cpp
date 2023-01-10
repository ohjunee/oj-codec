#include "Buffer.h"
#include "Node.h"
#include "input.h"

void sturct_memory_check(BUFFER* img, Data* file, uint32 height, uint32 width)
{
	// 구조체 변수 초기화
	img->extra_row = height % PSIZE ? PSIZE - (height % PSIZE) : 0;
	img->extra_col = width  % PSIZE ? PSIZE - (width % PSIZE) : 0;

	img->nRow = ((height + img->extra_row) / PSIZE);
	img->nCol = ((width + img->extra_col) / PSIZE);

	img->nWidth = PSIZE * img->nCol;
	img->index_mode = 0;

	img->UpLeft_px_org = 0;
	img->UpLeft_px_recon = 0;

	file->maxCount = img->nRow * img->nCol;

	memset(file->PredMode_4x4, 0, sizeof(uint8) * 4);
	memset(file->DCT_Mode_4x4, 0, sizeof(uint8) * 4);


	//구조체 변수 동적 메모리 할당
	img->org_b				=	(uint8*)	malloc((height + img->extra_row)		*(width + img->extra_col)	 * sizeof(uint8));
	img->recon_padding		=	(uint8*)	malloc((height + img->extra_row + 1)	*(width + img->extra_col + 1) * sizeof(uint8));
	img->Recon_DCT_blk		=	(double*)	calloc(PSIZE*PSIZE, sizeof(double));

	file->Quant_coeff_4x4	=	double_calloc2D(4, (uint8)pow(PSIZE / 2, 2));

	// 할당받은 메모리 초기화
	memset(img->cost,			0, sizeof(uint32)	* 2);
	memset(img->org_b,			PADDING_VAL, sizeof(uint8)*(height + img->extra_row)		*(width + img->extra_col));
	memset(img->recon_padding,	PADDING_VAL, sizeof(uint8)*(height + img->extra_row + 1)	*(width + img->extra_col + 1));

}


void struct_memory_free(BUFFER* img, Data* file)
{
	free(img->org_b);
	free(img->Recon_DCT_blk);
	free(img->recon_padding);

	free(file->Quant_coeff_4x4[0]);
	free(file->Quant_coeff_4x4);
}

// 2차원 메모리 동적할당
double** double_calloc2D(uint8 row, uint8 col)
{
	double** pp = 0;
	pp = (double**)calloc(row, sizeof(double*));

	*pp = (double*)calloc(row*col, sizeof(double));

	for (int j = 1; j < row; j++)
	{
		pp[j] = pp[j - 1] + col;
	}

	return pp;
}

uint8**	uint8_calloc2D(uint8 row, uint8 col)
{
	uint8** pp = 0;

	pp = (uint8**)calloc(row, sizeof(uint8*));

	*pp = (uint8*)calloc(row*col, sizeof(uint8));

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

	*pp = (uint32*)calloc(row*col, sizeof(uint32));

	for (int j = 1; j < row; j++)
	{
		pp[j] = pp[j - 1] + col;
	}

	return pp;
}

void memory_free2D(double** Quant_blk, uint8** Recon)
{
	free(Quant_blk[0]);
	free(Quant_blk);

	free(Recon[0]);
	free(Recon);
}

void Local_memory_free2D (double** Pred_Err, double** Dct_blk, uint8** Pred, uint32** cost)
{
	free(Pred_Err[0]);
	free(Pred_Err);

	free(Dct_blk[0]);
	free(Dct_blk);

	free(Pred[0]);
	free(Pred);

	free(cost[0]);
	free(cost);
}

// 3차원 메모리 동적할당
double*** double_calloc3D(uint8 DCT_mode, uint8 row, uint8 col)
{
	double*** ptr3d = 0;

	ptr3d = (double***)calloc(DCT_mode, sizeof(double**));

	for (int i = 0; i < DCT_mode; i++)
	{
		ptr3d[i] = double_calloc2D(row, col);
	}
	return ptr3d;
}

uint8*** uint8_calloc3D(uint8 DCT_mode, uint8 row, uint8 col)
{
	uint8*** ptr3d = 0;

	ptr3d = (uint8***)calloc(DCT_mode, sizeof(uint8**));

	for (int i = 0; i < DCT_mode; i++)
	{
		ptr3d[i] = uint8_calloc2D(row, col);
	}
	return ptr3d;
}

void Local_memory_free3D(double*** p1, uint8*** p2, uint8 DCT_mode)
{
	for (int i = 0; i < DCT_mode; i++)
	{
		memory_free2D(p1[i], p2[i]);
	}

	free(p1);
	free(p2);
}
#if TEST_HUFFMAN == 1

// 1차원 메모리 동적할당
void Tree_memory_alloc1D(Data* file, uint8 size)
{
	file->input_sequence	= (uint8*)calloc(sizeof(uint8), size);
	file->symbol			= (uint8*)calloc(sizeof(uint8), size);
	file->freq				= (uint8*)calloc(sizeof(uint8), size);
}

void Tree_memory_free1D(Data* file)
{
	free(file->input_sequence);
	free(file->symbol);
	free(file->freq);
}

//이진트리 메모리 해제
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

#endif