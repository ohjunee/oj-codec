#include "Buffer.h"
#include "input.h"

void Check_Padding_Image(BUFFER* img, Data* file, uint32 height, uint32 width, bool Bool)
{
	if (Bool)
	{
		/* 패딩할 화소갯수 초기화 */
		img->extra_row = height % PSIZE ? PSIZE - (height % PSIZE) : 0;
		img->extra_col = width  % PSIZE ? PSIZE - (width % PSIZE) : 0;

		img->nRow = ((height + img->extra_row) / PSIZE);
		img->nCol = ((width + img->extra_col) / PSIZE);

		// 패딩 원본이미지 버퍼의 가로길이
		img->nWidth = PSIZE * img->nCol;					

		//패딩 이미지 버퍼 메모리 할당, 128로 초기화
		img->recon_padding = (uint8*)malloc((height + img->extra_row + 1) * (width + img->extra_col + 1) * sizeof(uint8));

		memset(img->recon_padding, PADDING_VAL, sizeof(uint8)*(height + img->extra_row + 1)	* (width + img->extra_col + 1));
	}
	else
	{
		//패딩 이미지 버퍼 메모리 반환
		free(img->recon_padding);
	}
}