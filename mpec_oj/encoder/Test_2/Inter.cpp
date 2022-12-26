#include "Functions.h"

void checkFirstFrame(BUFFER* img, uint32& upleft_px)
{
	uint8 p_size = img->p_size;

	/* no prediction */
	for (int j = 0; j < p_size; j++) {
		for (int i = 0; i < p_size; i++)
		{
			img->Recon[upleft_px + j + i * img->width] = img->org[upleft_px + j + i * img->width];
		}
	}

}

void ME(BUFFER* img, DATA* file, Node* POC, uint32& upleft_px, int* mv)
{
	uint8 p_size = (!file->comp) ? PSIZE : PSIZE / 2;

	/* Motion Estimation: best motion vector 결정*/
	for (int j = 0; j < p_size; j++) {			// j -> x방향
		for (int i = 0; i < p_size; i++) {		// i -> y방향
		/* cost 계산 : sum(원본 - 참조) */
			img->cost += abs(img->org[upleft_px + j + i * img->width] - POC->Y[img->UpLeftPx_ref + (mv[0] + j) + (mv[1] + i) * (img->width + 2 * p_size)]);
		}
		/* k행까지의 연산이 이전에 구했던 최소 SAD보다 커질때 계산 종료*/
		if (img->cnt > 0 && img->cost > img->min_val)
			break;
	}

	if (img->cnt == 0)
		img->min_val = img->cost;

	/* 최적의 MV 구하기 */
	if (img->cost < img->min_val)
	{
		POC->mv_arr[0][img->blk_cnt] = file->mv_x = mv[0];
		POC->mv_arr[1][img->blk_cnt] = file->mv_y = mv[1];
		img->min_val = img->cost;
	}
	img->cost = 0;
	img->cnt++;
}

void INTER_Prediction(BUFFER* img, DATA* file, Node* POC, uint32& upleft_px)
{
	uint16	width	= img->width;
	uint8	p_size	= img->p_size;

	int mv[2] = { 0 }; // mv[0] = x, mv[1] = y
	
	file->mv_x = 0;
	file->mv_y = 0;

	img->cnt	 = 0;
	img->cost	 = 0;
	img->min_val = 0;

	// Y
	if (!file->comp)
	{
		uint8 max_X = 1, max_Y = 1;

		/* Motion Estimation: Full Search Argorithm, spiral scan with PDA */
		for (int MV_distance = 0; MV_distance < 2 * p_size + 1; MV_distance++, max_X++, max_Y++)
		{
			if (!MV_distance)
				ME(img, file, POC, upleft_px, mv);

			for (int j = (MV_distance == p_size * 2) ? 1 : 0; j < max_X; j++)
			{
				max_X % 2 ? mv[0]++ : mv[0]--;
				ME(img, file, POC, upleft_px, mv);
			}

			for (int i = 0; i < ((MV_distance == p_size * 2) ? 0 : max_Y); i++)
			{
				max_Y % 2 ? mv[1]++ : mv[1]--;
				ME(img, file, POC, upleft_px, mv);
			}
		}
	}

	// CbCr
	else
	{
		file->mv_x = round(POC->mv_arr[0][img->blk_cnt] / 2);
		file->mv_y = round(POC->mv_arr[1][img->blk_cnt] / 2);
	}

	/* component 값에 따라 달라지는 주소값 */
	switch (file->comp % 3)
	{
	case 0:
		file->buffer = POC->Y;
		break;
	case 1:
		file->buffer = POC->Cb;
		break;
	case 2:
		file->buffer = POC->Cr;
		break;
	}

	/* reconstruction: error image + prediction image*/
	for (int j = 0; j < p_size; j++) {
		for (int i = 0; i < p_size; i++)
		{
			img->Recon[upleft_px + j + i * width] = file->buffer[img->UpLeftPx_ref + (file->mv_x + j) + (file->mv_y + i) * (width + 2 * p_size)];
		}
	}

	img->UpLeftPx_ref += p_size;
}