#include "Functions.h"

void Setup(BUFFER* img, DATA* file, Node* POC)
{
	/* image format */
	img->height = (!file->comp) ? file->height : file->height / 2;
	img->width = (!file->comp) ? file->width : file->width / 2;
	img->p_size = (!file->comp) ? PSIZE : PSIZE / 2;

	img->imgSize = img->height * img->width;
	img->macroSize = img->p_size * img->p_size;

	img->nBlk_width = img->width / img->p_size;
	img->nBlk_height = img->height / img->p_size;
	img->total_blk = img->nBlk_width * img->nBlk_height;

	/* padding image format */
	img->height_pad = img->height + img->p_size * 2;
	img->width_pad = img->width + img->p_size * 2;

	img->nBlk_Pad_width = img->width_pad / img->p_size;
	img->nBlk_Pad_height = img->height_pad / img->p_size;
	img->tot_Pad_blk = (img->nBlk_Pad_width * img->nBlk_Pad_height);

	/* image buffer */
	img->Recon = (uint8*)calloc(img->imgSize, sizeof(uint8));
	img->Recon_DCT_blk = (double*)calloc(img->macroSize, sizeof(double));
	file->ZigZag_arr = (double*)calloc(img->macroSize, sizeof(double));
	file->recon_bit = (int*)calloc(img->macroSize, sizeof(int));

	if (file->pxDPCM_MODE != 6)
		img->Ref_Pixel2D = memory_calloc2D<double>(img->nBlk_width, img->p_size * 2);

	if (file->DC_DPCM_MODE != 6)
		img->DC_Recon = (double*)calloc(img->total_blk, sizeof(double));

	if (file->IntraEnable)
		file->PM_Arr = (uint8*)calloc(img->total_blk, sizeof(uint8));	//Probable Mode Array

	/* reset */
	img->blk_cnt = 0;
	img->row_order = 0;
	file->MPM_flag = 0;

	/* component 값에 따라 달라지는 주소값 */
	switch (file->comp % 3)
	{
	case 0:
	{
		file->ref_img = POC->Y = (uint8*)calloc(img->height_pad * img->width_pad, sizeof(uint8));
		POC->mv_arr = memory_calloc2D<int>(2, img->total_blk);
		break;
	}
	case 1:
		file->ref_img = POC->Cb = (uint8*)calloc(img->height_pad * img->width_pad, sizeof(uint8));
		break;
	case 2:
		file->ref_img = POC->Cr = (uint8*)calloc(img->height_pad * img->width_pad, sizeof(uint8));
		break;
	}
}

/* 복원 이미지 파일 생성 및 메모리 반환 */
void ImageSave(BUFFER* img, DATA* file)
{
	for (int i = 0; i < img->imgSize; i++)
	{
		fwrite(&img->Recon[i], sizeof(uint8), 1, file->fp_out);
	}

	// padding -> 패딩한 이미지를 참조화소로 저장
	Padding(img, file);

	free(img->Recon);
	free(img->Recon_DCT_blk);
	free(file->ZigZag_arr);
	free(file->recon_bit);

	if (file->DC_DPCM_MODE != 6)
		free(img->DC_Recon);

	if (file->pxDPCM_MODE != 6)
	{
		free(img->Ref_Pixel2D[0]);
		free(img->Ref_Pixel2D);
	}

	if (file->IntraEnable)
		free(file->PM_Arr);
}

void Padding(BUFFER* img, DATA* file)
{
	uint32 UpLeftPx_Pad = 0, UpLeftPx_Org = 0, blk_cnt = 0, row_order = 0;
	uint32 cnt_L = 0, cnt_R = 0, cnt_Up = 0, cnt_Bottom = 0;

	img->blk_cnt = 0;
	img->row_order = 0;

	while (img->blk_cnt < img->tot_Pad_blk)
	{
		img->isFirstBlk_in_row_pad = !(img->blk_cnt % img->nBlk_Pad_width);			// 0이면 true가 되어, 각 행의 첫번째 블록임을 표시
		img->isFirstBlk_in_row_org = !(blk_cnt % img->nBlk_width);					// 0이면 true가 되어, 각 행의 첫번째 블록임을 표시

		img->isLastBlk_in_row_pad = !((img->blk_cnt + 1) % img->nBlk_Pad_width);	// 0이면 true가 되어, 각 행의 마지막 블록임을 표시

		// 두번째 행 이상의 첫번째 블록의 경우
		if (img->blk_cnt != 0 && img->isFirstBlk_in_row_pad)
		{
			img->row_order++;
			UpLeftPx_Pad = img->row_order * (img->width + img->p_size * 2) * img->p_size;
		}

		// 왼쪽 블록열
		if (img->isFirstBlk_in_row_pad)
		{
			//좌 상단 블록
			if (img->row_order == 0)
			{
				for (int j = 0; j < img->p_size; j++) {
					for (int i = 0; i < img->p_size; i++) {
						file->ref_img[UpLeftPx_Pad + j + i * (img->width + img->p_size * 2)] = img->Recon[0];
					}
				}
			}

			//좌 하단 블록
			else if (img->row_order == img->nBlk_Pad_height - 1)
			{
				for (int j = 0; j < img->p_size; j++) {
					for (int i = 0; i < img->p_size; i++) {
						file->ref_img[UpLeftPx_Pad + j + i * (img->width + img->p_size * 2)] = img->Recon[img->width * (img->height - 1)];
					}
				}
			}

			else
			{
				for (int j = 0; j < img->p_size; j++) {
					for (int i = 0; i < img->p_size; i++) {
						file->ref_img[UpLeftPx_Pad + j + i * (img->width + img->p_size * 2)] = img->Recon[(cnt_L + i) * img->width];
					}
				}
				cnt_L += img->p_size;
			}
		}
		// 오른쪽 블록열
		else if (img->isLastBlk_in_row_pad)
		{
			//우 상단 블록
			if (img->row_order == 0)
			{
				for (int j = 0; j < img->p_size; j++) {
					for (int i = 0; i < img->p_size; i++) {
						file->ref_img[UpLeftPx_Pad + j + i * (img->width + img->p_size * 2)] = img->Recon[img->width - 1];
					}
				}
			}

			//우 하단 블록
			else if (img->row_order == img->nBlk_Pad_height - 1)
			{
				for (int j = 0; j < img->p_size; j++) {
					for (int i = 0; i < img->p_size; i++) {
						file->ref_img[UpLeftPx_Pad + j + i * (img->width + img->p_size * 2)] = img->Recon[img->width * img->height - 1];
					}
				}
			}

			else
			{
				for (int j = 0; j < img->p_size; j++) {
					for (int i = 0; i < img->p_size; i++) {
						file->ref_img[UpLeftPx_Pad + j + i * (img->width + img->p_size * 2)] = img->Recon[img->width - 1 + (cnt_R + i) * img->width];
					}
				}
				cnt_R += img->p_size;
			}
		}

		// 상단 블록행
		else if (img->blk_cnt != 0 && img->row_order == 0)
		{
			for (int j = 0; j < img->p_size; j++) {
				for (int i = 0; i < img->p_size; i++) {
					file->ref_img[UpLeftPx_Pad + j + i * (img->width + img->p_size * 2)] = img->Recon[cnt_Up + j];
				}
			}
			cnt_Up += img->p_size;
		}
		// 하단 블록행
		else if (img->isFirstBlk_in_row_pad == false && img->row_order == img->nBlk_Pad_height - 1)
		{
			for (int j = 0; j < img->p_size; j++) {
				for (int i = 0; i < img->p_size; i++) {
					file->ref_img[UpLeftPx_Pad + j + i * (img->width + img->p_size * 2)] = img->Recon[img->width * (img->height - 1) + cnt_Bottom + j];
				}
			}
			cnt_Bottom += img->p_size;
		}

		else
		{
			if (UpLeftPx_Org != 0 && img->isFirstBlk_in_row_org)
			{
				row_order++;
				UpLeftPx_Org = row_order * img->width * img->p_size;
			}

			for (int j = 0; j < img->p_size; j++) {
				for (int i = 0; i < img->p_size; i++) {
					file->ref_img[UpLeftPx_Pad + j + i * (img->width + 2 * img->p_size)] = img->Recon[UpLeftPx_Org + j + i * img->width];
				}
			}

			UpLeftPx_Org += img->p_size;
			blk_cnt++;
		}

		UpLeftPx_Pad += img->p_size;
		img->blk_cnt++;
	}
}
