#include "Buffer.h"
#include "input.h"
#include "Node.h"

void encoder(BUFFER* img, Data* file)
{
	//동영상 전체 frame 코딩
	while (file->frameCount < VIDEO_FRAME)
	{
		// 동영상 1 frame 코딩(4:2:0 방식, Y/Cb/Cr 각각의 경우의 비트스트림 생성)
		for (int component = 0; component < 3; component++)
			component == 0 ? Encoding(img, file, HEIGHT, WIDTH) : Encoding(img, file, HEIGHT / 2, WIDTH / 2);

		// 남은 비트는 msb 쪽으로 밀기
		if (file->frameCount == VIDEO_FRAME - 1 && file->length % 8 != 0)
		{
			file->output = file->output << (8 - (file->length % 8));
			fwrite(&file->output, sizeof(uint8), 1, file->fp_out);
		}
		file->frameCount++;
	}
}

void Encoding(BUFFER* img, Data* file, uint32 height, uint32 width)
{
	if (height == HEIGHT / 2)
		Just_Read_CbCr(img, file, height, width);				// Y성분을 제외한 Cb Cr정보는 영상압축과정에서 제외

	else
	{
		sturct_memory_check(img, file, height, width);			// 버퍼 영상 메모리 할당 및 초기화
		ImageLoad(img, file, height, width);					// 영상 입력
		Partitioning(img, file, img->nRow, img->nCol, PSIZE);	// 영상 분할
		Write_Recon_Image(img, file, height, width);			// 복원 영상 저장
		struct_memory_free(img, file);							// 버퍼 영상 메모리 반환
	}
}

void Just_Read_CbCr(BUFFER* img, Data* file, uint32 height, uint32 width)
{
	uint8* CbCr_img = (uint8*)calloc(height*width, sizeof(uint8));

	fread(CbCr_img, sizeof(uint8), height*width, file->fp_ori);

	free(CbCr_img);
}