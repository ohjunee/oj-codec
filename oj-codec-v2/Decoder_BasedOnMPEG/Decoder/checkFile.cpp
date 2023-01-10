#include "Functions.h"
#pragma warning(disable: 6387)

void Open_File(DATA* file, int argc, char* argv[])
{
	char ori[] = { "G:\\images\\" };
	char recon_en[] = { "G:\\images\\encoder\\ver_1.1\\recon_인코더\\" };
	char recon_de[] = { "G:\\images\\decoder\\recon_디코더\\" };
	char bitstream[] = { "G:\\images\\encoder\\ver_1.1\\bitstream\\" };

	char yuv[] = { ".yuv" };
	char raw[] = { ".raw" };

	file->ori = (char*)malloc(sizeof(char) * (strlen(argv[3]) + strlen(ori) + strlen(yuv)));
	file->recon_en = (char*)malloc(sizeof(char) * (strlen(argv[3]) + strlen(recon_en)) + strlen(yuv));
	file->recon_de = (char*)malloc(sizeof(char) * (strlen(argv[3]) + strlen(recon_de)) + strlen(yuv));
	file->infile = (char*)malloc(sizeof(char) * (strlen(argv[3]) + strlen(bitstream) + strlen(raw)));

	if (file->ori == 0)
		exit(0);
	if (file->recon_en == 0)
		exit(0);
	if (file->recon_de == 0)
		exit(0);
	if (file->infile == 0)
		exit(0);

	// 파일 저장 위치 복사
	strcpy(file->infile, bitstream);
	strcpy(file->recon_en, recon_en);
	strcpy(file->recon_de, recon_de);
	strcpy(file->ori, ori);

	// 원본 파일 이름을 저장위치에 이어 붙이기
	strcat(file->ori, argv[3]);

	// 복원파일, 비트스트림 파일 번호 수정
	memmove(argv[3], argv[2], strlen(argv[2])); // 3번 맨 첫번째 글자부터 2번으로 수정

	// 복원파일, 비트스트림 파일 이름을 저장위치에 이어붙이기
	strcat(file->recon_en, argv[3]);
	strcat(file->recon_de, argv[3]);
	strcat(file->infile, argv[3]);

	// 확장자 명을 저장위치에 이어붙이기
	strcat(file->infile, raw);
	strcat(file->recon_de, yuv);

	strcat(file->ori, yuv);
	strcat(file->recon_en, yuv);

	file->fp_in = fopen(file->infile, "rb");		// 입력 파일(비트스트림)
	file->fp_out = fopen(file->recon_de, "wb");		// 디코더 복원 영상

	file->VideoFrame = atoi(argv[1]);

	file->frameCount = 0;
	file->output = 0;
	file->length = 0;

	file->AC_flag = 0;
}

void Close_File(DATA* file)
{
	fclose(file->fp_in);
	fclose(file->fp_out);
}

void checkFile(DATA* file, int argc, char* argv[], bool Bool)
{
	(Bool) ? Open_File(file, argc, argv) : Close_File(file);
}

void Check_Image(DATA* file, char* argv[])
{
	double MAX_2 = 255.0 * 255.0, psnr = 0, MSE = 0;
	double OriFileSize = (file->height * file->width * 1.5 * file->VideoFrame);

	uint8 ori = 0, recon = 0;
	uint32 d = 0, sum = 0;

	uint16 frame = 0;
	uint32 imageSize = 0;
	uint32 cnt = 0;

	fopen_s(&file->fp_in, file->infile, "rb");	// 비트스트림 파일
	fopen_s(&file->fp_out, file->recon_de, "rb");	// 디코더 복원영상

	fopen_s(&file->fp_ori, file->ori, "rb");	// 원본영상
	fopen_s(&file->fp_recon_en, file->recon_en, "rb");	// 인코더 복원영상
	fopen_s(&file->fp_txt, TEXT_FILE, "a");	// 결과값 텍스트파일

	fseek(file->fp_in, 0L, SEEK_END);
	file->inFileSize = ftell(file->fp_in);

	double compressibility = ((double)file->inFileSize / OriFileSize) * 100;


	/* 인코더 디코더 복원영상의 mismatch 확인*/
	while (feof(file->fp_recon_en) == 0 && feof(file->fp_out) == 0)
	{
		uint8 file_1_pixel, file_2_pixel;

		fread(&file_1_pixel, sizeof(uint8), 1, file->fp_recon_en);
		fread(&file_2_pixel, sizeof(uint8), 1, file->fp_out);

		cnt++;

		if (file_1_pixel != file_2_pixel)
		{
			printf("두 파일이 다릅니다. cnt: %d\n\n", cnt);
			break;
		}
	}
	printf("두 파일은 같습니다.\n");

	fseek(file->fp_out, 0L, SEEK_SET);

	/* psnr 및 압축률 계산 */
	while (frame < file->VideoFrame) {

		MSE = 0;

		// 1 frame에 대한 차분 제곱값
		for (int component = 0; component < 3; component++)
		{
			imageSize = !component ? file->height * file->width : file->height * file->width / 4;

			sum = 0;

			//각 성분별 차분 제곱값
			for (uint32 i = 0; i < imageSize; i++)
			{
				fread(&ori, sizeof(uint8), 1, file->fp_ori);
				fread(&recon, sizeof(uint8), 1, file->fp_out);

				d = abs(ori - recon);
				sum += d * d;
			}

			MSE += (double)sum / (double)imageSize;
		}

		psnr += 10 * log10(MAX_2 / MSE);

		frame++;
	}

	psnr /= file->VideoFrame;

	fprintf(file->fp_txt, "%s: pixel_dpcm: %d, dc_dpcm: %d, intra_enable: %d, intra_period: %d, QP_dc: %d, QP_ac: %d,PSNR: %fdB, compressibility %f%\n", argv[3], file->pxDPCM_MODE, file->DC_DPCM_MODE, file->IntraEnable, file->IntraPeriod, file->QP_DC, file->QP_AC, psnr, compressibility);
	printf("%s: pixel_dpcm: %d, dc_dpcm: %d, intra_enable: %d, intra_period: %d, QP_dc: %d, QP_ac: %d,PSNR: %fdB, compressibility %f%\n", argv[3], file->pxDPCM_MODE, file->DC_DPCM_MODE, file->IntraEnable, file->IntraPeriod, file->QP_DC, file->QP_AC, psnr, compressibility);

	fclose(file->fp_ori);
	fclose(file->fp_recon_en);

	fclose(file->fp_txt);
	fclose(file->fp_in);
	fclose(file->fp_out);
}