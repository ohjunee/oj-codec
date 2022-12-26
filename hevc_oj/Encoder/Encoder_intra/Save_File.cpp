#include "Buffer.h"
#include "Node.h"
#include "input.h"

void Send_Quantized_DCT_Coeff(Data* file, double* Pred_Err, uint8 p_size)
{
	uint8 n_symbol = 0;

	file->PredErr = Pred_Err;

	Send_Sign(file, p_size);

#if TEST_HUFFMAN == 1
	//허프만
	Tree_memory_alloc1D(file, p_size*p_size);

	Extract_symbol(file, p_size, n_symbol);					//file 구조체 변수에 양자화 변환계수 블록의 심볼갯수, 심볼값, 빈도수, 그리고 입력시퀀스를 저장(변환계수 블록 각 화소값들의 절대값)

	n_symbol > 2 ? Huffman_coding(file, n_symbol, p_size) : Send_Symbol(file, n_symbol, p_size);

	Tree_memory_free1D(file);
#else
	//지수골룸
	Exponetial_Golomb_coding(file, n_symbol, p_size, SEND_SYMBOL_VAL_);
#endif
}

#if TEST_HUFFMAN == 1
void Send_Symbol(Data* file, uint8 n_symbol, uint8 p_size)
{
	Send_Huffman_Table(file, n_symbol, p_size);

	for (int i = 0; i < p_size*p_size; i++)
	{
		uint8 bitVal = file->input_sequence[i] == file->symbol[0] ? 0 : 1;	//심볼이 한 개인 경우 file->symbol[0]의 심볼 값만 존재하므로 비트값 0만 전송함
		file->output = file->output << 1 | bitVal;
		file->length++;

		if (file->length == 8)
		{
			fwrite(&file->output, sizeof(uint8), 1, file->fp_out);
			file->output = 0;
			file->length = 0;
		}
	}
}

void Fixed_Length_Coding(Data* file, uint8 n_symbol, uint8 p_size, int type)
{
	uint8 bitLen = round(log(p_size*p_size, 2)); //심볼종류 4*4블록은 최대 16개, 4비트/ 8*8은 최대 64개, 6비트/ 3: 0010
	uint8 check_bit = pow(2, bitLen - 1);
	uint8 buffer;

	for (int i = 0; i < (type ? 1 : n_symbol); i++)
	{
		for (int j = 0; j < bitLen; j++)
		{
			if (type)
			{
				buffer = n_symbol << j & check_bit ? 1 : 0;
			}	
			else
			{
				buffer = file->freq[i] << j & check_bit ? 1 : 0;
			}

			file->output = file->output << 1 | buffer;
			file->length++;

			if (file->length % 8 == 0)
			{
				fwrite(&file->output, sizeof(uint8), 1, file->fp_out);
				file->length = 0;
				file->output = 0;
			}
		}
	}
}

void Send_Huffman_Table(Data* file, uint8 n_symbol, uint8 p_size)
{
	//양자화된 변환계수 블록에서 관찰되는 심볼 갯수 정보 보내주기			
	Fixed_Length_Coding(file, n_symbol, p_size, SEND_SYMBOL_TYPE);

	//관찰된 심볼 값 차례로 보내기
	Exponetial_Golomb_coding(file, n_symbol, p_size, SEND_SYMBOL_VAL);

	//심볼의 종류가 3개 이상인 경우만 심볼의 발생 빈도수를 차례로 보내기
	if (n_symbol > 2)
		Fixed_Length_Coding(file, n_symbol, p_size, SEND_FREQ);
}

void Huffman_coding(Data* file, uint8 n_symbol, uint8 p_size)
{
	int* codeword = (int*)calloc(sizeof(int), p_size*p_size);
	int start_index = 0;

	Node*	head = NULL;																	// 머리노드 생성, 자식노드는 모두 NULL로 초기화
	Node**	tail = (Node**)malloc(sizeof(Node*)*n_symbol);

	initialize(file, tail, n_symbol);

	Send_Huffman_Table(file, n_symbol, p_size);

	head = create_HuffmanTree(tail, n_symbol);												// 머리 노드를 허프만 트리와 연결

	for (int i = 0; i < p_size*p_size; i++)
	{
		Tree_traversal(head, file, file->input_sequence[i], codeword, start_index);			// 전위 순회하여 왼쪽 꼬리노드부터 탐색
	}

	free_Tree(head);																		// 전위 순회방식으로 메모리 순차 해제
	free(codeword);
}
#endif

void Exponetial_Golomb_coding(Data* file, uint8& n_symbol, uint8 p_size, int type)
{
	uint8 result, codeNum = 0, buffer = 0;

	for (int i = 0; i < (type ? n_symbol : p_size*p_size); i++)
	{
		result = type ? file->symbol[i] : (uint8)fabs(file->PredErr[i]);

		codeNum = result + 1;

		uint8	prefix_num = log(codeNum, 2);
		uint8	bitLen = prefix_num * 2 + 1;

		for (int cnt = 0; cnt < bitLen; cnt++)
		{
			buffer = set_bits(codeNum, prefix_num, cnt);
			file->output = (file->output << 1) | buffer;

			file->length++;

			if (file->length % 8 == 0)
			{
				fwrite(&file->output, sizeof(uint8), 1, file->fp_out);
				file->length = 0;
				file->output = 0;
			}
		}

	}
}

void Send_Sign(Data* file, uint8 p_size)
{
	uint8 sign;

	for (int i = 0; i < p_size*p_size; i++)
	{
		sign = (file->PredErr[i] < 0) ? 1 : 0;

		file->output = file->output << 1 | sign;
		file->length++;

		if (file->length == 8)
		{
			fwrite(&file->output, sizeof(uint8), 1, file->fp_out);
			file->output = 0;
			file->length = 0;
		}
	}
}

void Send_PRED_INFO(Data* file, uint8 code_symbol, uint8 level)
{
	uint8	buffer = 0;
	uint8	bitLen = round(log(level, 2));				//표현부호어 갯수에 따라 비트의 길이 결정(반올림해야함), 예측모드의 수가 6이라면 log2의 6은 소수가 나온다.
	uint8	check_bit = pow(2, bitLen - 1);

	for (int i = 0; i < bitLen; i++)
	{
		buffer = code_symbol << i & check_bit ? 1 : 0;
		file->output = (file->output << 1) | buffer;
		file->length++;

		if (file->length % 8 == 0)
		{
			fwrite(&file->output, sizeof(uint8), 1, file->fp_out);
			file->length = 0;
			file->output = 0;
		}
	}
}

void Send_INTRA(Data* file, double* Quant_blk, uint8 PredMode, uint8 Best_DCT_Mode)
{
	Send_PRED_INFO(file, file->split_flag, 2);					// 분할 정보 전달 

	for (int i = 0; i < (file->split_flag ? 4 : 1); i++)
	{
		Send_PRED_INFO(file, file->split_flag ? file->PredMode_4x4[i] : PredMode, PRED_MODE);

		Send_PRED_INFO(file, file->split_flag ? file->DCT_Mode_4x4[i] : Best_DCT_Mode, 2);

		Send_Quantized_DCT_Coeff(file, file->split_flag ? file->Quant_coeff_4x4[i] : Quant_blk, file->split_flag ? PSIZE / 2 : PSIZE);
	}
}

/* 복원 이미지 파일 생성 */
void Write_Recon_Image(BUFFER* img, Data* file, uint32 height, uint32 width)
{
	Node* head = (Node*)malloc(sizeof(Node));

	head->left = NULL;
	head->right = NULL;

	uint32 init_position = (img->nWidth + 1) + 1;	//nWidth : padding buffer(recon)의 가로길이, //init_position: 값을 읽어오는 초기 위치

	for (int i = 0; i < height; i++)
	{
		int cur_position = init_position;

		for (int j = 0; j < width; j++)
		{
			fwrite(&img->recon_padding[cur_position], sizeof(uint8), 1, file->fp_ori_recon);

			cur_position++;
		}
		init_position += img->nWidth + 1;
	}
}

uint8 set_bits(uint32 code, uint8 prefix_num, uint8 count)
{
	uint32 check_bit = 1;
	uint8  result = 0;
	uint8  codeword_len = prefix_num * 2 + 1;
	uint8  shift = codeword_len - count - 1;	// shift가 비트연산에 적용되는 초기값은 discriminator위치까지의 이동값

	if (count < prefix_num)
	{
		return result = 0;
	}

	else if (count >= prefix_num)
	{
		return result = (check_bit << shift) & code ? 1 : 0;
	}
}