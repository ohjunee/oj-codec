#include "Buffer.h"
#include "input.h"


void I_Quantization(Data* file, uint8 p_size)
{
	int Quant_Table[2][64] =
	{
		{
			10, 14, 20, 24, 14, 20, 24, 27,
			20, 24, 27, 30, 24, 27, 30, 34,
			0,  0,  0,  0,  0,  0,  0,  0,
			0,  0,  0,  0,  0,  0,  0,  0,
			0,  0,  0,  0,  0,  0,  0,  0,
			0,  0,  0,  0,  0,  0,  0,  0,
			0,  0,  0,  0,  0,  0,  0,  0,
			0,  0,  0,  0,  0,  0,  0,  0,
		},

		{
			16, 11, 10, 16, 24,  40,  51,  61,
			12, 12, 14, 19, 26,  58,  60,  55,
			14, 13, 16, 24, 40,  57,  69,  56,
			14, 17, 22, 29, 51,  87,  80,  62,
			18, 22, 37, 56, 68,  109, 103, 77,
			24, 36, 55, 64, 81,  104, 113, 92,
			49, 64, 78, 87, 103, 121, 120, 101,
			72, 92, 95, 98, 112, 100, 103, 99
		}

	};

	// 역양자화, 
	for (int i = 0; i < p_size*p_size; i++)
	{
		file->Recon_DCT_blk[i] = file->Recon_DCT_blk[i] * Quant_Table[p_size >> 3][i];
	}
}


void IDCT(Data* file, uint8 p_size, int dct_mode)
{
	if (dct_mode)
	{
		double* i_dct_coeff_H = (double*)calloc(p_size*p_size, sizeof(double));
		int		index = 0;

		for (int l = 0; l < p_size; l++)
		{
			for (int j = 0; j < p_size; j++)
			{
				for (int i = 0; i < p_size; i++)
				{
					// V = qunatized X = buffer->dct_coeff	X1 = Recon_DCT_blk
					i_dct_coeff_H[index] += DCT_Basis_Vector(i, l, p_size) * file->Recon_DCT_blk[j + i * p_size]; // X1[index] = (C_T) * V
				}
				index++;
			}
		}

		index = 0;
		memset(file->Recon_DCT_blk, 0, sizeof(double) * p_size * p_size);

		for (int k = 0; k < p_size; k++)
		{
			for (int j = 0; j < p_size; j++)
			{
				for (int i = 0; i < p_size; i++)
				{
					//IDCT X = buffer->Recon_DCT_blk
					file->Recon_DCT_blk[index] += i_dct_coeff_H[i + k * p_size] * DCT_Basis_Vector(i, j, p_size); // X[index] = X1 * C
				}
				index++;
			}
		}

		free(i_dct_coeff_H);
	}
}



double DCT_Basis_Vector(uint8 k, uint8 n, uint8 p_size)
{
	double N = p_size;
	return (k == 0) ? sqrt((double)1 / N) : sqrt((double)2 / N) * cos((2 * n + 1) * M_PI_2 * k / N);
}