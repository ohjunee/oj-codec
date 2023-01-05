#include "Functions.h"

void Open_File(DATA* file, int argc, char* argv[])
{
	char ori[] = { "G:\\images\\" };
	char recon[] = { "G:\\images\\encoder\\reconImg\\" };
	char bitstream[] = { "G:\\images\\encoder\\bitstream\\" };

	char yuv[] = { ".yuv" };
	char raw[] = { ".raw" };

	file->in_img = (char*)malloc(sizeof(char) * (strlen(argv[9]) + strlen(ori) + strlen(yuv)));
	file->recon_img = (char*)malloc(sizeof(char) * (strlen(argv[9]) + strlen(recon)) + strlen(yuv));
	file->outfile = (char*)malloc(sizeof(char) * (strlen(argv[9]) + strlen(bitstream) + strlen(raw)));

	if (file->in_img == 0)
		exit(0);

	if (file->recon_img == 0)
		exit(0);

	if (file->outfile == 0)
		exit(0);

	// 파일 저장 위치 복사
	strcpy(file->in_img, ori);
	strcpy(file->recon_img, recon);
	strcpy(file->outfile, bitstream);

	// 원본 파일 이름을 저장위치에 이어 붙이기
	strcat(file->in_img, argv[9]);

	// 복원파일, 비트스트림 파일 번호 수정
	memmove(argv[9], argv[8], strlen(argv[8]));

	// 복원파일, 비트스트림 파일 이름을 저장위치에 이어붙이기
	strcat(file->recon_img, argv[9]);
	strcat(file->outfile, argv[9]);

	// 확장자 명을 저장위치에 이어붙이기
	strcat(file->in_img, yuv);
	strcat(file->recon_img, yuv);
	strcat(file->outfile, raw);

	file->fp_ori = fopen(file->in_img, "rb");			// 입력 영상
	file->fp_ori_recon = fopen(file->recon_img, "wb");			// 인코더 복원 영상
	file->fp_out = fopen(file->outfile, "wb");			// 출력 파일(비트스트림)


	file->VideoFrame = atoi(argv[1]);

	file->pixel_dpcm_mode = atoi(argv[2]); // 6
	file->dc_dpcm_mode = atoi(argv[3]); // 6
	file->intra_enable = atoi(argv[4]); // 0
	file->intra_period = atoi(argv[5]); // 1
	file->QP_dc = atoi(argv[6]); // 1
	file->QP_ac = atoi(argv[7]); // 1

	file->frameCount = 0;
	file->output = 0;
	file->length = 0;
	file->MVmodeflag = 0;
	file->AC_flag = 0;
	file->nbyte = 0;
}

void Close_File(DATA* file)
{
	fclose(file->fp_ori);
	fclose(file->fp_out);
	fclose(file->fp_ori_recon);
}

void checkFile(DATA* file, int argc, char* argv[], bool Bool)
{
	(Bool) ? Open_File(file, argc, argv) : Close_File(file);
}