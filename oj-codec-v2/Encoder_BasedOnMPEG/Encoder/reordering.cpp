#include "Functions.h"

void ZigZagScan(DATA* file, double* Quant_blk, uint8& p_size)
{
	memset(file->ZigZag_arr, 0, sizeof(double) * p_size * p_size);

	int idx = 0, cur = 0, row = 0, col = 0, cnt = 0, check = 1;

	//정방향 스캔
	for (cnt = 0; cnt <= (p_size - 1) * 2; cnt++)
	{
		file->ZigZag_arr[idx] = Quant_blk[row * p_size + col];

		if (cnt < p_size - 1)
			check == 1 ? col++ : row++;
		else
			check == 1 ? row++ : col++;

		idx++;

		cur = abs(row - col);

		for (int tmp = 0; tmp < cur && idx < p_size * p_size; tmp++, idx++)
		{
			file->ZigZag_arr[idx] = Quant_blk[row * p_size + col];		// reorder된 양자화 변환계수값들 -> file->ZigZag_arr에 저장

			check == 1 ? col-- : col++;
			check == 1 ? row++ : row--;
		}

		check *= -1;
	}
}


