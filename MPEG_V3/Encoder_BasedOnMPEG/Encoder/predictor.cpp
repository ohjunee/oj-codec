#include "Functions.h"

void getPredBlock(BUFFER* img, uint8* Pred_b, uint8& mode, uint32& upleft_px)
{
	uint8 i, j, idx;

	switch (mode) {

		/*수직 예측*/
	case 0:
	{
		//첫번째 행에 존재하는 블록들은 수직모드 예측시 상단에 인접픽셀이 없으므로 gray scale의 픽셀범위의 중간값인 128로 예측
		if (img->row_order == 0)
			memset(Pred_b, 128, sizeof(uint8) * img->p_size * img->p_size);		//			Pred[j + i * p_size] = 128;

		else
		{
			//복원이미지 버퍼의 기준화소 위치에서 수평방향으로 1, ... , (p_size + 1)만큼 이동
			for (j = 0; j < img->p_size; j++) {
				for (i = 0; i < img->p_size; i++) {
					Pred_b[j + i * img->p_size] = img->Recon[upleft_px + j - img->width];
				}
			}
		}
		break;
	}

	/*수평 예측*/
	case 1:
	{
		if (img->isFirstBlk_in_row_org)
			memset(Pred_b, 128, sizeof(uint8) * img->p_size * img->p_size);		//			Pred[j + i * p_size] = 128;

		else
		{
			//복원이미지 버퍼의 기준화소 위치에서 수직방향으로 0, ... , (p_size - 1)만큼 이동
			for (j = 0; j < img->p_size; j++) {
				for (i = 0; i < img->p_size; i++) {
					Pred_b[j + i * img->p_size] = img->Recon[i * img->width + upleft_px - 1];
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
			for (idx = 0; idx < img->p_size; idx++)
				dc_row += img->Recon[upleft_px + idx * img->width - 1];

			mean = round((dc_row + dc_col) / img->p_size);
			memset(Pred_b, mean, sizeof(uint8) * img->p_size * img->p_size);
		}

		// 첫번째 블록을 제외한 첫째 열
		else if (img->row_order != 0 && img->isFirstBlk_in_row_org)
		{
			for (idx = 0; idx < img->p_size; idx++)
				dc_col += img->Recon[upleft_px - img->width + idx];

			mean = round((dc_row + dc_col) / img->p_size);
			memset(Pred_b, mean, sizeof(uint8) * img->p_size * img->p_size);
		}

		else {
			//복원이미지 버퍼의 기준화소 위치에서 수직/수평방향으로 0, ... , (p_size -1) 위치에 있는 화소값을 모두 더함
			for (idx = 0; idx < img->p_size; idx++)
			{
				dc_row += img->Recon[upleft_px - 1 + idx * img->width];
				dc_col += img->Recon[upleft_px - img->width + idx];
			}

			mean = round((dc_row + dc_col) / (2 * img->p_size));
			memset(Pred_b, mean, sizeof(uint8) * img->p_size * img->p_size);
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
			for (j = 0; j < img->p_size; j++) {
				for (i = 0; i < img->p_size; i++)
				{
					// 화소값의 좌표값(i,j)를 이용하여 대각 방향 예측
					diff = abs(i - j);
					Pred_b[j + i * img->p_size] = (i <= j) ? 128 : img->Recon[upleft_px - 1 + (diff - 1) * img->width];
				}
			}
		}

		// 첫번째 블록을 제외한 첫째 열
		else if (img->row_order != 0 && img->isFirstBlk_in_row_org)
		{
			for (j = 0; j < img->p_size; j++) {
				for (i = 0; i < img->p_size; i++)
				{
					// 화소값의 좌표값(i,j)를 이용하여 대각 방향 예측
					diff = abs(i - j);
					Pred_b[j + i * img->p_size] = (i < j) ? img->Recon[upleft_px - 1 - img->width + diff] : 128;
				}
			}
		}

		else
		{
			for (j = 0; j < img->p_size; j++) {
				for (i = 0; i < img->p_size; i++)
				{
					// 화소값의 좌표값(i,j)를 이용하여 대각 방향 예측
					diff = abs(i - j);
					Pred_b[j + i * img->p_size] = (i <= j) ? img->Recon[upleft_px - 1 - img->width + diff] : img->Recon[upleft_px - 1 + (diff - 1) * img->width];
				}
			}
		}

		break;
	}
	}
}