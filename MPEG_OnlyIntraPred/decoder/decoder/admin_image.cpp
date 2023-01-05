#include "Functions.h"

void Setup_Memory(BUFFER* img, DATA* file, Node* POC, uint32 height, uint32 width)
{
	img->Recon			= (uint8*)calloc(height * width, sizeof(uint8));

	img->total_blk		= (width / PSIZE) * (height / PSIZE);
	img->nBlk_width		= width / PSIZE;
	img->nBlk_height	= height / PSIZE;

	img->PredErr_buf2D	= (double**)double_calloc2D(width / PSIZE, PSIZE * 2);
	img->DC_Recon		= (double*)calloc(img->total_blk, sizeof(double));
	img->Recon_DCT_blk = (double*)calloc(PSIZE * PSIZE, sizeof(double));
	img->ZigZag_arr = (double*)calloc(PSIZE * PSIZE, sizeof(double));

	img->blk_cnt	= 0;
	img->row_order	= 0;
	file->MPM_flag	= 0;

	file->PM_Arr		= (uint8*)calloc(img->total_blk, sizeof(uint8));	//Probable Mode Array

	POC->ref_img = (uint8*)calloc(height * width, sizeof(uint8));
}

/* 복원 이미지 파일 생성 및 메모리 반환 */
void ImageSave(BUFFER* img, DATA* file, Node* POC, uint32 height, uint32 width)
{
	for (int i = 0; i < height * width; i++)
	{
		fwrite(&img->Recon[i], sizeof(uint8), 1, file->fp_out);
		POC->ref_img[i] = img->Recon[i];
	}

	free(img->Recon);
	free(img->Recon_DCT_blk);
	free(img->ZigZag_arr);

	free(img->DC_Recon);

	free(img->PredErr_buf2D[0]);
	free(img->PredErr_buf2D);

	free(file->PM_Arr);
}