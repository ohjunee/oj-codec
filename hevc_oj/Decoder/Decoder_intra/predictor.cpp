#include "Buffer.h"
#include "input.h"

void Get_PredBlock(BUFFER* img, uint8* Pred_Block, uint8 p_size, uint8 mode, uint32 ul_recon)
{
	uint8 i, j;
	switch (mode) {

		/*수직 예측*/
	case 0:

		//복원이미지 버퍼의 기준화소 위치에서 수평방향으로 1, ... , (p_size + 1)만큼 이동
		for (j = 0; j < p_size; j++) {
			for (i = 0; i < p_size; i++)
			{
				Pred_Block[j + i * p_size] = img->recon_padding[ul_recon + j + 1];
			}
		}
		break;

		/*수평 예측*/
	case 1:

		//복원이미지 버퍼의 기준화소 위치에서 수직방향으로 1, ... , (p_size + 1)만큼 이동
		for (j = 0; j < p_size; j++) {
			for (i = 0; i < p_size; i++)
			{
				Pred_Block[j + i * p_size] = img->recon_padding[ul_recon + (i + 1) * (img->nWidth + 1)];
			}
		}
		break;

		/*DC 예측*/
	case 2:
	{
		uint32 dc_row = 0, dc_col = 0;

		//복원이미지 버퍼의 기준화소 위치에서 수직/수평방향으로 1, ... , (p_size + 1) 위치에 있는 화소값을 모두 더함
		for (int tmp = 0; tmp < p_size; tmp++)
		{
			dc_row += img->recon_padding[ul_recon + tmp + 1];
			dc_col += img->recon_padding[ul_recon + (tmp + 1) * (img->nWidth + 1)];
		}

		uint32 dc_tot = round(double(dc_row + dc_col) / (2 * p_size));

		for (i = 0; i < p_size*p_size; i++) { Pred_Block[i] = dc_tot; }

		break;
	}

	/*대각선 예측*/
	case 3:
	{
		int d_index;
		uint8 refPx_i, u_pxVal, l_pxVal;

		for (j = 0; j < p_size; j++) {
			for (i = 0; i < p_size; i++)
			{
				d_index = i - j;
				refPx_i = abs(d_index);										// 화소값의 차이를 참조화소의 인덱스값으로
				u_pxVal = img->recon_padding[ul_recon + refPx_i];				// (i,j)의 위치의 화소에 대해 i <= j가 참이면 현재 코딩하고자 하는 블록 상단의 참조화소값을 대입
				l_pxVal = img->recon_padding[ul_recon + (refPx_i)* (img->nWidth + 1)];	// 거짓이면 블록 좌측의 참조화소 값을 대입

				Pred_Block[j + i * p_size] = (i <= j) ? u_pxVal : l_pxVal;
			}
		}
		break;
	}
	}
}