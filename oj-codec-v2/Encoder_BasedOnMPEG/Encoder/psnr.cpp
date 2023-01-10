#include "Functions.h"


void PSNR(DATA* file, char* argv[])
{
	file->fp_ori = fopen(file->in_img, "rb");	// 입력 영상
	file->fp_ori_recon = fopen(file->recon_img, "rb");	// 인코더 복원 영상
	file->fp_txt = fopen(TEXT_FILE, "a");	// 결과값 출력 텍스트파일

	double MAX_2 = 255.0 * 255.0;

	double OriFileSize = (HEIGHT * WIDTH * 1.5 * file->VideoFrame);
	double	psnr = 0;

	double	MSE = 0;
	uint8	ori = 0;
	uint8	recon = 0;

	uint32	d = 0;
	uint32	sum = 0;

	uint32 imageSize = 0;

	uint16  frame = 0;

	//동영상 전체 frame 코딩
	while (frame < file->VideoFrame) {

		MSE = 0;

		// 1 frame에 대한 차분 제곱값
		for (int component = 0; component < 3; component++)
		{
			imageSize = !component ? HEIGHT * WIDTH : HEIGHT * WIDTH / 4;

			sum = 0;

			//각 성분별 차분 제곱값
			for (uint32 i = 0; i < imageSize; i++)
			{
				fread(&ori, sizeof(uint8), 1, file->fp_ori);
				fread(&recon, sizeof(uint8), 1, file->fp_ori_recon);

				d = abs(ori - recon);
				sum += d * d;
			}

			MSE += (double)sum / (double)imageSize;
		}

		psnr += 10 * log10(MAX_2 / MSE);

		frame++;
	}

	psnr /= file->VideoFrame;

	fprintf(file->fp_txt, "%s: PSNR: %fdB\n", argv[9], psnr);
	printf("%s: PSNR: %fdB\n", argv[9], psnr);
	fclose(file->fp_ori_recon);
	fclose(file->fp_ori);
	fclose(file->fp_txt);
}

/*
void PSNR(DATA* file, char* argv[])
{
	file->fp_ori		= fopen(file->in_img,	 "rb");	// 입력 영상
	file->fp_ori_recon  = fopen(file->recon_img, "rb");	// 인코더 복원 영상
	file->fp_out		= fopen(file->outfile,	 "rb");	// 비트스트림 파일
	file->fp_txt		= fopen(TEXT_FILE,		 "a");	// 결과값 출력 텍스트파일

	fseek(file->fp_out, 0L, SEEK_END);
	uint32 inFileSize = ftell(file->fp_out);

	double MAX_2 = 255.0 * 255.0;

	double OriFileSize		= (HEIGHT * WIDTH * 1.5 * file->VideoFrame);
	double compressibility	= (double)inFileSize / OriFileSize * 100;
	double	psnr	= 0;

	double	MSE		= 0;
	uint8	ori		= 0;
	uint8	recon	= 0;

	uint32	d		= 0;
	uint32	sum		= 0;

	uint32 imageSize  = 0;

	uint16  frame	  = 0;

	//동영상 전체 frame 코딩
	while (frame < file->VideoFrame){

		MSE = 0;

		// 1 frame에 대한 차분 제곱값
		for (int component = 0; component < 3; component++)
		{
			imageSize = !component ? HEIGHT * WIDTH : HEIGHT * WIDTH / 4;

			sum = 0;

			//각 성분별 차분 제곱값
			for (uint32 i = 0; i < imageSize; i++)
			{
				fread(&ori, sizeof(uint8), 1, file->fp_ori);
				fread(&recon, sizeof(uint8), 1, file->fp_ori_recon);

				d = abs(ori - recon);
				sum += d * d;
			}

			MSE += (double)sum / (double)imageSize;
		}

		psnr += 10 * log10(MAX_2 / MSE);

		frame++;
	}

	psnr /= file->VideoFrame;

	fprintf(file->fp_txt, "%s: pixel_dpcm: %d, dc_dpcm: %d, intra_enable: %d, intra_period: %d, QP_dc: %d, QP_ac: %d,PSNR: %fdB, compressibility %f\n", argv[9], file->pixel_dpcm_mode, file->dc_dpcm_mode, file->intra_enable, file->intra_period, file->QP_dc, file->QP_ac, psnr, compressibility);
	printf("%s: pixel_dpcm: %d, dc_dpcm: %d, intra_enable: %d, intra_period: %d, QP_dc: %d, QP_ac: %d,PSNR: %fdB, compressibility %f\n", argv[9], file->pixel_dpcm_mode, file->dc_dpcm_mode, file->intra_enable, file->intra_period, file->QP_dc, file->QP_ac, psnr, compressibility);
	fclose(file->fp_ori_recon);
	fclose(file->fp_ori);
	fclose(file->fp_txt);
	fclose(file->fp_out);
}

*/