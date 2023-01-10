#include "Functions.h"

void swap(double& left, double& right)
{
	double tmp = left;
	left = right;
	right = tmp;
}

void swap(int& left, int& right)
{
	int tmp = left;
	left = right;
	right = tmp;
}

//quicksort를 사용한 이유: 예측모드 수가 많아질 경우 오름차순 정렬하는데 걸리는 시간이 짧음
void QuickSort(double* ref_val, int left, int right)
{
	int L = left;
	int R = right;

	double pivot = ref_val[(L + R) / 2];

	do
	{
		while (ref_val[L] < pivot)
			L++;
		while (ref_val[R] > pivot)
			R--;

		if (L <= R)
		{
			swap(ref_val[L], ref_val[R]);
			L++; R--;
		}

	} while (L <= R);

	//피봇 기준으로

	//왼쪽 정렬: R이 음수가 되면, 왼쪽 배열은 공집합이므로 정렬x
	if (left < R)
		QuickSort(ref_val, left, R);

	//오른쪽 정렬: L이 분할된 배열의 오른쪽 끝에 도달하면 정렬x
	if (right > L)
		QuickSort(ref_val, L, right);
}

void QuickSort(int* ref_val, int left, int right)
{
	int L = left;
	int R = right;

	int pivot = ref_val[(L + R) / 2];

	do
	{
		while (ref_val[L] < pivot)
			L++;
		while (ref_val[R] > pivot)
			R--;

		if (L <= R)
		{
			swap(ref_val[L], ref_val[R]);
			L++; R--;
		}

	} while (L <= R);

	//피봇 기준으로

	//왼쪽 정렬: R이 음수가 되면, 왼쪽 배열은 공집합이므로 정렬x
	if (left < R)
		QuickSort(ref_val, left, R);

	//오른쪽 정렬: L이 분할된 배열의 오른쪽 끝에 도달하면 정렬x
	if (right > L)
		QuickSort(ref_val, L, right);
}

void Recon_ZigZagScan(BUFFER* img, DATA* file)
{
	memset(img->ZigZag_arr, 0, sizeof(double) * PSIZE * PSIZE);

	int idx = 0, cur = 0, row = 0, col = 0, cnt = 0, check = 1;

	for (cnt = 0; cnt <= (PSIZE - 1) * 2; cnt++)
	{
		img->ZigZag_arr[row * PSIZE + col] = file->buffer[idx];

		if (cnt < PSIZE - 1)
			check == 1 ? col++ : row++;
		else
			check == 1 ? row++ : col++;

		idx++;

		cur = abs(row - col);

		for (int tmp = 0; tmp < cur && idx < PSIZE * PSIZE; tmp++, idx++)
		{
			img->ZigZag_arr[row * PSIZE + col] = file->buffer[idx];

			check == 1 ? col-- : col++;
			check == 1 ? row++ : row--;
		}

		check *= -1;
	}
}