#include "Functions.h"

void Inverse_DC_DPCM(BUFFER* img, DATA* file)
{
	if (file->DC_DPCM_MODE == 0)
	{
		double ref_px[3] = { 0 };	// 0: Left, 1:Upper, 2: Upper left
		double p_val = 0;
		uint16 nWidth_blk = img->nBlk_width;

		// 맨 처음 블록의 DC 예측값
		if (img->blk_cnt == 0)
			p_val = 1024;

		//첫번째 블록 행
		else if (img->blk_cnt != 0 && img->row_order == 0)
			p_val = img->DC_Recon[img->blk_cnt - 1];		// left

		//나머지
		else if (img->blk_cnt != 0 && img->row_order != 0)
		{
			ref_px[0] = img->isFirstBlk_in_row_org ? img->DC_Recon[img->blk_cnt - nWidth_blk] : img->DC_Recon[img->blk_cnt - 1];		// left
			ref_px[1] = img->DC_Recon[img->blk_cnt - nWidth_blk];																	// up
			ref_px[2] = img->isLastBlk_in_row_org ? img->DC_Recon[img->blk_cnt - nWidth_blk - 1] : img->DC_Recon[img->blk_cnt - nWidth_blk + 1];// upper left or upper right

			QuickSort(ref_px, 0, 2);

			p_val = ref_px[1];
		}

		//현재블록의 DC_val = 현재 dc값 - 예측 dc값 
		file->ZigZag_arr[0] = img->DC_Recon[img->blk_cnt] = file->ZigZag_arr[0] + p_val;
	}
}

void Inverse_Pixel_DPCM(BUFFER* img, DATA* file, double* PredErr)
{
	if (file->pxDPCM_MODE == 0)
	{
		double ref_px[3] = { 0 };	// 0: Left, 1:Upper, 2: Upper right
		double p_val = 0;
		uint8 p_size = img->p_size;

		// DPCM 복호화
		for (int row = 0; row < p_size; row++) {
			for (int idx = 0; idx < p_size; idx++) {

				//첫번째 블록
				if (img->blk_cnt == 0)
				{
					// 맨 위쪽 화소열
					if (row == 0)
					{
						p_val = (row == 0 && idx == 0) ? 128 : PredErr[idx - 1];					// left
						PredErr[row * p_size + idx] = img->Recon_DCT_blk[row * p_size + idx] + p_val;
						continue;
					}

					// 나머지
					else
					{
						ref_px[0] = idx ? PredErr[row * p_size + idx - 1] : PredErr[(row - 1) * p_size];
						ref_px[1] = PredErr[(row - 1) * p_size + idx];
						ref_px[2] = (idx == p_size - 1) ? PredErr[(row - 1) * p_size + idx - 1] : PredErr[(row - 1) * p_size + idx + 1];
					}
				}

				//첫번째 블록 행
				else if (img->blk_cnt != 0 && img->row_order == 0)
				{
					// 맨 위쪽 화소열
					if (row == 0)
					{
						p_val = (row == 0 && idx == 0) ? img->Ref_Pixel2D[img->blk_cnt % (img->nBlk_width) - 1][0] : PredErr[idx - 1];					// left
						PredErr[row * p_size + idx] = img->Recon_DCT_blk[row * p_size + idx] + p_val;
						continue;
					}

					// 나머지 화소
					else
					{
						ref_px[0] = idx ? PredErr[row * p_size + idx - 1] : img->Ref_Pixel2D[img->blk_cnt % (img->nBlk_width) - 1][row];					// left
						ref_px[1] = PredErr[(row - 1) * p_size + idx];		// up
						ref_px[2] = (row != 0 && idx == p_size - 1) ? PredErr[(row - 1) * p_size - 1 + idx] : PredErr[(row - 1) * p_size + 1 + idx];	// upleft : upRight를 upleft로 대체
					}
				}

				//첫번째 블록열
				else if (img->blk_cnt != 0 && img->isFirstBlk_in_row_org)
				{
					// 맨 위쪽 화소열
					if (row == 0)
					{
						ref_px[0] = idx ? PredErr[idx - 1] : img->Ref_Pixel2D[0][idx + p_size];
						ref_px[1] = img->Ref_Pixel2D[0][idx + p_size];				// upper
						ref_px[2] = (idx == p_size - 1) ? img->Ref_Pixel2D[1][p_size] : img->Ref_Pixel2D[0][idx + p_size + 1];				// upper

					}

					// 나머지
					else
					{
						ref_px[0] = idx ? PredErr[row * p_size + idx - 1] : PredErr[(row - 1) * p_size + idx];
						ref_px[1] = PredErr[(row - 1) * p_size + idx];		// up
						ref_px[2] = (idx == p_size - 1) ? PredErr[(row - 1) * p_size + idx - 1] : PredErr[(row - 1) * p_size + idx + 1];
					}
				}

				//나머지
				else
				{
					// 맨 위쪽 화소열
					if (row == 0)
					{
						ref_px[0] = idx ? PredErr[row * p_size + idx - 1] : img->Ref_Pixel2D[img->blk_cnt % (img->nBlk_width) - 1][0];// left
						ref_px[1] = img->Ref_Pixel2D[img->blk_cnt % (img->nBlk_width)][idx + p_size];			// up

						//맨 오른쪽 블럭인 경우
						if ((img->blk_cnt + 1) % (img->nBlk_width) == 0)
							ref_px[2] = (idx == p_size - 1) ? img->Ref_Pixel2D[img->blk_cnt % (img->nBlk_width)][idx + p_size - 1] : img->Ref_Pixel2D[img->blk_cnt % (img->nBlk_width)][idx + p_size + 1];
						else
							ref_px[2] = (idx == p_size - 1) ? img->Ref_Pixel2D[img->blk_cnt % (img->nBlk_width) + 1][p_size] : img->Ref_Pixel2D[img->blk_cnt % (img->nBlk_width)][idx + p_size + 1];
					}

					// 나머지
					else
					{
						ref_px[0] = idx ? PredErr[row * p_size + idx - 1] : img->Ref_Pixel2D[img->blk_cnt % (img->nBlk_width) - 1][row];
						ref_px[1] = PredErr[(row - 1) * p_size + idx];					// up
						ref_px[2] = (idx == p_size - 1) ? PredErr[(row - 1) * p_size - 1 + idx] : PredErr[(row - 1) * p_size + idx + 1];
					}
				}

				QuickSort(ref_px, 0, 2);

				//현재 값 + 중간 값 
				PredErr[row * p_size + idx] = img->Recon_DCT_blk[row * p_size + idx] + ref_px[1];
			}
		}

		// 예측 에러블록의 맨 우측 화소열, 맨 아래 화소행을 순서대로 저장
		for (uint8 l = 0; l < 2; l++) {
			for (uint8 i = 0; i < p_size; i++)
			{
				int index = (img->blk_cnt) % (img->nBlk_width);
				img->Ref_Pixel2D[index][i + l * p_size] = l ? PredErr[i + (p_size - 1) * p_size] : PredErr[(p_size - 1) + i * p_size];
			}
		}
	}
}

double DCT_Basis_Vector(uint8 k, uint8 n, uint8& p_size)
{
	double N = p_size;
	return (k == 0) ? sqrt((double)1 / N) : sqrt((double)2 / N) * cos((2 * n + 1) * M_PI_2 * k / N);
}

void IDCT(BUFFER* img, DATA* file)
{
	memset(img->Recon_DCT_blk, 0, sizeof(double) * img->macroSize);

	double* i_dct_coeff_H = (double*)calloc(img->macroSize, sizeof(double));

	if (!i_dct_coeff_H)
	{
		printf("memory error");
		exit(0);
	}

	int	index = 0;

	for (int l = 0; l < img->p_size; l++)
	{
		for (int j = 0; j < img->p_size; j++)
		{
			for (int i = 0; i < img->p_size; i++)
			{
				// V = qunatized X = buffer->dct_coeff	X1 = dct_coeff
				i_dct_coeff_H[index] += DCT_Basis_Vector(i, l, img->p_size) * file->ZigZag_arr[j + i * img->p_size]; // X1[index] = (C_T) * V
			}
			index++;
		}
	}
	index = 0;

	for (int k = 0; k < img->p_size; k++)
	{
		for (int j = 0; j < img->p_size; j++)
		{
			for (int i = 0; i < img->p_size; i++)
			{
				//IDCT X = buffer->dct_coeff
				img->Recon_DCT_blk[index] += i_dct_coeff_H[i + k * img->p_size] * DCT_Basis_Vector(i, j, img->p_size); // X[index] = X1 * C
			}
			index++;
		}
	}

	free(i_dct_coeff_H);
}

void Inverse_Q(BUFFER* img, DATA* file)
{
	uint8 QP = 0;

	for (int i = 0; i < img->macroSize; i++)
	{
		QP = !i ? file->QP_DC : file->QP_AC;
		file->ZigZag_arr[i] = file->ZigZag_arr[i] * QP;
	}

	printf("");
}


void Recon_ZigZagScan(BUFFER* img, DATA* file)
{
	memset(file->ZigZag_arr, 0, sizeof(double) * img->macroSize);

	int idx = 0, cur = 0, row = 0, col = 0, cnt = 0, check = 1;

	for (cnt = 0; cnt <= (img->p_size - 1) * 2; cnt++)
	{
		file->ZigZag_arr[row * img->p_size + col] = file->recon_bit[idx];

		if (cnt < img->p_size - 1)
			check == 1 ? col++ : row++;
		else
			check == 1 ? row++ : col++;

		idx++;

		cur = abs(row - col);

		for (int tmp = 0; tmp < cur && idx < img->macroSize; tmp++, idx++)
		{
			file->ZigZag_arr[row * img->p_size + col] = file->recon_bit[idx];

			check == 1 ? col-- : col++;
			check == 1 ? row++ : row--;
		}

		check *= -1;
	}
}

/*
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

*/

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
