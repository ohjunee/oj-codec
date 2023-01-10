#include "Functions.h"

void Read_Inter(BUFFER* img, DATA* file, Node* POC, uint32& upleft_px)
{
	uint8* Pred_Block = (uint8*)calloc(img->macroSize, sizeof(uint8));

	if (!Pred_Block)
	{
		printf("memory error");
		exit(0);
	}

	double* ImgBuf = (double*)calloc(img->macroSize, sizeof(double));	//복원된 예측에러블록

	if (!ImgBuf)
	{
		printf("memory error");
		exit(0);
	}

	memset(file->recon_bit, 0, sizeof(int) * img->macroSize);

	/* A1. read MVmodeflag, motion vector */

	// Y
	if (!file->comp)
	{
		/* Entropy 1. MV modeflag: ME mode 16x16(0), 8x8(1) */
		Read_PRED_INFO(file, file->MVmodeflag, 2);

		/* Entropy 2. Motion Vector */
		Read_MV(file);

		/* save motion vector of Y in macro block unit*/
		POC->mv_arr[0][img->blk_cnt] = file->mv_arr[0];
		POC->mv_arr[1][img->blk_cnt] = file->mv_arr[1];
	}

	/* A1. read Y motion vector */

	// CbCr
	else
	{
		file->mv_arr[0] = round(POC->mv_arr[0][img->blk_cnt] / 2);
		file->mv_arr[1] = round(POC->mv_arr[1][img->blk_cnt] / 2);
	}

	/* component 값에 따라 달라지는 주소값 */
	switch (file->comp % 3)
	{
	case 0:
		file->buffer = POC->Y;
		break;
	case 1:
		file->buffer = POC->Cb;
		break;
	case 2:
		file->buffer = POC->Cr;
		break;
	}

	/* C2 전달받은 예측에러블록 정보 복호화 -> 역양자화, IDCT 수행 */

		/* C2 전달받은 예측에러블록 정보 복호화 */
	for (uint8 i = 0; i < img->macroSize; i++)
	{
		uint8 bitCnt = 0;
		uint16 result = 0, buffer = 0;

		if (i == 1)
		{
			file->AC_flag = file->output << (file->length % 8) & 128 ? 1 : 0;
			file->length++;

			if (file->length % 8 == 0)
			{
				fread(&file->output, sizeof(uint8), 1, file->fp_in);
				file->length = 0;
			}

			/* AC flag = 1; all-zero라는 의미로 1비트 전송, AC성분 인코딩 x, 디코더에서 1이라는 비트를 받으면, AC성분값을 모두 0으로 설정) */
			if (file->AC_flag)
				break;
		}

		while (bitCnt < 10)
		{
			// read code word
			buffer = file->output << (file->length % 8) & 128 ? 1 : 0;

			result = result << 1 | buffer;

			file->length++;
			bitCnt++;

			if (file->length % 8 == 0)
			{
				fread(&file->output, sizeof(uint8), 1, file->fp_in);
				file->length = 0;
			}

			switch (result)
			{
			case 0:
				if (bitCnt == 2)
				{
					file->recon_bit[i] = 0;
					bitCnt = 10;
				}
				break;

			case 2:
				if (bitCnt == 3)
				{
					int sign = file->output << (file->length % 8) & 128 ? 1 : 0;
					file->length++;
					file->recon_bit[i] = pow(-1, sign);

					if (file->length % 8 == 0)
					{
						fread(&file->output, sizeof(uint8), 1, file->fp_in);
						file->length = 0;
					}

					bitCnt = 10;
				}
				break;

			case 3:
				if (bitCnt == 3)
				{
					Read_Symbol(file, i, 1, QT_COEFF);
					bitCnt = 10;
				}
				break;

			case 4:
				if (bitCnt == 3)
				{
					Read_Symbol(file, i, 2, QT_COEFF);
					bitCnt = 10;
				}
				break;

			case 5:
				if (bitCnt == 3)
				{
					Read_Symbol(file, i, 3, QT_COEFF);
					bitCnt = 10;
				}
				break;

			case 6:
				if (bitCnt == 3)
				{
					Read_Symbol(file, i, 4, QT_COEFF);
					bitCnt = 10;
				}
				break;

			case 14:
				if (bitCnt == 4)
				{
					Read_Symbol(file, i, 5, QT_COEFF);
					bitCnt = 10;
				}
				break;

			case 30:
				if (bitCnt == 5)
				{
					Read_Symbol(file, i, 6, QT_COEFF);
					bitCnt = 10;
				}
				break;

			case 62:
				if (bitCnt == 6)
				{
					Read_Symbol(file, i, 7, QT_COEFF);
					bitCnt = 10;
				}
				break;

			case 126:
				if (bitCnt == 7)
				{
					Read_Symbol(file, i, 8, QT_COEFF);
					bitCnt = 10;
				}
				break;

			case 254:
				if (bitCnt == 8)
				{
					Read_Symbol(file, i, 9, QT_COEFF);
					bitCnt = 10;
				}
				break;

			case 510:
				if (bitCnt == 9)
				{
					Read_Symbol(file, i, 10, QT_COEFF);
					bitCnt = 10;
				}
				break;

			case 1022:
				Read_Symbol(file, i, 11, QT_COEFF);
				break;
			}
		}
	}
	/* Reordering (Decoder에서 복호화된 양자화 계수값들 재정렬) */
	Recon_ZigZagScan(img, file);										// file->recon_bit -> file->ZigZag_blk

	/* 양자화 변환계수블록의 DC값을 DPCM 복호화 */
	Inverse_DC_DPCM(img, file);
	Inverse_Q(img, file);
	IDCT(img, file);										// file->ZigZag_blk -> img->Recon_DCT_blk

	/* D. 예측블록 + 복원된 예측에러블록 = 복원블록 -> 복원이미지 */
	/* reconstruction: error image + prediction image*/
	for (int j = 0; j < img->p_size; j++) {
		for (int i = 0; i < img->p_size; i++)
		{
			int recon_px = file->buffer[img->UpLeftPx_ref + (file->mv_arr[0] + j) + (file->mv_arr[1] + i) * (img->width_pad)] + (int)round(img->Recon_DCT_blk[j + i * img->p_size]);

			// 클리핑
			if (recon_px > 255)
				img->Recon[upleft_px + j + i * img->width] = 255;
			else if (recon_px < 0)
				img->Recon[upleft_px + j + i * img->width] = 0;
			else
				img->Recon[upleft_px + j + i * img->width] = recon_px;
		}
	}
	img->UpLeftPx_ref += img->p_size;

	free(Pred_Block);
}

void Read_MV(DATA* file)
{
	uint8 symbol, sign = 0;

	/* C2 전달받은 예측에러블록 정보 복호화 */
	for (uint8 i = 0; i < 2; i++)
	{
		uint8 bitCnt = 0;
		uint8 result = 0, buffer = 0;

		while (bitCnt < 3)
		{
			// read code word
			buffer = file->output << (file->length % 8) & 128 ? 1 : 0;

			result = result << 1 | buffer;

			file->length++;
			bitCnt++;

			if (file->length % 8 == 0)
			{
				fread(&file->output, sizeof(uint8), 1, file->fp_in);
				file->length = 0;
			}

			switch (result)
			{
			case 0:
				if (bitCnt == 2)
				{
					file->mv_arr[i] = 0;
					bitCnt = 3;
				}
				break;

			case 2:
				if (bitCnt == 3)
				{
					int sign = file->output << (file->length % 8) & 128 ? 1 : 0;

					file->length++;
					file->mv_arr[i] = pow(-1, sign);

					if (file->length % 8 == 0)
					{
						fread(&file->output, sizeof(uint8), 1, file->fp_in);
						file->length = 0;
					}

					bitCnt = 3;
				}
				break;

			case 3:
				if (bitCnt == 3)
				{
					Read_Symbol(file, i, 1, MV);
					bitCnt = 3;
				}
				break;

			case 4:
				if (bitCnt == 3)
				{
					Read_Symbol(file, i, 2, MV);
					bitCnt = 3;
				}
				break;

			case 5:
				if (bitCnt == 3)
				{
					Read_Symbol(file, i, 3, MV);
					bitCnt = 3;
				}
				break;
			}
		}
	}
}