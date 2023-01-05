#pragma once
#include "structure.h"
#include "input.h"

void		checkFile(DATA* file, int argc, char* argv[], bool Bool);
void		Open_File(DATA* file, int argc, char* argv[]);
void		Close_File(DATA* file);

void		ImageLoad(BUFFER* img, DATA* file, Node* POC);
void		ImageSave(BUFFER* img, DATA* file);
void		Padding(BUFFER* img, DATA* file);

void		encoder(BUFFER* img, DATA* file, Node* head);
void		Read_YCbCr(BUFFER* img, DATA* file, Node* POC);
void		Partitioning(BUFFER* img, DATA* file, Node* POC);

/* INTRA */
void		INTRA_Prediction(BUFFER* img, DATA* file, uint32& upleft_px);
void		getPredBlock(BUFFER* img, uint8* Pred_b, uint8& mode, uint32& upleft_px);
void		Find_MPM(BUFFER* img, DATA* file);

/* INTER */
void		INTER_Prediction(BUFFER* img, DATA* file, Node* POC, uint32& upleft_px);

void		DCT(double* input_blk, double* DCT_buf, uint8& p_size);

double		DCT_Basis_Vector(uint8 u, uint8 v, uint8& p_size);
void		IDCT(double* DCT_buf, double* result, uint8& p_size);
void		Quantization(DATA* file, double* DCT_buf, double* Quant_buf, uint8& p_size, bool type);

void		ZigZagScan(DATA* file, double* Quant_blk, uint8& p_size);

void		DC_DPCM(BUFFER* img, DATA* file, double* Quant_blk);
void		Inverse_DC_DPCM(BUFFER* img, DATA* file, double* Quant_blk);

void		Pixel_DPCM(BUFFER* img, DATA* file, double* PredErr, double* Pixel_DPCM_blk);
void		Inverse_Pixel_DPCM(BUFFER* img, DATA* file, double* Recon_DCT_blk, double* Recon_PredErr);

Node* createFirstNode(Node* head);
void		removeLastNode(Node* head);

void		Send_StreamHeader(DATA* file);

void		Send_INTER_INFO(DATA* file);

void		Send_Quantized_DCT_Coeff(DATA* file);
void		Send_PredInfo_FLC(DATA* file, uint8 code_symbol, uint8 level);
void		Send_MV(DATA* file);
void		Send_AbsVal(DATA* file, uint16 code_num, uint8 bitLen);
void		check_AC_flag(DATA* file);
void		classification(DATA* file);


void		PSNR(DATA* file, char* argv[]);


// 2차원 메모리 동적할당
template <typename T>
T** memory_calloc2D(uint16 row, uint16 col)
{
	T** pp = 0;

	pp = (T**)calloc(row, sizeof(T*));

	*pp = (T*)calloc(row * col, sizeof(T));

	for (int j = 1; j < row; j++)
	{
		pp[j] = pp[j - 1] + col;
	}

	return pp;
}
template <typename T>
void swap(T& left, T& right)
{
	T tmp = left;
	left = right;
	right = tmp;
}

//quicksort를 사용한 이유: 예측모드 수가 많아질 경우 오름차순 정렬하는데 걸리는 시간이 짧음
template <typename T>
void QuickSort(T* ref_val, int left, int right)
{
	int L = left;
	int R = right;

	T pivot = ref_val[(L + R) / 2];

	do
	{
		while (ref_val[L] < pivot)
			L++;
		while (ref_val[R] > pivot)
			R--;

		if (L <= R)
		{
			swap(ref_val[L], ref_val[R]);
			L++; R--;
		}

	} while (L <= R);

	//피봇 기준으로

	//왼쪽 정렬: R이 음수가 되면, 왼쪽 배열은 공집합이므로 정렬x
	if (left < R)
		QuickSort(ref_val, left, R);

	//오른쪽 정렬: L이 분할된 배열의 오른쪽 끝에 도달하면 정렬x
	if (right > L)
		QuickSort(ref_val, L, right);
}

/* Entropy 4.1 부호비트 전송: 심볼값이 0이 아닌 경우만  */
template<typename T>
void Send_Sign(DATA* file, T& sign)
{
	file->output = file->output << 1 | sign;
	file->length++;

	if (file->length == 8)
	{
		fwrite(&file->output, sizeof(uint8), 1, file->fp_out);
		file->nbyte++;
		file->output = 0;
		file->length = 0;
	}
}

/* Entropy 4.2 Range 비트 전송: 심볼값이 2 이상인 경우만 range비트 고정길이부호화 */
template<typename T>
void Send_Range_FLC(DATA* file, T& codenum)
{
	uint8 bitLen = floor(log(codenum, 2)) + 1;				// 심볼 길이는 양자화변환계수의 절대값을 로그 취한 값
	uint16 val = codenum - pow(2, bitLen - 1);				// 시작 codeword를 0으로 맞추기 위해 (현재 codenum - 자기 자신) 을 해줌

	uint16 check_bit = pow(2, bitLen - 2);				// 검사비트
	uint8 buffer;

	for (int i = 0; i < bitLen - 1; i++)
	{
		buffer = (val << i) & check_bit ? 1 : 0;

		file->output = file->output << 1 | buffer;
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