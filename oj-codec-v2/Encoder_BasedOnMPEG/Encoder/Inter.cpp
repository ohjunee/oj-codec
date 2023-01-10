#include "Functions.h"

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
	uint16 width = img->width;
	uint8 p_size = img->p_size;

	int mv[2] = { 0 }; // mv[0] = x, mv[1] = y

	double* DCT_blk = (double*)calloc(img->macroSize, sizeof(double));
	double* Quant_blk = (double*)calloc(img->macroSize, sizeof(double));
	double* PredErr_blk = (double*)calloc(img->macroSize, sizeof(double));

	if (PredErr_blk == 0)
	{
		printf("memory error");
		return exit(0);
	}

	file->mv_x = 0;
	file->mv_y = 0;

	img->cnt = 0;
	img->cost = 0;
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

	/* Motion Compensation -> intra 예측블록 - 원본블록 = Error image */
	for (int j = 0; j < p_size; j++) {			// j -> x방향
		for (int i = 0; i < p_size; i++) {		// i -> y방향
			PredErr_blk[j + i * p_size] = (double)(img->org[upleft_px + j + i * width] - file->buffer[img->UpLeftPx_ref + (file->mv_x + j) + (file->mv_y + i) * (width + 2 * p_size)]);
		}
	}

	/* DCT -> Q -> dc dpcm */
	DCT(PredErr_blk, DCT_blk, p_size);					// PredErr_blk	-> DCT_blk
	Quantization(file, DCT_blk, Quant_blk, p_size, true);		// DCT_blk		-> Quant_blk	(forward)
	DC_DPCM(img, file, Quant_blk);

	/* zig zag -> entropy coding */
	ZigZagScan(file, Quant_blk, p_size);						//Quant_blk			-> file->ZigZag_blk 

	// Entropy
	Send_INTER_INFO(file);	// Intra 예측정보를 디코더로 전송하기 위한 비트스트림 파일 생성

	memset(PredErr_blk, 0, sizeof(double) * img->macroSize);

	/* inverse dc dpcm -> inverse Q -> iDCT */
	Inverse_DC_DPCM(img, file, Quant_blk);
	Quantization(file, DCT_blk, Quant_blk, p_size, false);	// Quant_blk	-> DCT_blk		(inverse)
	IDCT(DCT_blk, PredErr_blk, p_size);				// DCT_blk		-> PredErr_blk (recon)

	/* reconstruction: error image + prediction image*/
	for (int j = 0; j < p_size; j++) {
		for (int i = 0; i < p_size; i++)
		{
			int recon_px = file->buffer[img->UpLeftPx_ref + (file->mv_x + j) + (file->mv_y + i) * (width + 2 * p_size)] + (int)round(PredErr_blk[j + i * p_size]);

			// 클리핑
			if (recon_px > 255)
				img->Recon[upleft_px + j + i * width] = 255;
			else if (recon_px < 0)
				img->Recon[upleft_px + j + i * width] = 0;
			else
				img->Recon[upleft_px + j + i * width] = recon_px;
		}
	}

	img->UpLeftPx_ref += p_size;

	// 지역변수에 할당한 동적메모리 해제
	free(Quant_blk);
	free(DCT_blk);
	free(PredErr_blk);
}