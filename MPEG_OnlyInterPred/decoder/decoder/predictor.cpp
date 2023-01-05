#include "Functions.h"

void getPredBlock(BUFFER* img, uint8* Pred_b, uint8 mode, uint32 upleft_px)
{
	uint8 i, j, idx;

	switch (mode) {

		/*수직 예측*/
	case 0:
	{
		//첫번째 행에 존재하는 블록들은 수직모드 예측시 상단에 인접픽셀이 없으므로 gray scale의 픽셀범위의 중간값인 128로 예측
		if (img->row_order == 0)
			memset(Pred_b, 128, sizeof(uint8) * PSIZE * PSIZE);		//			Pred[j + i * PSIZE] = 128;

		else
		{
			//복원이미지 버퍼의 기준화소 위치에서 수평방향으로 1, ... , (PSIZE + 1)만큼 이동
			for (j = 0; j < PSIZE; j++) {
				for (i = 0; i < PSIZE; i++) {
					Pred_b[j + i * PSIZE] = img->Recon[upleft_px + j - WIDTH];
				}
			}
		}
		break;
	}

	/*수평 예측*/
	case 1:
	{
		if (img->isFirstBlk_in_row)
			memset(Pred_b, 128, sizeof(uint8) * PSIZE * PSIZE);		//			Pred[j + i * PSIZE] = 128;

		else
		{
			//복원이미지 버퍼의 기준화소 위치에서 수직방향으로 0, ... , (PSIZE - 1)만큼 이동
			for (j = 0; j < PSIZE; j++) {
				for (i = 0; i < PSIZE; i++) {
					Pred_b[j + i * PSIZE] = img->Recon[i * WIDTH + upleft_px - 1];
				}
			}
		}

		break;
	}
	/*DC 예측*/
	case 2:
	{
		uint32 dc_row = 0, dc_col = 0;

		uint8 mean;

		// 첫번째 블록을 제외한 첫째 줄
		if (img->row_order == 0)
		{
			for (idx = 0; idx < PSIZE; idx++)
				dc_row += img->Recon[upleft_px + idx * WIDTH - 1];

			mean = round((dc_row + dc_col) / PSIZE);
			memset(Pred_b, mean, sizeof(uint8) * PSIZE * PSIZE);
		}

		// 첫번째 블록을 제외한 첫째 열
		else if (img->row_order != 0 && img->isFirstBlk_in_row)
		{
			for (idx = 0; idx < PSIZE; idx++)
				dc_col += img->Recon[upleft_px - WIDTH + idx];

			mean = round((dc_row + dc_col) / PSIZE);
			memset(Pred_b, mean, sizeof(uint8) * PSIZE * PSIZE);
		}

		else {
			//복원이미지 버퍼의 기준화소 위치에서 수직/수평방향으로 0, ... , (PSIZE -1) 위치에 있는 화소값을 모두 더함
			for (idx = 0; idx < PSIZE; idx++)
			{
				dc_row += img->Recon[upleft_px - 1 + idx * WIDTH];
				dc_col += img->Recon[upleft_px - WIDTH + idx];
			}

			mean = round((dc_row + dc_col) / (2 * PSIZE));
			memset(Pred_b, mean, sizeof(uint8) * PSIZE * PSIZE);
		}

		break;
	}

	/*대각선 예측*/
	case 3:
	{
		uint8 diff;

		// 첫번째 블록을 제외한 첫째 줄
		if (img->row_order == 0)
		{
			for (j = 0; j < PSIZE; j++) {
				for (i = 0; i < PSIZE; i++)
				{
					// 화소값의 좌표값(i,j)를 이용하여 대각 방향 예측
					diff = abs(i - j);
					Pred_b[j + i * PSIZE] = (i <= j) ? 128 : img->Recon[upleft_px - 1 + (diff - 1) * WIDTH];
				}
			}
		}

		// 첫번째 블록을 제외한 첫째 열
		else if (img->row_order != 0 && img->isFirstBlk_in_row)
		{
			for (j = 0; j < PSIZE; j++) {
				for (i = 0; i < PSIZE; i++)
				{
					// 화소값의 좌표값(i,j)를 이용하여 대각 방향 예측
					diff = abs(i - j);
					Pred_b[j + i * PSIZE] = (i < j) ? img->Recon[upleft_px - 1 - WIDTH + diff] : 128;
				}
			}
		}

		else
		{
			for (j = 0; j < PSIZE; j++) {
				for (i = 0; i < PSIZE; i++)
				{
					// 화소값의 좌표값(i,j)를 이용하여 대각 방향 예측
					diff = abs(i - j);
					Pred_b[j + i * PSIZE] = (i <= j) ? img->Recon[upleft_px - 1 - WIDTH + diff] : img->Recon[upleft_px - 1 + (diff - 1) * WIDTH];
				}
			}
		}

		break;
	}
	}
}

//첫번째 블록을 제외한 나머지 블록들에 대해 MPM_flag 예측
void Find_MPM(BUFFER* img, DATA* file, uint32 width)
{
	int ref_Pmode[3] = { 0 };	// 0: Left, 1:Upper, 2: Upper left

	//첫번째 블록 행
	if (img->row_order == 0)
		ref_Pmode[1] = file->PM_Arr[img->blk_cnt - 1];					// left

	//첫번째 블록 열
	else if (img->isFirstBlk_in_row)
		ref_Pmode[1] = file->PM_Arr[img->blk_cnt - width / PSIZE];		// up

	//나머지
	else
	{
		ref_Pmode[0] = file->PM_Arr[img->blk_cnt - 1];					// left
		ref_Pmode[1] = file->PM_Arr[img->blk_cnt - width / PSIZE];		// up
		ref_Pmode[2] = file->PM_Arr[img->blk_cnt - width / PSIZE - 1];	// upleft

		QuickSort(ref_Pmode, 0, 2);
	}

	file->PredMode = ref_Pmode[1];
}