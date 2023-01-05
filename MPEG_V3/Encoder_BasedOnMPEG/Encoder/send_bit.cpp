#include "Functions.h"

void Send_StreamHeader(DATA* file)				// 12byte		
{
	char ASCII_Array[4] = { 73, 67, 83, 80 };	// ICSP				4byte

	uint16 H = HEIGHT, W = WIDTH;				// 4byte


	for (int i = 0; i < 4; i++)
	{
		fwrite(&ASCII_Array[i], sizeof(char), 1, file->fp_out);
		file->nbyte++;
	}
	fwrite(&H, sizeof(uint16), 1, file->fp_out);
	fwrite(&W, sizeof(uint16), 1, file->fp_out);
	file->nbyte += 4;

	Send_PredInfo_FLC(file, file->QP_dc, 17);						// (0~16)				5
	Send_PredInfo_FLC(file, file->QP_ac, 17);						// (0~16)				5
	Send_PredInfo_FLC(file, file->pixel_dpcm_mode, nDPCM_Mode);		// (0~6) { 0, 2, 6 };	3bit
	Send_PredInfo_FLC(file, file->dc_dpcm_mode, nDPCM_Mode);			// (0~6) { 0, 6 };		3
//	Send_PredInfo_FLC(file, MV_PREDMODE, 6);							// (0~5)				3
	Send_PredInfo_FLC(file, file->intra_period, 11);				// (0~10)				4
	Send_PredInfo_FLC(file, file->intra_enable, 2);					// (0,1)				1		총 21bit (3byte)

	// send lsb (1byte) lsb 3bit
	if (file->length % 8 != 0)
	{
		file->output = file->output << (8 - (file->length % 8));
		fwrite(&file->output, sizeof(uint8), 1, file->fp_out);
		file->nbyte++;
		file->length = 0;
	}
}

void Send_INTER_INFO(DATA* file)
{
	if (!file->comp)
	{
		/* Entropy 1. MV modeflag 전송: ME mode 16x16(0), 8x8(1) */
		Send_PredInfo_FLC(file, file->MVmodeflag, 2);

		/* Entropy 2. Motion Vector 전송 */
		Send_MV(file);
	}

	/* Entropy 3. 양자화변환계수 전송; DC값, AC flag, AC값*/
	Send_Quantized_DCT_Coeff(file);
}


void Send_PredInfo_FLC(DATA* file, uint8 code_symbol, uint8 level)
{
	uint8	buffer = 0;
	uint8	bitLen = ceil(log(level, 2));				//표현부호어 갯수에 따라 비트의 길이 결정(반올림해야함), 예측모드의 수가 6이라면 log2의 6은 소수가 나온다.
	uint8	check_bit = pow(2, bitLen - 1);

	for (int i = 0; i < bitLen; i++)
	{
		buffer = code_symbol << i & check_bit ? 1 : 0;
		file->output = (file->output << 1) | buffer;
		file->length++;

		if (file->length % 8 == 0)
		{
			fwrite(&file->output, sizeof(uint8), 1, file->fp_out);
			file->nbyte++;
			file->length = 0;
			file->output = 0;
		}
	}
}

void Send_MV(DATA* file)
{
	file->mv_arr[0] = file->mv_x;
	file->mv_arr[1] = file->mv_y;

	uint8 symbol, sign = 0;

	for (int idx = 0; idx < 2; idx++)
	{
		symbol = abs(file->mv_arr[idx]);
		sign = (file->mv_arr[idx] < 0) ? 1 : 0;	//negative 1 , positive 0

		/* Entropy 4. 심볼 전송 */
		if (symbol == 0)			// 0
			Send_AbsVal(file, 0, 2);

		else if (symbol == 1)	// 1
		{
			Send_AbsVal(file, 2, 3);
			Send_Sign(file, sign);
		}

		else
		{
			for (int n = 1; n < 14; n++)
			{
				if (symbol >= (1 << n) && symbol < (1 << n + 1))
				{
					switch (n)
					{
					case 1:
						Send_AbsVal(file, 3, 3); // 011
						n = 14;
						break;

					case 2:
						Send_AbsVal(file, 4, 3); // 100
						n = 14;
						break;

					case 3:
						Send_AbsVal(file, 5, 3); // 101
						n = 14;
						break;
					}
				}
			}
			Send_Sign(file, sign);
			Send_Range_FLC(file, symbol);
		}
	}
}

void Send_Quantized_DCT_Coeff(DATA* file)
{
	/* Entropy 2.1  AC flag여부 확인 */
	check_AC_flag(file);

	/* Entropy 2.1 양자화된 변환계수값을 Code Table에 따라 부호화 */
	classification(file);
}

void check_AC_flag(DATA* file)
{
	uint8 p_size = (!file->comp) ? PSIZE : PSIZE / 2;

	int index = 0;
	int ac_cnt = 0;

	for (int i = 1; i < p_size * p_size; i++)
	{
		if (!file->ZigZag_arr[i])
			ac_cnt++;
	}
	if (ac_cnt == p_size * p_size - 1)
		file->AC_flag = 1;
	else
		file->AC_flag = 0;
}

void classification(DATA* file)
{
	uint16 symbol = 0;
	uint8 sign = 0;

	uint8 p_size = !file->comp ? PSIZE : PSIZE / 2;

	for (int idx = 0; idx < p_size * p_size; idx++)
	{
		symbol = (uint16)fabs(file->ZigZag_arr[idx]);
		sign = (file->ZigZag_arr[idx] < 0) ? 1 : 0;	//negative 1 , positive 0

		/* Entropy 3. AC Flag 전송; */
		if (idx == 1)
		{
			file->output = file->output << 1 | file->AC_flag;
			file->length++;

			if (file->length == 8)
			{
				fwrite(&file->output, sizeof(uint8), 1, file->fp_out);
				file->nbyte++;
				file->output = 0;
				file->length = 0;
			}

			/* AC flag = 1; all-zero라는 의미로 1비트 전송, AC성분 인코딩 x, 디코더에서 1이라는 비트를 받으면, AC성분값을 모두 0으로 설정) */
			if (file->AC_flag)
				break;
		}

		/* Entropy 4. 심볼 전송 */
		if (symbol == 0)			// 0
			Send_AbsVal(file, 0, 2);

		else if (symbol == 1)	// 1
		{
			Send_AbsVal(file, 2, 3);
			Send_Sign(file, sign);
		}

		else
		{
			for (int n = 1; n < 12; n++)
			{
				if (symbol >= (1 << n) && symbol < (1 << n + 1))
				{
					switch (n)
					{
					case 1:
						Send_AbsVal(file, 3, 3); // 011  (2 ~ 3)
						n = 12;
						break;

					case 2:
						Send_AbsVal(file, 4, 3); // 100  (4 ~ 7)
						n = 12;
						break;

					case 3:
						Send_AbsVal(file, 5, 3); // 101  (7 ~ 16)
						n = 12;
						break;

					case 4:
						Send_AbsVal(file, 6, 3); // 110  (16 ~ 31)
						n = 12;
						break;

					case 5:
						Send_AbsVal(file, 14, 4); // 1110  (32 ~ 63)
						n = 12;
						break;

					case 6:
						Send_AbsVal(file, 30, 5); // 1 1110  (64 ~ 127)
						n = 12;
						break;

					case 7:
						Send_AbsVal(file, 62, 6); // 11 1110  (128 ~ 255)
						n = 12;
						break;

					case 8:
						Send_AbsVal(file, 126, 7); // 111 1110  (256 ~ 511)
						n = 12;
						break;

					case 9:
						Send_AbsVal(file, 254, 8); // 1111 1110	  (512 ~ 1023)
						n = 12;
						break;

					case 10:
						Send_AbsVal(file, 510, 9); // 1 1111 1110	(1024 ~ 2047)
						n = 12;
						break;

					case 11:
						Send_AbsVal(file, 1022, 10); // 11 1111 1110	(2048 ~ 4095)
						n = 12;
						break;
					}
				}
			}

			Send_Sign(file, sign);
			Send_Range_FLC(file, symbol);
		}
	}
}

void Send_AbsVal(DATA* file, uint16 code_num, uint8 bitLen)
{
	uint8 buffer = 0;
	uint16 check_bit = pow(2, bitLen - 1);

	for (uint8 i = 0; i < bitLen; i++)
	{
		buffer = check_bit & (code_num << i) ? 1 : 0;
		file->output = file->output << 1 | buffer;
		file->length++;

		if (file->length == 8)
		{
			fwrite(&file->output, sizeof(uint8), 1, file->fp_out);
			file->nbyte++;
			file->output = 0;
			file->length = 0;
		}
	}
}

