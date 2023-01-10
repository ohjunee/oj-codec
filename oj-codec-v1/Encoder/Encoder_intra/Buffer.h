#pragma once
#include "main.h"

typedef struct ImageBuffer
{
	uint8*	org_b;
	uint8*	recon_padding;
	double*  Recon_DCT_blk;

	uint32	nRow;
	uint32	nCol;
	uint32	nWidth;

	uint8	extra_row;
	uint8	extra_col;

	uint32	UpLeft_px_recon;
	uint32	UpLeft_px_org;

	uint32  cost[2];

	uint8	index_mode;
}BUFFER;


typedef struct File_BUFFER
{
	FILE*	fp_ori;				//인코더 원본영상 버퍼(패딩된 이미지)
	FILE*	fp_ori_recon;		//인코더 복원영상 버퍼
	FILE*	fp_out;				//인코더 비트스트림
	
	FILE*	fp;					//텍스트 파일

	uint8* symbol;
	uint8* freq;
	uint8* input_sequence;

	uint8	n_symbol;
	uint8	output;
	uint8	split_flag;

	double** Quant_coeff_4x4;
	double*  PredErr;

	uint8	DCT_Mode_4x4[4];
	uint8	PredMode_4x4[4];

	uint32	length;
	uint32	maxCount;

	uint8  frameCount;

}Data;

void		ImageLoad				(BUFFER* img, Data* file, uint32 heigth, uint32 width);
void		sturct_memory_check		(BUFFER* img, Data* file, uint32 height, uint32 width);
void		struct_memory_free		(BUFFER* img, Data* file);

uint8**		uint8_calloc2D			(uint8 row, uint8 col);
uint32**	uint32_calloc2D			(uint8 row, uint8 col);
double**	double_calloc2D			(uint8 row, uint8 col);

double***	double_calloc3D			(uint8 DCT_mode, uint8 row, uint8 col);
uint8***	uint8_calloc3D			(uint8 DCT_mode, uint8 row, uint8 col);

void		memory_free2D			(double**  Quant_blk, uint8** Recon);
void		Local_memory_free3D		(double*** Quant_blk,	uint8*** Recon, uint8 DCT_mode);
void		Local_memory_free2D		(double** Pred_Err, double** Dct_blk, uint8** Pred, uint32** cost);

void		Partitioning			(BUFFER* img, Data* file,	uint32 nRow,	uint32 nCol,		uint8 p_size);

void		INTRA_Prediction		(BUFFER* img, Data* file,	uint8 p_size,	uint32 ul_recon,	uint32 ul_org);
void		INTER_Prediction		(BUFFER* img, Data* file,	uint8 p_size,	uint32 ul_recon,	uint32 ul_org);

void		Encoding				(BUFFER* img, Data* file, uint32 height, uint32 width);
void		Just_Read_CbCr			(BUFFER* img, Data* file, uint32 height, uint32 width);

void		getPredBlock			(BUFFER* img, uint8* Pred, uint8 p_size, uint8 mode, uint32 ul_recon);

void		Write_ReconImageBuffer	(BUFFER* img, uint8* Recon,		uint8 p_size,	uint32 ul_recon);
void		getPredErrBlock			(BUFFER* img, double* Pred_Err, uint8* Pred,	uint8 p_size, uint32 ul_org);

double		DCT_Basis_Vector		(uint8 u, uint8 v, uint8 p_size);

void		Quantization			(BUFFER* img, uint8 p_size, double* DCT_buf, double* Quant_buf, int type);
void		DCT						(BUFFER* img, uint8 p_size, double* PredErr_buf, double* DCT_buf, int dct_mode);
void		IDCT					(BUFFER* img, uint8 p_size, double* DCT_buf, int dct_mode);

void		checkFile				(Data* file, bool Bool);
void		Open_File				(Data* file);
void		Close_File				(Data* file);

void		encoder					(BUFFER* img, Data* file);

void		Write_Recon_Image		(BUFFER* img, Data* file, uint32 heigth, uint32 width);
uint8		set_bits				(uint32 code, uint8 prefix_num, uint8 count);

void		Send_INTRA				(Data* file, double* Quant_blk, uint8 PredMode, uint8 Best_DCT_Mode);

void		Send_Quantized_DCT_Coeff	(Data* file, double* Pred_Err, uint8 p_size);
void		Send_PRED_INFO				(Data* file, uint8 code_symbol, uint8 level);

void		Send_Sign					(Data* file, uint8 p_size);

