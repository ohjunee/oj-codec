#include "Functions.h"

void DC_DPCM(BUFFER* img, DATA* file, double* Quant_blk)
{
	if (file->dc_dpcm_mode == 0)
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
		else if (img->blk_cnt != 0)
		{
			ref_px[0] = img->isFirstBlk_in_row_org ? img->DC_Recon[img->blk_cnt - nWidth_blk] : img->DC_Recon[img->blk_cnt - 1];		// left
			ref_px[1] = img->DC_Recon[img->blk_cnt - nWidth_blk];																	// up
			ref_px[2] = img->isLastBlk_in_row_org ? img->DC_Recon[img->blk_cnt - nWidth_blk - 1] : img->DC_Recon[img->blk_cnt - nWidth_blk + 1];// upper left or upper right

			QuickSort(ref_px, 0, 2);

			p_val = ref_px[1];
		}

		img->DC_Recon[img->blk_cnt] = Quant_blk[0];

		//현재블록의 DC_val = 현재 dc값 - 예측 dc값 
		Quant_blk[0] = Quant_blk[0] - p_val;
	}
}

void Inverse_DC_DPCM(BUFFER* img, DATA* file, double* Quant_blk)
{
	if (file->dc_dpcm_mode == 0)
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
		Quant_blk[0] = img->DC_Recon[img->blk_cnt] = Quant_blk[0] + p_val;
	}
}

void Pixel_DPCM(BUFFER* img, DATA* file, double* PredErr, double* Pixel_DPCM_blk)
{
	if (file->pixel_dpcm_mode == 0)
	{
		double ref_px[3] = { 0 };	// 0: Left, 1:Upper, 2: Upper right
		double p_val = 0;

		uint16 nWidth_blk = img->nBlk_width;
		uint8 p_size = img->p_size;

		for (int row = 0; row < p_size; row++) {
			for (int idx = 0; idx < p_size; idx++) {

				//첫번째 블록
				if (img->blk_cnt == 0)
				{
					// 맨 위쪽 화소열
					if (row == 0)
					{
						p_val = (row == 0 && idx == 0) ? 128 : PredErr[idx - 1];					// left
						Pixel_DPCM_blk[row * p_size + idx] = PredErr[row * p_size + idx] - p_val;
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
						p_val = (row == 0 && idx == 0) ? img->Ref_Pixel2D[img->blk_cnt % (nWidth_blk)-1][0] : PredErr[idx - 1];				// left
						Pixel_DPCM_blk[row * p_size + idx] = PredErr[row * p_size + idx] - p_val;
						continue;
					}

					// 나머지 화소
					else
					{
						ref_px[0] = idx ? PredErr[row * p_size + idx - 1] : img->Ref_Pixel2D[img->blk_cnt % (nWidth_blk)-1][row];					// left
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
						ref_px[0] = idx ? PredErr[row * p_size + idx - 1] : img->Ref_Pixel2D[img->blk_cnt % (nWidth_blk)-1][0];// left
						ref_px[1] = img->Ref_Pixel2D[img->blk_cnt % (nWidth_blk)][idx + p_size];			// up

						//맨 오른쪽 블럭인 경우
						if ((img->blk_cnt + 1) % (nWidth_blk) == 0)
							ref_px[2] = (idx == p_size - 1) ? img->Ref_Pixel2D[img->blk_cnt % (nWidth_blk)][idx + p_size - 1] : img->Ref_Pixel2D[img->blk_cnt % (nWidth_blk)][idx + p_size + 1];
						else
							ref_px[2] = (idx == p_size - 1) ? img->Ref_Pixel2D[img->blk_cnt % (nWidth_blk)+1][p_size] : img->Ref_Pixel2D[img->blk_cnt % (nWidth_blk)][idx + p_size + 1];
					}

					// 나머지
					else
					{
						ref_px[0] = idx ? PredErr[row * p_size + idx - 1] : img->Ref_Pixel2D[img->blk_cnt % (nWidth_blk)-1][row];
						ref_px[1] = PredErr[(row - 1) * p_size + idx];					// up
						ref_px[2] = (idx == p_size - 1) ? PredErr[(row - 1) * p_size - 1 + idx] : PredErr[(row - 1) * p_size + idx + 1];
					}
				}

				QuickSort(ref_px, 0, 2);

				//현재 값 - 중간 값 
				Pixel_DPCM_blk[row * p_size + idx] = PredErr[row * p_size + idx] - ref_px[1];
			}
		}
	}

}

void Inverse_Pixel_DPCM(BUFFER* img, DATA* file, double* Recon_DCT_blk, double* PredErr)
{
	if (file->pixel_dpcm_mode == 0)
	{
		double ref_px[3] = { 0 };	// 0: Left, 1:Upper, 2: Upper right
		double p_val = 0;
		uint8 p_size = img->p_size;

		// DPCM 복호화
		memset(PredErr, 0, sizeof(double) * img->macroSize);

		for (int row = 0; row < p_size; row++) {
			for (int idx = 0; idx < p_size; idx++) {

				//첫번째 블록
				if (img->blk_cnt == 0)
				{
					// 맨 위쪽 화소열
					if (row == 0)
					{
						p_val = (row == 0 && idx == 0) ? 128 : PredErr[idx - 1];					// left
						PredErr[row * p_size + idx] = Recon_DCT_blk[row * p_size + idx] + p_val;
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
						PredErr[row * p_size + idx] = Recon_DCT_blk[row * p_size + idx] + p_val;
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
				PredErr[row * p_size + idx] = Recon_DCT_blk[row * p_size + idx] + ref_px[1];
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