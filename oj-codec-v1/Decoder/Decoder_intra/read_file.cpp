#include "Buffer.h"
#include "Node.h"
#include "input.h"

#if TEST_HUFFMAN == 1
void Read_Sign(Data* file, uint8 p_size)
{
	uint8 sign;

	for (int i = 0; i < p_size*p_size; i++)
	{
		sign = (file->output << (file->length % 8) & 128) ? 1 : 0;

		file->Recon_DCT_blk[i] = pow(-1, sign);

		file->length++;

		if (file->length % 8 == 0)
		{
			fread(&file->output, sizeof(uint8), 1, file->fp_in);
			file->length = 0;
		}
	}
}

void Read_Huffman_Table(Data* file, uint8 p_size)
{
	// 심볼 종류의 갯수 읽기
	Fixed_Length_Decoding(file, p_size, READ_SYMBOL_NUM);

	//심볼 값, 빈도 수값 저장버퍼 메모리 할당
	Tree_memory_alloc1D(file);

	//심볼 종류의 갯수만큼 심볼 값 읽기
	Exponetial_Golomb_Decoding(file, p_size, READ_SYMBOL_VAL);

	//블록크기만큼 각 심볼의 빈도 수 값 읽어오기
	if (file->n_symbol > 2)
		Fixed_Length_Decoding(file, p_size, READ_FREQ);
}

void Read_AbsVal(Data* file, uint8 p_size)
{
	file->index = 0;		// 변환계수 버퍼의 저장인덱스 위치 초기화
							
	file->n_symbol > 2 ? Huffman_decoding(file, p_size) : Read_Symbol(file, p_size); // 심볼 종류 수가 3개 이상인 경우에만 허프만 트리만들고, 만들어진 트리정보를 통해 비트스트림 읽기
																					 // 2개 이하인 경우는 비트스트림 정보로 심볼값 복원
	//허프만 테이블 정보 메모리 해제
	Tree_memory_free1D(file);
}

void Read_Symbol(Data* file, uint8 p_size)
{
	for (int i = 0; i < p_size*p_size; i++)
	{
		file->Recon_DCT_blk[i] *= (file->output << (file->length % 8) & 128) ? file->symbol[1] : file->symbol[0];		// 조건문이 참-> 읽은 비트가 1, table의 두번째 요소 값// 심볼종류가 1가지인 경우 0번째 인덱스값만 저장됨
		file->length++;

		if (file->length % 8 == 0)
		{
			fread(&file->output, sizeof(uint8), 1, file->fp_in);
			file->length = 0;
		}
	}
}

void Fixed_Length_Decoding(Data* file, uint8 p_size, int type)
{
	uint8 buffer = 0, bitLen = round(log(p_size*p_size, 2)); //표현부호어 갯수에 따라 비트의 길이 결정

	for (int i = 0; i < (type ? 1 : file->n_symbol); i++)
	{
		if (type)
			file->n_symbol = 0;

		for (int j = 0; j < bitLen; j++)
		{
			buffer = (file->output << (file->length % 8) & 128) ? 1 : 0;

			if (type)
			{
				file->n_symbol = file->n_symbol << 1 | buffer;
			}
			else
			{
				file->freq[i] = file->freq[i] << 1 | buffer;
			}

			file->length++;

			if (file->length % 8 == 0)
			{
				fread(&file->output, sizeof(uint8), 1, file->fp_in);
				file->length = 0;
			}
		}

	}
}
#else

void Read_Sign(Data* file, uint8 p_size)
{
	uint8 sign;

	for (int i = 0; i < p_size*p_size; i++)
	{
		sign = (file->output << (file->length % 8) & 128) ? 1 : 0;

		file->length++;

		if (file->length % 8 == 0)
		{
			fread(&file->output, sizeof(uint8), 1, file->fp_in);
			file->length = 0;
		}
		file->Recon_DCT_blk[i] = pow(-1, sign);
	}
}

#endif

void Read_PredErr(Data* file, uint8 p_size)
{
#if TEST_HUFFMAN == 1
	Read_Sign			(file, p_size);
	Read_Huffman_Table	(file, p_size);
	Read_AbsVal			(file, p_size);
#else
	Read_Sign(file, p_size);
	Exponetial_Golomb_Decoding(file, p_size, READ_SYMBOL_VAL_);
#endif
	/* C2 전달받은 예측에러블록 정보 역양자화, IDCT 수행 */
	I_Quantization(file, p_size);

	IDCT(file, p_size, file->DCT_skip);
}

void Read_PRED_INFO(Data* file, uint8* code, uint8 level)
{
	uint8 buffer = 0;
	uint8 bitLen = round(log(level, 2));		//표현부호어 갯수에 따라 비트의 길이 결정

	*code = 0;

	for (int cnt = 0; cnt < bitLen; cnt++)
	{
		buffer	= (file->output << (file->length % 8) & 128) ? 1 : 0;
		*code	= *code << 1 | buffer;

		file->length++;

		if (file->length % 8 == 0)
		{
			fread(&file->output, sizeof(uint8), 1, file->fp_in);
			file->length = 0;
		}
	}
}

void Exponetial_Golomb_Decoding(Data* file, uint8 p_size, int type)
{
	for (int i = 0; i < (type ? file->n_symbol : p_size*p_size); i++)		//type 1 : 심볼 값 읽기, 0 : 예측에러 절대값읽기
	{
		uint8 result = 0, buffer = 0, bitLen = 8;

		for (int cnt = 0; cnt < bitLen; cnt++)
		{
			buffer = (file->output << (file->length % 8) & 128) ? 1 : 0;
			result = result << 1 | buffer;

			file->length++;

			if (result == 1)
			{
				bitLen = cnt * 2 + 1;
			}

			if (file->length % 8 == 0)
			{
				fread(&file->output, sizeof(uint8), 1, file->fp_in);
				file->length = 0;
			}
		}
		result -= 1;
		type ? file->symbol[i] = result : file->Recon_DCT_blk[i] *= result;
	}
}