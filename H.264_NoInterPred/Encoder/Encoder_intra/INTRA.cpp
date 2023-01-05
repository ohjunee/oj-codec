#include "Buffer.h"
#include "Node.h"
#include "input.h"

void INTRA_Prediction(BUFFER* img, Data* file, uint8 p_size, uint32 ul_recon, uint32 ul_org)
{
	file->split_flag = (p_size == PSIZE) ? 0 : 1;

	double***	Quant_blk = 0;
	uint8***	Recon = 0;

	double**	Pred_Err = 0;
	double**	Dct_blk = 0;

	uint8**		Pred = 0;
	uint32**	cost = 0;

	uint8 mode = 0;
	uint8 PredMode = 0;
	uint8 Best_DCT_Mode = 0;

	int	DCT_SKIP[PRED_MODE] = { 0 };

	uint32 min_val;

	Quant_blk = double_calloc3D(DCT_MODE, PRED_MODE, p_size*p_size);
	Recon = uint8_calloc3D(DCT_MODE, PRED_MODE, p_size*p_size);

	Pred_Err = double_calloc2D(PRED_MODE, p_size*p_size);
	Dct_blk = double_calloc2D(PRED_MODE, p_size*p_size);

	Pred = uint8_calloc2D(PRED_MODE, p_size*p_size);
	cost = uint32_calloc2D(2, PRED_MODE);

	/* 최적의 예측모드 판단 */
	while (mode < 4)
	{
		cost[file->split_flag][mode] = 0;
		int DCT_cost[DCT_MODE] = { 0 };

		getPredBlock(img, Pred[mode], p_size, mode, ul_recon);
		getPredErrBlock(img, Pred_Err[mode], Pred[mode], p_size, ul_org); //잔차블록: Pred_Err , 예측블록: Pred , 원본 블록 좌상단 좌표: ul_org

		for (int dct_mode = 0; dct_mode < DCT_MODE; dct_mode++)
		{
			DCT(img, p_size, Pred_Err[mode], Dct_blk[mode], dct_mode);						// dct_mode 0: PredErr = DCT_blk
			Quantization(img, p_size, Dct_blk[mode], Quant_blk[dct_mode][mode], FORWARD);   // DCT_blk -> Quant_blk

			Quantization(img, p_size, Dct_blk[mode], Quant_blk[dct_mode][mode], INVERSE);   // Quant_blk -> DCT_blk
			IDCT(img, p_size, Dct_blk[mode], dct_mode);										// DCT_blk -> img->Recon_DCT_blk

			for (int j = 0; j < p_size; j++) {
				for (int i = 0; i < p_size; i++)
				{
					/* 예측블록 + 복원된 예측에러블록의 합 = 복원블록 */
					int recon_px = Pred[mode][j + i * p_size] + (int)round(img->Recon_DCT_blk[j + i * p_size]);

					// 클리핑
					if (recon_px > 255)
					{
						Recon[dct_mode][mode][j + i * p_size] = 255;
					}

					else if (recon_px < 0)
					{
						Recon[dct_mode][mode][j + i * p_size] = 0;
					}
					else
					{
						Recon[dct_mode][mode][j + i * p_size] = recon_px;
					}

					/* cost 계산 : sum(원본 - 복원) */
					DCT_cost[dct_mode] += abs(img->org_b[ul_org + j + i * img->nWidth] - Recon[dct_mode][mode][j + i * p_size]);
				}
			}
		}
		// DCT cost가 최소가 되는 값을 해당 예측모드의 SAD로 정함; index 0 : DCT FALSE, index 1: DCT TRUE
		cost[file->split_flag][mode] = DCT_cost[0] < DCT_cost[1] ? DCT_cost[0] : DCT_cost[1];

		if (mode == 0)
		{
			min_val = cost[file->split_flag][0];
		}

		// DCT skip: 0 false, 1 true 
		DCT_SKIP[mode] = DCT_cost[0] < DCT_cost[1] ? 0 : 1;

		/* 최적의 예측모드 구하기 : cost 판단 */

		if (cost[file->split_flag][mode] < min_val)		// if (cost[file->split_flag][mode] < cost[file->split_flag][0])
		{
			PredMode = mode;
			DCT_SKIP[0] = DCT_SKIP[mode];

			min_val = cost[file->split_flag][mode];
		}
		mode++;
	}

	Best_DCT_Mode = DCT_SKIP[0];

	/* 8x8상태라면 분할 */
	if (file->split_flag == 0)
	{
		img->cost[file->split_flag] = min_val;		//	img->cost[file->split_flag] = cost[file->split_flag][0];

		Partitioning(img, file, 2, 2, p_size / 2);

		img->index_mode = 0;			//partitioning이 끝나면, 4x4분할의 최적의 예측모드를 저장할 시작 인덱스값을 0로 초기화

		/* INTRA: 분할정보, 예측모드정보, DCT_SKIP, 변환계수 정보 전송 */
		file->split_flag = img->cost[1] < img->cost[0] ? 1 : 0;

		if (!(file->split_flag))
			Write_ReconImageBuffer(img, Recon[Best_DCT_Mode][PredMode], p_size, ul_recon);

		if(file->frameCount > 0)
			INTER_Prediction(img, file, p_size, ul_recon, ul_org);

#if TEST == 0
		Send_INTRA(file, Quant_blk[Best_DCT_Mode][PredMode], PredMode, Best_DCT_Mode);
#else
		Send_PRED_INFO(file, file->split_flag, 2);					// 분할 정보 전달 

		for (int i = 0; i < (file->split_flag ? 4 : 1); i++)
		{
			Send_PRED_INFO(file, file->split_flag ? file->PredMode_4x4[i] : PredMode, PRED_MODE);

			Send_PRED_INFO(file, file->split_flag ? file->DCT_Mode_4x4[i] : Best_DCT_Mode, 2);

			Send_Quantized_DCT_Coeff(file, file->split_flag ? file->Quant_coeff_4x4[i] : Quant_blk[Best_DCT_Mode][PredMode], file->split_flag ? p_size / 2 : p_size);
		}
#endif
	}

	/* 4x4상태라면 분할x */
	else
	{
		file->PredMode_4x4[img->index_mode] = PredMode;
		file->DCT_Mode_4x4[img->index_mode] = Best_DCT_Mode;

		for (int i = 0; i < p_size*p_size; i++)
			file->Quant_coeff_4x4[img->index_mode][i] = Quant_blk[Best_DCT_Mode][PredMode][i];

		Write_ReconImageBuffer(img, Recon[Best_DCT_Mode][PredMode], p_size, ul_recon);

		img->cost[file->split_flag] += min_val;	//	img->cost[file->split_flag] += cost[file->split_flag][0];

		img->index_mode++;
	}

	Local_memory_free3D(Quant_blk, Recon, DCT_MODE);
	Local_memory_free2D(Pred_Err, Dct_blk, Pred, cost);
}

void Partitioning(BUFFER* img, Data* file, uint32 nRow, uint32 nCol, uint8 p_size)
{
	uint32 row_order = 0;
	uint32 block_cnt = 0;
	uint32 UpLeftPx_recon = img->UpLeft_px_recon;
	uint32 UpLeftPx_org = img->UpLeft_px_org;

	while (block_cnt < nRow * nCol)
	{
		if (block_cnt != 0 && block_cnt % nCol == 0)
		{
			row_order++;
			if (p_size == PSIZE)
			{
				UpLeftPx_recon = img->UpLeft_px_recon = row_order * (img->nWidth + 1) * p_size;		//등호 차이
				UpLeftPx_org = img->UpLeft_px_org = row_order * img->nWidth * p_size;
			}

			else
			{
				UpLeftPx_recon = img->UpLeft_px_recon + row_order * (img->nWidth + 1) * p_size;
				UpLeftPx_org = img->UpLeft_px_org + row_order * img->nWidth * p_size;
			}
		}
		INTRA_Prediction(img, file, p_size, UpLeftPx_recon, UpLeftPx_org);

		UpLeftPx_recon += p_size;
		UpLeftPx_org += p_size;

		if (p_size == PSIZE)
		{
			img->UpLeft_px_recon = UpLeftPx_recon;
			img->UpLeft_px_org = UpLeftPx_org;
		}
		block_cnt++;
	}
}

void INTER_Prediction(BUFFER* img, Data* file, uint8 p_size, uint32 ul_recon, uint32 ul_org)
{

}