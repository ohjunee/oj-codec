#include "Functions.h"

double DCT_Basis_Vector(uint8 k, uint8 n, uint8& p_size)
{
	double N = p_size;
	return (k == 0) ? sqrt((double)1 / N) : sqrt((double)2 / N) * cos((2 * n + 1) * M_PI_2 * k / N);
}

void DCT(double* input_blk, double* DCT_buf, uint8& p_size)
{
	double* dct_coeff_H = (double*)calloc(p_size * p_size, sizeof(double));
	//	memset(DCT_buf, 0, sizeof(double) * PSIZE * PSIZE);

	int		index = 0;

	//행방향 1차원 DCT
	for (int k = 0; k < p_size; k++)
	{
		for (int j = 0; j < p_size; j++)
		{
			for (int i = 0; i < p_size; i++)
			{
				dct_coeff_H[index] += DCT_Basis_Vector(k, i, p_size) * input_blk[j + i * p_size]; // V1 = C * X
			}
			index++;
		}
	}

	index = 0;

	//열방향 1차원 DCT
	for (int l = 0; l < p_size; l++)
	{
		for (int j = 0; j < p_size; j++)
		{
			for (int i = 0; i < p_size; i++)
			{
				//DCT 계수
				DCT_buf[index] += dct_coeff_H[i + l * p_size] * DCT_Basis_Vector(j, i, p_size); // V1 = X * C_T
			}
			index++;
		}
	}

	free(dct_coeff_H);
}

void IDCT(double* DCT_buf, double* result, uint8& p_size)
{
	//결과 버퍼값 초기화
	memset(result, 0, sizeof(double) * p_size * p_size);

	double* i_dct_coeff_H = (double*)calloc(p_size * p_size, sizeof(double));

	int	index = 0;

	for (int l = 0; l < p_size; l++)
	{
		for (int j = 0; j < p_size; j++)
		{
			for (int i = 0; i < p_size; i++)
			{
				// V = qunatized X = buffer->dct_coeff	X1 = dct_coeff
				i_dct_coeff_H[index] += DCT_Basis_Vector(i, l, p_size) * DCT_buf[j + i * p_size]; // X1[index] = (C_T) * V
			}
			index++;
		}
	}
	index = 0;

	for (int k = 0; k < p_size; k++)
	{
		for (int j = 0; j < p_size; j++)
		{
			for (int i = 0; i < p_size; i++)
			{
				//IDCT X = buffer->dct_coeff
				result[index] += i_dct_coeff_H[i + k * p_size] * DCT_Basis_Vector(i, j, p_size); // X[index] = X1 * C
			}
			index++;
		}
	}

	free(i_dct_coeff_H);
}

void Quantization(DATA* file, double* DCT_buf, double* Quant_buf, uint8& p_size, bool type)
{
	uint8 QP = 0;

	if (type) // 양자화
	{
		for (int i = 0; i < p_size * p_size; i++)
		{
			QP = !i ? file->QP_dc : file->QP_ac;
			Quant_buf[i] = (DCT_buf[i] / QP == -0.5) ? 0 : round(DCT_buf[i] / QP);
		}
	}
	else     // 역양자화
	{
		for (int i = 0; i < p_size * p_size; i++)
		{
			QP = !i ? file->QP_dc : file->QP_ac;
			DCT_buf[i] = Quant_buf[i] * QP;
		}
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