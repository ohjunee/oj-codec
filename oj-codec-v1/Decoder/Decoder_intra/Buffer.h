#pragma once
#include "main.h"

typedef struct ImageBuffer
{
	FILE*	fp_txt;

	uint8*	recon_padding;

	uint32	nRow;
	uint32	nCol;
	uint32	nWidth;

	uint8	extra_row;
	uint8	extra_col;

}BUFFER;

typedef struct File_BUFFER
{
	FILE*	fp_in;
	FILE*	fp_out;

	FILE*	fp;

	FILE*	fp_1;
	FILE*	fp_2;

	uint8	split_flag;
	uint8	PredMode;
	uint8	DCT_skip;
	uint8	output;

	uint32	length;
	uint8	frameCount;

	////////////////////

	uint8* symbol;
	uint8* freq;

	uint8  direction;

	uint8  n_symbol;
	uint8  index;

	double*	Recon_DCT_blk;
}Data;


double DCT_Basis_Vector		(uint8 u, uint8 v, uint8 p_size);

void I_Quantization			(Data* data, uint8 p_size);
void IDCT					(Data* file, uint8 p_size, int dct_mode);

void Check_File				(Data* file, bool Bool);
void Open_File				(Data* file);
void Close_File				(Data* file);

void Decoder				(BUFFER* img, Data* file);

void Get_PredBlock			(BUFFER* img, uint8* Pred_Block, uint8 p_size, uint8 mode, uint32 ul_recon);
void Get_ReconBuff			(BUFFER* img, Data* file, uint8* Pred_Block, uint8 p_size, uint32 ul_recon);

void Combine_Partition		(BUFFER* img, Data* file, uint32 nRow, uint32 nCol, uint8 p_size);
void Combine_4x4			(BUFFER* img, Data* file, uint8 p_size, uint32 ul_recon);
void Combine_8x8			(BUFFER* img, Data* file, uint8 p_size, uint32 ul_recon);

void Select_SplitFlag		(BUFFER* img, Data* file, uint8 p_size, uint32 ul_recon);
void Read_PredErr			(Data* file, uint8 p_size);

void Read_PRED_INFO			(Data* file, uint8* code, uint8 level);

void Write_Recon_Image		(BUFFER* img, Data* file, uint32 height, uint32 width);
void Check_Image			(Data* file);

void File_Decoding			(BUFFER* img, Data* file, uint32 height, uint32 width);
void Check_Padding_Image	(BUFFER* img, Data* file, uint32 height, uint32 width, bool Bool);