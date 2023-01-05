 #include "Buffer.h"
#include "input.h"

void Combine_Partition(BUFFER* img, Data* file, uint32 nRow, uint32 nCol, uint8 p_size)
{
	uint32 row_order = 0;
	uint32 UpLeftPx_recon = 0;;
	uint32 block_cnt = 0;

	while (block_cnt < nRow * nCol)
	{
		if (block_cnt != 0 && block_cnt % nCol == 0)
		{
			row_order++;
			UpLeftPx_recon = row_order * (img->nWidth + 1) * p_size;
		}

		Select_SplitFlag(img, file, p_size, UpLeftPx_recon);

		UpLeftPx_recon += p_size;
		block_cnt++;
	}
}

void Combine_4x4(BUFFER* img, Data* file, uint8 p_size, uint32 ul_recon)
{
	uint32 row_order = 0;
	uint32 UpLeft_px_recon = ul_recon;

	uint8* Pred_Block	= (uint8*)	calloc(p_size*p_size, sizeof(uint8));
	file->Recon_DCT_blk	= (double*)	calloc(p_size*p_size, sizeof(double));	//복원된 예측에러블록

	//예측모드 받고, 예측블록 생성->8*8예측블록에 저장, 양자화된 변환계수 i_q, idct 후 8*8복원블록에 저장 
	for (int i = 0; i < 4; i++)
	{
		if (i != 0 && i % 2 == 0)
		{
			row_order++;
			UpLeft_px_recon = ul_recon + row_order * (img->nWidth + 1) * p_size;
		}
		// A0. 예측 모드 읽기
		Read_PRED_INFO(file, &file->PredMode, PRED_MODE);

#if READ_DCT_SKIP == 1
		//A1.DCT skip 읽기
		Read_PRED_INFO(file, &file->DCT_skip, 2);
#else
		file->DCT_skip = 1;
#endif 
		/* B. 예측 모드에 대한 예측 블록 생성 */
		Get_PredBlock(img, Pred_Block, p_size, file->PredMode, UpLeft_px_recon); //nWidth + 1

		/* 양자화된 변환계수를 읽고, 역변환 */
		Read_PredErr(file, p_size);

		/* D. 생성된 복원블록을 padding buffer(recon)에 저장 */
		Get_ReconBuff(img, file, Pred_Block, p_size, UpLeft_px_recon); //nWidth + 1
		
		UpLeft_px_recon += p_size;
	}

	free(file->Recon_DCT_blk);
	free(Pred_Block);
}

void Combine_8x8(BUFFER* img, Data* file, uint8 p_size, uint32 ul_recon)
{
	uint8*	Pred_Block	= (uint8*)		calloc(p_size*p_size, sizeof(uint8));
	file->Recon_DCT_blk  = (double*)	calloc(p_size*p_size, sizeof(double));	//복원된 예측에러블록

	// A0. 예측모드 읽기
	Read_PRED_INFO(file, &file->PredMode, PRED_MODE);

	//A1.DCT skip 읽기
#if READ_DCT_SKIP == 1
	Read_PRED_INFO(file, &file->DCT_skip, 2);
#else
	file->DCT_skip = 1;
#endif
	/* B. 예측 모드에 대한 예측 블록 생성 */
	Get_PredBlock(img, Pred_Block, p_size, file->PredMode, ul_recon); //nWidth + 1

	/* C2 전달받은 예측에러블록 정보 역양자화, IDCT 수행 */
	Read_PredErr(file, p_size);

	/* D. 생성된 복원블록을 padding buffer(recon)에 저장 */
	Get_ReconBuff(img, file, Pred_Block, p_size, ul_recon); //nWidth + 1

	free(file->Recon_DCT_blk);
	free(Pred_Block);
}

/* 분할 모드*/
void Select_SplitFlag(BUFFER* img, Data* file, uint8 p_size, uint32 ul_recon)
{
	Read_PRED_INFO(file, &file->split_flag, 2);

	/* flag가 1이면 분할, 0이면 그대로 */
	file->split_flag ? Combine_4x4(img, file, p_size / 2, ul_recon) : Combine_8x8(img, file, p_size, ul_recon);
}