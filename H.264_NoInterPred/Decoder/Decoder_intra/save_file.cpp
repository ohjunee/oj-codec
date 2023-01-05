#include "Buffer.h"
#include "input.h"

void Get_ReconBuff(BUFFER* img, Data* file, uint8* Pred_Block, uint8 p_size, uint32 ul_recon)
{
	/* 만들어진 잔차블록과 예측블록의 합을 padding buffer(recon)에 저장 */
	uint32 init_position = ul_recon + (img->nWidth + 1) + 1;

	for (uint8 j = 0; j < p_size; j++) {
		for (uint8 i = 0; i < p_size; i++)
		{
			int recon_px = Pred_Block[j + i * p_size] + (int)round(file->Recon_DCT_blk[j + i * p_size]);

			// 클리핑
			if (recon_px > 255)
			{
				img->recon_padding[init_position + i * (img->nWidth + 1) + j] = 255;
			}

			else if (recon_px < 0)
			{
				img->recon_padding[init_position + i * (img->nWidth + 1) + j] = 0;
			}
			else
			{
				img->recon_padding[init_position + i * (img->nWidth + 1) + j] = recon_px;
			}
		}
	}
}

void Write_Recon_Image(BUFFER* img, Data* file, uint32 height, uint32 width)
{
	uint32 init_position = (img->nWidth + 1) + 1;	//nWidth : padding buffer(recon)의 가로길이, //init_position: 값을 읽어오는 초기 위치
	
	for (int i = 0; i < height; i++)
	{
		int cur_position = init_position;

		for (int j = 0; j < width; j++)
		{
			fwrite(&img->recon_padding[cur_position], sizeof(uint8), 1, file->fp_out);
			cur_position++;
		}
		init_position += img->nWidth + 1;
	}
}