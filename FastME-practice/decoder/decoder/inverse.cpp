#include "Functions.h"

void Inverse_DC_DPCM(BUFFER* img, DATA* file)
{
	double ref_px[3] = { 0 };	// 0: Left, 1:Upper, 2: Upper left
	double p_val = 0;
	int nWidth_blk = WIDTH / PSIZE;

	// 맨 처음 블록의 DC 예측값
	if (img->blk_cnt == 0)
		p_val = 1024;

	//첫번째 블록 행
	else if (img->blk_cnt != 0 && img->row_order == 0)
		p_val = img->DC_Recon[img->blk_cnt - 1];		// left

	//나머지
	else if (img->blk_cnt != 0 && img->row_order != 0)
	{
		ref_px[0] = img->isFirstBlk_in_row ? img->DC_Recon[img->blk_cnt - nWidth_blk] : img->DC_Recon[img->blk_cnt - 1];		// left
		ref_px[1] = img->DC_Recon[img->blk_cnt - nWidth_blk];																	// up
		ref_px[2] = img->isLastBlk_in_row ? img->DC_Recon[img->blk_cnt - nWidth_blk - 1] : img->DC_Recon[img->blk_cnt - nWidth_blk + 1];// upper left or upper right

		QuickSort(ref_px, 0, 2);

		p_val = ref_px[1];
	}

	//현재블록의 DC_val = 현재 dc값 - 예측 dc값 
	img->ZigZag_arr[0] = img->DC_Recon[img->blk_cnt] = img->ZigZag_arr[0] + p_val;
}

double DCT_Basis_Vector(uint8 k, uint8 n)
{
	double N = PSIZE;
	return (k == 0) ? sqrt((double)1 / N) : sqrt((double)2 / N) * cos((2 * n + 1) * M_PI_2 * k / N);
}

void IDCT(BUFFER* img, DATA* file)
{
	double* i_dct_coeff_H = (double*)calloc(PSIZE * PSIZE, sizeof(double));

	int	index = 0;

	for (int l = 0; l < PSIZE; l++)
	{
		for (int j = 0; j < PSIZE; j++)
		{
			for (int i = 0; i < PSIZE; i++)
			{
				// V = qunatized X = buffer->dct_coeff	X1 = dct_coeff
				i_dct_coeff_H[index] += DCT_Basis_Vector(i, l) * img->ZigZag_arr[j + i * PSIZE]; // X1[index] = (C_T) * V
			}
			index++;
		}
	}
	index = 0;

	for (int k = 0; k < PSIZE; k++)
	{
		for (int j = 0; j < PSIZE; j++)
		{
			for (int i = 0; i < PSIZE; i++)
			{
				//IDCT X = buffer->dct_coeff
				img->Recon_DCT_blk[index] += i_dct_coeff_H[i + k * PSIZE] * DCT_Basis_Vector(i, j); // X[index] = X1 * C
			}
			index++;
		}
	}

	free(i_dct_coeff_H);
}

void Inverse_Q(BUFFER* img, DATA* file)
{
	int Quant_Table[64] =
	{
			1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1
	};


	for (int i = 0; i < PSIZE * PSIZE; i++)
		img->ZigZag_arr[i] = img->ZigZag_arr[i] * Quant_Table[i];

	printf("");
}


/*

		16, 11, 10, 16, 24,  40,  51,  61,
		12, 12, 14, 19, 26,  58,  60,  55,
		14, 13, 16, 24, 40,  57,  69,  56,
		14, 17, 22, 29, 51,  87,  80,  62,
		18, 22, 37, 56, 68,  109, 103, 77,
		24, 36, 55, 64, 81,  104, 113, 92,
		49, 64, 78, 87, 103, 121, 120, 101,
		72, 92, 95, 98, 112, 100, 103, 99
*/


/*
			1,  16, 16, 16, 16, 16, 16, 16,
			16, 16, 16, 16, 16, 16, 16, 16,
			16, 16, 16, 16, 16, 16, 16, 16,
			16, 16, 16, 16, 16, 16, 16, 16,
			16, 16, 16, 16, 16, 16, 16, 16,
			16, 16, 16, 16, 16, 16, 16, 16,
			16, 16, 16, 16, 16, 16, 16, 16,
			16, 16, 16, 16, 16, 16, 16, 16

*/

/*

			1, 8, 8, 8, 8, 8, 8, 8,
			8, 8, 8, 8, 8, 8, 8, 8,
			8, 8, 8, 8, 8, 8, 8, 8,
			8, 8, 8, 8, 8, 8, 8, 8,
			8, 8, 8, 8, 8, 8, 8, 8,
			8, 8, 8, 8, 8, 8, 8, 8,
			8, 8, 8, 8, 8, 8, 8, 8,
			8, 8, 8, 8, 8, 8, 8, 8
*/

/*

			1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1
*/