#include "Functions.h"

void Read_StreamHeader(DATA* file)
{
	/* the head of header */
	char ASCII_Array[4];	// ICSP				4byte
	memset(ASCII_Array, 0, sizeof(char) * 4);

	// read title 4byte
	for (int i = 0; i < 4; i++)
	{
		fread(&ASCII_Array[i], sizeof(char), 1, file->fp_in);
		printf("%c ", ASCII_Array[i]);
	}

	// send set value 4byte + 25bit
	fread(&file->height, sizeof(uint16), 1, file->fp_in);
	fread(&file->width, sizeof(uint16), 1, file->fp_in);
	fread(&file->output, sizeof(uint8), 1, file->fp_in);

	Read_PRED_INFO(file, file->QP_DC, 17);
	Read_PRED_INFO(file, file->QP_AC, 17);
	Read_PRED_INFO(file, file->pxDPCM_MODE, 7);
	Read_PRED_INFO(file, file->DC_DPCM_MODE, 7);
	// 	Read_PRED_INFO(file, file->MV_PREDMODE, 6);
	Read_PRED_INFO(file, file->IntraPeriod, 11);
	Read_PRED_INFO(file, file->IntraEnable, 2);
	/* the end of header */

	// skip lsb 7bit
	fread(&file->output, sizeof(uint8), 1, file->fp_in);
	file->length = 0;
}

void Read_PRED_INFO(DATA* file, uint8& code, uint8 level)
{
	uint8 buffer = 0;
	uint8 bitLen = ceil(log(level, 2));		//표현부호어 갯수에 따라 비트의 길이 결정

	code = 0;

	for (int cnt = 0; cnt < bitLen; cnt++)
	{
		buffer = file->output << (file->length % 8) & 128 ? 1 : 0;
		code = code << 1 | buffer;

		file->length++;

		if (file->length % 8 == 0)
		{
			fread(&file->output, sizeof(uint8), 1, file->fp_in);
			file->length = 0;
		}
	}
}


void Read_Symbol(DATA* file, uint8& idx, uint8 bitLen, int type)
{
	/* read sign bit */
	uint8	sign = file->output << (file->length % 8) & 128 ? 1 : 0, buffer = 0;
	uint32	result = 0;

	file->length++;

	if (file->length % 8 == 0)
	{
		fread(&file->output, sizeof(uint8), 1, file->fp_in);
		file->length = 0;
	}

	/* read range bit FLC */
	for (int i = 0; i < bitLen; i++)
	{
		buffer = file->output << (file->length % 8) & 128 ? 1 : 0;

		result = result << 1 | buffer;

		file->length++;

		if (file->length % 8 == 0)
		{
			fread(&file->output, sizeof(uint8), 1, file->fp_in);
			file->length = 0;
		}
	}

	if (type)
		file->recon_bit[idx] = pow(-1, sign) * (result + pow(2, bitLen));
	else
		file->mv_arr[idx] = pow(-1, sign) * (result + pow(2, bitLen));
}