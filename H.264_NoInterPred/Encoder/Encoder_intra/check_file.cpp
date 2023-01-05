#include "Buffer.h"
#include "input.h"

void Open_File(Data* file)
{
	file->fp_ori		= fopen(INPUT_FILENAME,		"rb"); // 입력 영상
	file->fp_out		= fopen(OUTPUT_FILENAME_1,	"wb"); // 출력 영상
	file->fp_ori_recon	= fopen(OUTPUT_FILENAME_2,	"wb"); // 인코더 복원 영상

	file->frameCount = 0;
	file->output = 0;
	file->length = 0;
}

void Close_File(Data* file)
{
	fclose(file->fp_ori);
	fclose(file->fp_out);
	fclose(file->fp_ori_recon);
}

void checkFile(Data* file, bool Bool)
{
	(Bool) ? Open_File(file) : Close_File(file);
}